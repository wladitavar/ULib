// test_ssl_server.cpp

#include <ulib/ssl/certificate.h>
#include <ulib/net/server/server.h>
#include <ulib/ssl/net/sslsocket.h>

class USSLClientImage : public UClientImage<USSLSocket> {
public:

   USSLClientImage() : UClientImage<USSLSocket>()
      {
      U_TRACE_REGISTER_OBJECT(5, USSLClientImage, "", 0)
      }

   ~USSLClientImage()
      {
      U_TRACE_UNREGISTER_OBJECT(5, USSLClientImage)
      }

   // DEBUG

#ifdef DEBUG
   const char* dump(bool reset) const { return UClientImage<USSLSocket>::dump(reset); }
#endif

protected:

   // define method VIRTUAL of class UEventFd

   virtual int handlerRead()
      {
      U_TRACE(5, "USSLClientImage::handlerRead()")

      pClientImage = this; // NB: we need this because we reuse the same object UClientImage_Base...

      UClientImage_Base::genericReset();

      USocketExt::size_message  = 0; // manage buffered read (pipelining)

      int result = (USocketExt::read(socket, *rbuffer) ? U_NOTIFIER_OK
                                                       : U_NOTIFIER_DELETE);

      // check if close connection... (read() == 0)

      if (rbuffer->empty() == false && UServer_Base::isLog()) logRequest();

      if (result == U_NOTIFIER_OK)
         {
         static bool init;

         if (init == false)
            {
            init = true;

            X509* x509 = getSocket()->getPeerCertificate();

            U_INTERNAL_ASSERT_EQUALS(x509, 0)

            /*
            */
            if (getSocket()->askForClientCertificate())
               {
               X509* x509 = getSocket()->getPeerCertificate();

               U_INTERNAL_ASSERT_DIFFERS(x509, 0)

               cerr << UCertificate(x509).print();
               }
            }

         USocketExt::size_message = rbuffer->size(); // manage buffered read (pipelining)

         result = handlerWrite();
         }

      U_RETURN(result);
      }
};

U_MACROSERVER(USSLServer, USSLClientImage, USSLSocket);

static const char* getArg(const char* param) { return (param && *param ? param : 0); }

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   USSLServer s(0);

   // Load our certificate

   s.getSocket()->setContext(getArg(argv[1]), getArg(argv[2]), getArg(argv[3]), getArg(argv[4]), getArg(argv[5]), atoi(argv[6]));

   s.run();
}
