// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    client_image.cpp - Handles accepted TCP/IP connections
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/uhttp.h>
#include <ulib/utility/escape.h>
#include <ulib/net/server/server.h>

bool        UClientImage_Base::bIPv6;
bool        UClientImage_Base::pipeline;
bool        UClientImage_Base::write_off;
uint32_t    UClientImage_Base::rstart;
uint32_t    UClientImage_Base::size_request;
UString*    UClientImage_Base::body;
UString*    UClientImage_Base::rbuffer;
UString*    UClientImage_Base::wbuffer;
UString*    UClientImage_Base::request; // NB: it is only a pointer, not a string object...
UString*    UClientImage_Base::pbuffer;
UString*    UClientImage_Base::msg_welcome;
const char* UClientImage_Base::rpointer;

#ifdef HAVE_SSL
SSL_CTX*    UClientImage_Base::ctx;
#endif

// NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

UString* UClientImage_Base::_value;
UString* UClientImage_Base::_buffer;
UString* UClientImage_Base::_encoded;
UString* UClientImage_Base::_set_cookie;

// NB: we cannot put this in .h for the dependency of UServer_Base class...
// ------------------------------------------------------------------------
void UClientImage_Base::logRequest(const char* filereq)
{
   U_TRACE(0+256, "UClientImage_Base::logRequest(%S)", filereq)

   U_INTERNAL_ASSERT_POINTER(logbuf)
   U_INTERNAL_ASSERT_POINTER(request)
   U_INTERNAL_ASSERT(UServer_Base::isLog())
   U_ASSERT_EQUALS(request->empty(), false)

   const char* ptr = request->data();
   uint32_t sz = request->size(), u_printf_string_max_length_save = u_printf_string_max_length;

   U_INTERNAL_DUMP("u_printf_string_max_length = %d", u_printf_string_max_length)

   if (u_printf_string_max_length == -1)
      {
      u_printf_string_max_length = u_findEndHeader(ptr, sz);

      if (u_printf_string_max_length == -1) u_printf_string_max_length = sz;

      U_INTERNAL_ASSERT_MAJOR(u_printf_string_max_length, 0)
      }

   U_INTERNAL_DUMP("u_printf_string_max_length = %d", u_printf_string_max_length)

   UServer_Base::log->log("%sreceived request (%u bytes) %.*S from %.*s\n", UServer_Base::mod_name, sz, sz, ptr, U_STRING_TO_TRACE(*logbuf));

   u_printf_string_max_length = u_printf_string_max_length_save;

#if GCC_VERSION_NUM != 30303 /* Test for GCC == 3.3.3 (SuSE Linux) */
   if (filereq) (void) UFile::writeToTmpl(filereq, *request, true);
#endif
}

void UClientImage_Base::logResponse(const char* fileres)
{
   U_TRACE(0+256, "UClientImage_Base::logResponse(%S)", fileres)

   U_INTERNAL_ASSERT_POINTER(logbuf)
   U_INTERNAL_ASSERT_POINTER(wbuffer)
   U_INTERNAL_ASSERT(UServer_Base::isLog())
   U_ASSERT_EQUALS(wbuffer->empty(), false)

   const char* ptr = wbuffer->data();
   uint32_t sz = wbuffer->size(), u_printf_string_max_length_save = u_printf_string_max_length;

   U_INTERNAL_DUMP("u_printf_string_max_length = %d", u_printf_string_max_length)

   if (u_printf_string_max_length == -1)
      {
      U_ASSERT_DIFFERS(wbuffer->empty(), true)

      u_printf_string_max_length = u_findEndHeader(ptr, sz);

      if (u_printf_string_max_length == -1) u_printf_string_max_length = sz;

      U_INTERNAL_ASSERT_MAJOR(u_printf_string_max_length, 0)
      }

   U_INTERNAL_DUMP("u_printf_string_max_length = %d", u_printf_string_max_length)

   UServer_Base::log->log("%ssent response (%u bytes) %.*S to %.*s\n", UServer_Base::mod_name, sz, sz, ptr, U_STRING_TO_TRACE(*logbuf));

   u_printf_string_max_length = u_printf_string_max_length_save;

#if GCC_VERSION_NUM != 30303 /* Test for GCC == 3.3.3 (SuSE Linux) */
   if (fileres) (void) UFile::writeToTmpl(fileres, *wbuffer, true);
#endif
}

UClientImage_Base::UClientImage_Base()
{
   U_TRACE_REGISTER_OBJECT(0, UClientImage_Base, "")

   socket        = 0;
   logbuf        = (UServer_Base::isLog() ? U_NEW(UString(4000U)) : 0);
   last_response = 0;

#ifdef HAVE_SSL
   ssl = 0;
#endif

   sfd    = state = 0;
   start  = count = 0;
   bclose = U_MAYBE;
}

// ------------------------------------------------------------------------

void UClientImage_Base::init()
{
   U_TRACE(0, "UClientImage_Base::init()")

   U_INTERNAL_ASSERT_EQUALS(body,        0)
   U_INTERNAL_ASSERT_EQUALS(rbuffer,     0)
   U_INTERNAL_ASSERT_EQUALS(wbuffer,     0)
   U_INTERNAL_ASSERT_EQUALS(pbuffer,     0)
   U_INTERNAL_ASSERT_EQUALS(_value,      0)
   U_INTERNAL_ASSERT_EQUALS(_buffer,     0)
   U_INTERNAL_ASSERT_EQUALS(_encoded,    0)
   U_INTERNAL_ASSERT_EQUALS(_set_cookie, 0)

   body    = U_NEW(UString);
   rbuffer = U_NEW(UString(U_CAPACITY));
   wbuffer = U_NEW(UString);
   pbuffer = U_NEW(UString);

   // NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

   _value      = U_NEW(UString(U_CAPACITY));
   _buffer     = U_NEW(UString(U_CAPACITY));
   _encoded    = U_NEW(UString(U_CAPACITY));
   _set_cookie = U_NEW(UString(100U));
}

void UClientImage_Base::clear()
{
   U_TRACE(0, "UClientImage_Base::clear()")

   U_INTERNAL_ASSERT_POINTER(body)
   U_INTERNAL_ASSERT_POINTER(wbuffer)
   U_INTERNAL_ASSERT_POINTER(pbuffer)
   U_INTERNAL_ASSERT_POINTER(rbuffer)

   if (msg_welcome) delete msg_welcome;

   if (body)
      {
      delete body;
      delete wbuffer;
      delete pbuffer;
      delete rbuffer;

      // NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

      U_INTERNAL_ASSERT_POINTER(_value)
      U_INTERNAL_ASSERT_POINTER(_buffer)
      U_INTERNAL_ASSERT_POINTER(_encoded)
      U_INTERNAL_ASSERT_POINTER(_set_cookie)

      delete _value;
      delete _buffer;
      delete _encoded;
      delete _set_cookie;
      }
}

void UClientImage_Base::checkCookie()
{
   U_TRACE(0, "UClientImage_Base::checkCookie()")

   U_INTERNAL_DUMP("_set_cookie = %.*S", U_STRING_TO_TRACE(*_set_cookie))

   if (_set_cookie->empty() == false)
      {
      (void) wbuffer->append(*_set_cookie);

      _set_cookie->setEmpty();
      }
}

// Check whether the ip address client ought to be allowed

__pure bool UClientImage_Base::isAllowed(UVector<UIPAllow*>& vallow_IP)
{
   U_TRACE(0, "UClientImage_Base::isAllowed(%p)", &vallow_IP)

   U_INTERNAL_ASSERT_POINTER(socket)

   bool ok = UIPAllow::isAllowed(socket->remoteIPAddress().getInAddr(), vallow_IP);

   U_RETURN(ok);
}

void UClientImage_Base::setMsgWelcome(const UString& msg)
{
   U_TRACE(0, "UClientImage_Base::setMsgWelcome(%.*S)", U_STRING_TO_TRACE(msg))

   if (msg.empty() == false)
      {
      msg_welcome = U_NEW(UString(U_CAPACITY));

      if (UEscape::decode(msg, *msg_welcome) == false)
         {
         delete msg_welcome;
                msg_welcome = 0;
         }
      }
}

int UClientImage_Base::sendfile()
{
   U_TRACE(1, "UClientImage_Base::sendfile()")

   U_INTERNAL_ASSERT_MAJOR(sfd,0)
   U_INTERNAL_ASSERT_MAJOR(count,0)
   U_INTERNAL_ASSERT_EQUALS(socket->iSockDesc, UEventFd::fd)

   ssize_t value;
   off_t offset = start;

   if (count < UServer_Base::sendfile_threshold_nonblock)
      {
      value = (socket->sendfile(sfd, &offset, count) ? count : 0); // NB: it can block if new data arrive within one second...
      }
   else
      {
      U_INTERNAL_ASSERT_DIFFERS(socket->flags          & SOCK_NONBLOCK,0)
      U_INTERNAL_ASSERT_DIFFERS(USocket::accept4_flags & SOCK_NONBLOCK,0)

      value = U_SYSCALL(sendfile, "%d,%d,%p,%u", UEventFd::fd, sfd, &offset, count);
      }

   if (value <= 0L)
      {
      U_INTERNAL_DUMP("errno = %d", errno)

      if (errno == EAGAIN) goto maybe;

      U_RETURN(U_NOT);
      }

   count -= value;

   if (count == 0)
      {
      sfd = 0;

      if (bclose != U_YES)
         {
         socket->setTcpCork(0U); // On Linux, sendfile() depends on the TCP_CORK socket option to avoid undesirable packet boundaries

         if (UEventFd::op_mask == U_WRITE_OUT)
            {
            UEventFd::op_mask = U_READ_IN;

            UNotifier::modify(this);
            }
         else if ((socket->flags          &    O_NONBLOCK) == 0 &&
                  (USocket::accept4_flags & SOCK_NONBLOCK) != 0)
            {
            socket->flags = UFile::setBlocking(UEventFd::fd, socket->flags, false);
            }
         }

      U_RETURN(U_YES);
      }

   start += value;

maybe:

   if (UEventFd::op_mask != U_WRITE_OUT)
      {
      UEventFd::op_mask = U_WRITE_OUT;

      if (UNotifier::find(UEventFd::fd)) UNotifier::modify(this);
      }

   U_RETURN(U_MAYBE);
}

// aggiungo nel log il certificato Peer del client ("issuer","serial")

void UClientImage_Base::logCertificate(void* x509)
{
   U_TRACE(0, "UClientImage_Base::logCertificate(%p)", x509)

   // NB: OpenSSL already tested the cert validity during SSL handshake and returns a X509 ptr just if the certificate is valid...

#ifdef HAVE_SSL
   if (x509)
      {
      U_INTERNAL_ASSERT_POINTER(logbuf)
      U_INTERNAL_ASSERT(UServer_Base::isLog())

      UCertificate::setForLog((X509*)x509, *logbuf);

      U_INTERNAL_DUMP("logbuf = %.*S", U_STRING_TO_TRACE(*logbuf))
      }
#endif
}

void UClientImage_Base::manageRequestSize(bool request_resize)
{
   U_TRACE(0, "UClientImage_Base::manageRequestSize(%b)", request_resize)

   U_INTERNAL_DUMP("pipeline = %b size_request = %u", pipeline, size_request)

   U_INTERNAL_ASSERT_MAJOR(size_request, 0)

   if (pipeline)
      {
      U_INTERNAL_ASSERT_DIFFERS(rstart,0U)
      U_INTERNAL_ASSERT_EQUALS(request,pbuffer)
      U_INTERNAL_ASSERT_DIFFERS(pbuffer->isNull(),true)
      U_INTERNAL_ASSERT_DIFFERS(pbuffer->same(*rbuffer),true)

      if (request_resize == false) pbuffer->size_adjust(size_request);
      else
         {
         // NB: we use request as the new read buffer... 

         rstart   = size_request = 0;
         pipeline = false;
         rpointer = pbuffer->data();

         *rbuffer = *pbuffer;
          request =  rbuffer;
         }
      }
   else
      {
      pipeline = (rbuffer->size() > size_request);

      if (pipeline)
         {
         U_INTERNAL_ASSERT_EQUALS(rstart,0U)

         *pbuffer = rbuffer->substr(0U,size_request);
          request = pbuffer;
         }
      }
}

__pure int UClientImage_Base::genericRead()
{
   U_TRACE(0, "UClientImage_Base::genericRead()")

   U_INTERNAL_ASSERT_POINTER(body)
   U_INTERNAL_ASSERT_POINTER(socket)
   U_INTERNAL_ASSERT_POINTER(rbuffer)
   U_INTERNAL_ASSERT_POINTER(pbuffer)
   U_INTERNAL_ASSERT_POINTER(wbuffer)
   U_INTERNAL_ASSERT_EQUALS(socket->iSockDesc,UEventFd::fd)

   handlerError(USocket::CONNECT); // NB: we must call function cause of SSL (must be a virtual method)...

   // reset buffer before read

   U_INTERNAL_ASSERT_EQUALS(rbuffer->isNull(), false);
   U_ASSERT(                rbuffer->writeable())

   rbuffer->setBuffer(U_CAPACITY); // NB: this string can be referenced more than one (often if U_SUBSTR_INC_REF is defined)...

   if (USocketExt::read(socket, *rbuffer, U_SINGLE_READ, UServer_Base::timeoutMS) == false)
      {
      // check if close connection... (read() == 0)

      if (socket->isClosed()) goto error;
      if (rbuffer->empty())   U_RETURN(U_PLUGIN_HANDLER_AGAIN); // NONBLOCKING...
      }

   request = rbuffer;

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);

error:
   if (UServer_Base::isParallelization()) U_EXIT(0);

   U_RETURN(U_PLUGIN_HANDLER_ERROR);
}

void UClientImage_Base::initAfterGenericRead()
{
   U_TRACE(0, "UClientImage_Base::initAfterGenericRead()")

   U_INTERNAL_DUMP("pipeline = %b", pipeline)

   U_INTERNAL_ASSERT_DIFFERS(pipeline, true)

   if (pbuffer->isNull() == false) pbuffer->clear();
   if (   body->isNull() == false)    body->clear();
                                   wbuffer->clear();
}

// method VIRTUAL to redefine

bool UClientImage_Base::newConnection()
{
   U_TRACE(0, "UClientImage_Base::newConnection()")

   U_INTERNAL_ASSERT_POINTER(socket)
   U_INTERNAL_ASSERT_EQUALS(UEventFd::fd,0)

   U_INTERNAL_DUMP("fd = %d sock_fd = %d", UEventFd::fd, socket->iSockDesc)

   UEventFd::fd = socket->iSockDesc;

   if (logbuf)
      {
      bool berror = false;
      UString tmp(U_CAPACITY);

      socket->getRemoteInfo(*logbuf);

      if (ULog::prefix) tmp.snprintf(ULog::prefix);

      tmp.snprintf_add("new client connected from %.*s, %s clients currently connected\n", U_STRING_TO_TRACE(*logbuf), UServer_Base::getNumConnection());

      if (msg_welcome)
         {
         if (ULog::prefix) tmp.snprintf_add(ULog::prefix);

         tmp.snprintf_add("sent welcome message to %.*s\n", U_STRING_TO_TRACE(*logbuf));

         if (USocketExt::write(socket, *msg_welcome) == false) berror = true;
         }

      struct iovec iov[1] = { { (caddr_t)tmp.data(), tmp.size() } };

      UServer_Base::log->write(iov, 1);

      if (berror)
         {
         U_INTERNAL_ASSERT_EQUALS(socket->iSockDesc, 0)

         U_RETURN(false);
         }
      }

   U_RETURN(true);
}

// define method VIRTUAL of class UEventFd

void UClientImage_Base::handlerError(int sock_state)
{
   U_TRACE(0, "UClientImage_Base::handlerError(%d)", sock_state)

   U_INTERNAL_ASSERT_POINTER(socket)
   U_INTERNAL_ASSERT_EQUALS(socket->iSockDesc,UEventFd::fd)

   U_INTERNAL_DUMP("fd = %d sock_fd = %d", UEventFd::fd, socket->iSockDesc)

   socket->iState = sock_state;

   UServer_Base::pClientImage = this;

   /* maybe we need some specific processing...

   if (socket->isTimeout())
      {
      }
   */
}

int UClientImage_Base::handlerRead()
{
   U_TRACE(0, "UClientImage_Base::handlerRead()")

   U_INTERNAL_ASSERT_POINTER(body)
   U_INTERNAL_ASSERT_POINTER(socket)
   U_INTERNAL_ASSERT_POINTER(rbuffer)
   U_INTERNAL_ASSERT_POINTER(wbuffer)
   U_INTERNAL_ASSERT_POINTER(pbuffer)

   // Connection-wide hooks

   state = genericRead(); // read request...

   if (state == U_PLUGIN_HANDLER_AGAIN) U_RETURN(U_NOTIFIER_OK); // NONBLOCKING...
   if (state == U_PLUGIN_HANDLER_ERROR) U_RETURN(U_NOTIFIER_DELETE);

   rstart   = size_request = 0;
   pipeline = false;
   rpointer = rbuffer->data();

#ifndef U_HTTP_CACHE_REQUEST
   UClientImage_Base::initAfterGenericRead();
#endif

loop:
   if (logbuf) logRequest();

   state = UServer_Base::pluginsHandlerREAD(); // check request type...

   if (state == U_PLUGIN_HANDLER_FINISHED)
      {
      // NB: it is possible a resize of the read buffer string...

      if (rpointer != rbuffer->data())
         {
          rpointer  = rbuffer->data();

         if (pipeline) *pbuffer = rbuffer->substr(rstart);
         }

      state = UServer_Base::pluginsHandlerRequest(); // manage request...
      }

   if (handlerWrite() == U_NOTIFIER_DELETE) state = U_PLUGIN_HANDLER_ERROR;

   u_http_info.method = 0; // NB: mark end processing of http request...

#ifdef U_HTTP_CACHE_REQUEST
   if ((state & U_PLUGIN_HANDLER_AGAIN) != 0)
      {
      if ((state & U_PLUGIN_HANDLER_ERROR) == 0) goto end;
           state = U_PLUGIN_HANDLER_ERROR;
      }
   else
#endif
      if (UServer_Base::bpluginsHandlerReset &&
          UServer_Base::pluginsHandlerReset() == U_PLUGIN_HANDLER_ERROR)
      {
      state = U_PLUGIN_HANDLER_ERROR;
      }

   U_INTERNAL_DUMP("state = %d pipeline = %b socket->isClosed() = %b", state, pipeline, socket->isClosed())

   // NB: it is difficult to change this tests...
   // -------------------------------------------
   if (socket->isClosed()                  ||
       (state    == U_PLUGIN_HANDLER_ERROR &&
        pipeline == false)                 ||
        UServer_Base::flag_loop == false)
      {
      if (UServer_Base::isParallelization()) U_EXIT(0);

      U_RETURN(U_NOTIFIER_DELETE);
      }
   // -------------------------------------------

   if (pipeline)
      {
      // manage pipelining

      U_INTERNAL_ASSERT_POINTER(request)
      U_INTERNAL_ASSERT(request == pbuffer && pbuffer->isNull() == false && pbuffer->same(*rbuffer) == false)

      U_INTERNAL_DUMP("size_request = %u request->size() = %u", size_request, request->size())

      U_ASSERT_EQUALS(size_request, request->size())
      U_ASSERT_EQUALS(rstart, rbuffer->distance(request->data()))

      rstart += size_request;

      U_INTERNAL_DUMP("rstart = %u rbuffer->size() = %u", rstart, rbuffer->size())

      if (rbuffer->size() > rstart)
         {
         // NB: check for spurios white space (IE)...

         if (rbuffer->isWhiteSpace(rstart) == false)
            {
            *pbuffer = rbuffer->substr(rstart);

               body->clear();
            wbuffer->clear();

            goto loop;
            }
         }
      }

end:
   if (u_pthread_time)
      {
      last_response = u_now->tv_sec;

      U_INTERNAL_DUMP("last_response = %ld", last_response)
      }

   U_RETURN(U_NOTIFIER_OK);
}

int UClientImage_Base::handlerWrite()
{
   U_TRACE(0, "UClientImage_Base::handlerWrite()")

   U_INTERNAL_ASSERT_POINTER(socket)

   U_INTERNAL_DUMP("write_off = %b", write_off)

   if (write_off)
      {
      write_off = false;

      U_RETURN(U_NOTIFIER_OK);
      }

   U_INTERNAL_ASSERT_POINTER(wbuffer)
   U_INTERNAL_ASSERT(socket->isOpen())

   U_INTERNAL_DUMP("count = %u", count)

   if (count)
      {
      if (UEventFd::op_mask == U_WRITE_OUT) goto send;

      socket->setTcpCork(1U); // On Linux, sendfile() depends on the TCP_CORK socket option to avoid undesirable packet boundaries
      }

   U_INTERNAL_DUMP("wbuffer(%u) = %.*S", wbuffer->size(), U_STRING_TO_TRACE(*wbuffer))
   U_INTERNAL_DUMP("   body(%u) = %.*S",    body->size(), U_STRING_TO_TRACE(*body))

   U_ASSERT_DIFFERS(wbuffer->empty(), true)

   if (logbuf) logResponse();

   if (USocketExt::write(socket, *wbuffer, *body) == false) U_RETURN(U_NOTIFIER_DELETE);  

   if (count)
      {
send:
      U_ASSERT(body->empty())
      U_INTERNAL_ASSERT_MAJOR(sfd,0)

      int ret = sendfile();

      U_INTERNAL_DUMP("state = %d %B pipeline = %b", state, state, pipeline)

           if (ret == U_NOT) state = U_PLUGIN_HANDLER_ERROR | U_PLUGIN_HANDLER_AGAIN;
      else if (ret == U_YES)
         {
         U_INTERNAL_ASSERT_EQUALS(sfd,0)
         U_INTERNAL_ASSERT_EQUALS(count,0)

         if (bclose == U_YES) U_RETURN(U_NOTIFIER_DELETE);
         }
      }

   U_RETURN(U_NOTIFIER_OK);
}

void UClientImage_Base::handlerDelete()
{
   U_TRACE(0, "UClientImage_Base::handlerDelete()")

   U_INTERNAL_ASSERT_POINTER(socket)

   U_INTERNAL_DUMP("UNotifier::num_connection = %d UNotifier::min_connection = %d", UNotifier::num_connection, UNotifier::min_connection)

   if (UNotifier::num_connection > UNotifier::min_connection)
      {
      --UNotifier::num_connection;

      UServer_Base::handlerCloseConnection(this);

      if (socket->isOpen()) socket->closesocket();

      U_INTERNAL_DUMP("sfd = %d count = %u UEventFd::op_mask = %B", sfd, count, UEventFd::op_mask)

      if (count)
         {
         // NB: pending sendfile...

         U_INTERNAL_ASSERT_MAJOR(sfd,0)

         sfd   = 0;
         count = 0;
         }

      UEventFd::op_mask = U_READ_IN;

      if (USocket::accept4_flags & SOCK_NONBLOCK) socket->flags |= O_NONBLOCK;

      // NB: to reuse object...

      UEventFd::fd = 0;

      U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, U_READ_IN)
      U_INTERNAL_ASSERT_EQUALS(((USocket::accept4_flags & SOCK_CLOEXEC)  != 0),((socket->flags & O_CLOEXEC)  != 0))
      U_INTERNAL_ASSERT_EQUALS(((USocket::accept4_flags & SOCK_NONBLOCK) != 0),((socket->flags & O_NONBLOCK) != 0))
      }
#ifdef DEBUG
   else
      {
      delete socket;

      if (logbuf) delete logbuf;
      }
#endif
}

// DEBUG

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* UClientImage_Base::dump(bool _reset) const
{
   *UObjectIO::os << "sfd                                " << sfd                << '\n'
                  << "state                              " << state              << '\n'
                  << "start                              " << start              << '\n'
                  << "count                              " << count              << '\n'
                  << "bIPv6                              " << bIPv6              << '\n'
                  << "bclose                             " << bclose             << '\n'
                  << "write_off                          " << write_off          << '\n'
                  << "last_response                      " << last_response      << '\n'
                  << "body            (UString           " << (void*)body        << ")\n"
                  << "logbuf          (UString           " << (void*)logbuf      << ")\n"
                  << "rbuffer         (UString           " << (void*)rbuffer     << ")\n"
                  << "wbuffer         (UString           " << (void*)wbuffer     << ")\n"
                  << "request         (UString           " << (void*)request     << ")\n"
                  << "pbuffer         (UString           " << (void*)pbuffer     << ")\n"
                  << "socket          (USocket           " << (void*)socket      << ")\n"
                  << "msg_welcome     (UString           " << (void*)msg_welcome << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#ifdef HAVE_SSL
const char* UClientImage<USSLSocket>::dump(bool _reset) const
{
   UClientImage_Base::dump(false);

   *UObjectIO::os << '\n'
                  << "ssl                                " << (void*)ssl;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
#endif
