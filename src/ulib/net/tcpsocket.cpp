// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    tcpsocket.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/notifier.h>
#include <ulib/net/tcpsocket.h>

// VIRTUAL METHOD

#ifdef closesocket
#undef closesocket
#endif

void UTCPSocket::closesocket()
{
   U_TRACE(1, "UTCPSocket::closesocket()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(USocket::isOpen())

   U_INTERNAL_DUMP("isBroken()   = %b", USocket::isBroken())
   U_INTERNAL_DUMP("isTimeout()  = %b", USocket::isTimeout())
   U_INTERNAL_DUMP("isEpollErr() = %b", USocket::isEpollErr())

   if (USocket::isBroken())
      {
#  if defined(HAVE_EPOLL_WAIT) && !defined(USE_LIBEVENT)
      if (USocket::isEpollErr())
         {
         U_INTERNAL_ASSERT_MAJOR(UNotifier::epollfd,0)

         if ((UNotifier::pevents->events & EPOLLRDHUP) != 0) goto next1;
                                                             goto next2;
         }
#  endif

      /* To obtain a clean closure sockets, one would call shutdown() with SHUT_WR
       * on the socket, call recv() until obtaining a return value of 0 indicating
       * that the peer has also performed an orderly shutdown, and finally call
       * close() on the socket.
       *
       * The shutdown() tells the receiver the server is done sending data. No
       * more data is going to be send. More importantly, it doesn't close the
       * socket. At the socket layer, this sends a TCP/IP FIN packet to the receiver
       */

      if (USocket::shutdown(SHUT_WR))
         {
         uint32_t count = 0;
         char _buf[8 * 1024];

         /* At this point, the socket layer has to wait until the receiver has
          * acknowledged the FIN packet by receiving a ACK packet. This is done by
          * using the recv() command in a loop until 0 or less value is returned.
          * Once recv() returns 0 (or less), 1/2 of the socket is closed
          */

         if (USocket::isBlocking()) (void) UFile::setBlocking(iSockDesc, flags, false);

         do {
            if (++count > 5) break;

            errno = 0;

            if (count == 2 && USocket::isTimeout() == false) (void) UFile::setBlocking(iSockDesc, flags, true);
            }
         while ((USocket::recv(getFd(), _buf, sizeof(_buf)) > 0) ||
                (errno == EAGAIN && UNotifier::waitForRead(iSockDesc, 500) > 0));
         }
      }

   // NB: to avoid epoll_wait() fire events on file descriptor already closed...

#if defined(HAVE_EPOLL_WAIT) && !defined(USE_LIBEVENT)
   if (UNotifier::isDynamicConnection(iSockDesc))
      {
next1:
      (void) U_SYSCALL(epoll_ctl, "%d,%d,%d,%p", UNotifier::epollfd, EPOLL_CTL_DEL, iSockDesc, (struct epoll_event*)1);
      }
next2:
#endif

   // Now we know that our FIN is ACK-ed, then you can close the second half of the socket by calling closesocket()

   USocket::_closesocket();
}
