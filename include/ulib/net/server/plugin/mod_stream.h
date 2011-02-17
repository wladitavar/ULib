// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    mod_stream.h - distributing realtime input
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_STREAM_H
#define U_MOD_STREAM_H 1

#include <ulib/utility/ring_buffer.h>
#include <ulib/net/server/server_plugin.h>

/*
The plugin interface is an integral part of UServer which provides a flexible way to add specific functionality to UServer.
Plugins allow you to enhance the functionality of UServer without changing the core of the server. They can be loaded at
startup time and can change virtually some aspect of the behaviour of the server.

UServer has 5 hooks which are used in different states of the execution of the request:
--------------------------------------------------------------------------------------------
* Server-wide hooks:
````````````````````
1) handlerConfig: called when the server finished to process its configuration
2) handlerInit:   called when the server finished its init, and before start to run

* Connection-wide hooks:
````````````````````````
3) handlerREAD:
4) handlerRequest:
5) handlerReset:
  called in `UClientImage_Base::handlerRead()`
--------------------------------------------------------------------------------------------

RETURNS:
  U_PLUGIN_HANDLER_GO_ON    if ok
  U_PLUGIN_HANDLER_FINISHED if the final output is prepared
  U_PLUGIN_HANDLER_AGAIN    if the request is empty (NONBLOCKING)

  U_PLUGIN_HANDLER_ERROR    on error
*/

class UCommand;

class U_EXPORT UStreamPlugIn : public UServerPlugIn {
public:

   static const UString* str_URI_PATH;
   static const UString* str_METADATA;
   static const UString* str_CONTENT_TYPE;

   static void str_allocate();

   // COSTRUTTORI

   UStreamPlugIn()
      {
      U_TRACE_REGISTER_OBJECT(0, UStreamPlugIn, "")

      command   = 0;
      fmetadata = 0;

      if (str_URI_PATH == 0) str_allocate();
      }

   virtual ~UStreamPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg);
   virtual int handlerInit();

   // Connection-wide hooks

   virtual int handlerRequest();

   // DEBUG

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   URingBuffer rbuf;
   UFile* fmetadata;
   UCommand* command;
   UString uri_path, metadata, content_type;

   static pid_t pid;

   static RETSIGTYPE handlerForSigTERM(int signo);

private:
   UStreamPlugIn(const UStreamPlugIn&) : UServerPlugIn() {}
   UStreamPlugIn& operator=(const UStreamPlugIn&)        { return *this; }
};

#endif