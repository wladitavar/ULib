// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    mod_soap.cpp - this is a plugin soap for userver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file_config.h>
#include <ulib/utility/uhttp.h>
#include <ulib/plugin/mod_soap.h>
#include <ulib/net/server/server.h>
#include <ulib/xml/soap/soap_object.h>
#include <ulib/xml/soap/soap_parser.h>

U_CREAT_FUNC(USoapPlugIn)

USoapPlugIn::~USoapPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, USoapPlugIn)

   if (URPCMethod::encoder)
      {
      delete soap_parser;
      delete URPCMethod::encoder;
      delete URPCObject::dispatcher;
      }
}

// Server-wide hooks

int USoapPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "USoapPlugIn::handlerConfig(%p)", &cfg)

   // Perform registration of server SOAP method

   soap_parser = U_NEW(USOAPParser);

   USOAPObject::loadGenericMethod(&cfg);

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int USoapPlugIn::handlerInit()
{
   U_TRACE(0, "USoapPlugIn::handlerInit()")

   if (URPCMethod::encoder == 0)
      {
      soap_parser = U_NEW(USOAPParser);

      USOAPObject::loadGenericMethod(0);
      }

   U_SRV_LOG_MSG("initialization of plugin success");

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

// Connection-wide hooks

int USoapPlugIn::handlerRequest()
{
   U_TRACE(0, "USoapPlugIn::handlerRequest()")

   if (UHTTP::isSOAPRequest())
      {
      // process the SOAP message -- should be the contents of the message from "<SOAP:" to the end of the string

      U_INTERNAL_ASSERT_POINTER(soap_parser)

      bool bSendingFault;

      UString body   = soap_parser->processMessage(*UClientImage_Base::body, *URPCObject::dispatcher, bSendingFault),
              method = soap_parser->getMethodName();

      U_SRV_LOG_VAR_WITH_ADDR("method %.*S process %s for", U_STRING_TO_TRACE(method), (bSendingFault ? "failed" : "passed"));

      UHTTP::setHTTPResponse(HTTP_OK, UHTTP::str_ctype_soap, &body);

      UHTTP::setHTTPRequestProcessed();
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

// DEBUG

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* USoapPlugIn::dump(bool reset) const
{
   *UObjectIO::os << "soap_parser (USOAPParser " << (void*)soap_parser << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#endif
