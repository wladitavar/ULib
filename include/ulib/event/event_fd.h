// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    event_fd.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_EVENT_FD_H
#define ULIB_EVENT_FD_H 1

#ifdef HAVE_LIBEVENT
#  include <ulib/libevent/event.h>
#endif

#ifdef EPOLLIN
#define U_READ_IN  EPOLLIN
#else
#define U_READ_IN  0x001 // NB: same as EPOLLIN
#endif
#ifdef EPOLLOUT
#define U_WRITE_OUT EPOLLOUT
#else
#define U_WRITE_OUT 0x004 // NB: same as EPOLLOUT
#endif
#ifndef EPOLLET
#define EPOLLET 0
#endif

#define U_NOTIFIER_OK      0
#define U_NOTIFIER_DELETE -1

class U_EXPORT UEventFd {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UEventFd* next;
   int fd, op_mask; // [ U_READ_IN | U_WRITE_OUT ]

   UEventFd()
      {
      U_TRACE_REGISTER_OBJECT(0, UEventFd, "")

      // EPOLLET  is edge-triggered (alas SIGIO, when that descriptor transitions from not ready to ready, the kernel notifies you)
      // --------------------------------------------------------------------------------------------------------------------------
      // NB: we lose notification of event with this...
   
      next    = 0;
      fd      = 0;
      op_mask = U_READ_IN;

#  ifdef HAVE_LIBEVENT
      pevent = 0;
#  endif
      }

   virtual ~UEventFd()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UEventFd)

#  ifdef HAVE_LIBEVENT
      if (pevent)
         {
         UDispatcher::del(pevent);
                   delete pevent;
         }
#  endif
      }

   // method VIRTUAL to define

   virtual int  handlerRead()           { return U_NOTIFIER_DELETE; }
   virtual int  handlerWrite()          { return U_NOTIFIER_DELETE; }
   virtual void handlerDelete()         { delete this; }
   virtual void handlerError(int state) {}

#ifdef HAVE_LIBEVENT
   UEvent<UEventFd>* pevent;

   void operator()(int fd, short event);
#endif

#ifdef DEBUG
   const char* dump(bool reset) const { return "..."; } 
#endif

private:
   UEventFd(const UEventFd&)            {}
   UEventFd& operator=(const UEventFd&) { return *this; }
};

#endif
