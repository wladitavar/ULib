// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    mod_http.cpp - this is a plugin http for UServer
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/url.h>
#include <ulib/file_config.h>
#include <ulib/mime/header.h>
#include <ulib/plugin/mod_http.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/string_ext.h>

U_CREAT_FUNC(UHttpPlugIn)

bool                 UHttpPlugIn::virtual_host;
UString*             UHttpPlugIn::str_URI_PROTECTED_MASK;
UString*             UHttpPlugIn::str_URI_PROTECTED_ALLOWED_IP;
UVector<UIPAllow*>*  UHttpPlugIn::vallow_IP;

void UHttpPlugIn::str_allocate()
{
   U_TRACE(0, "UHttpPlugIn::str_allocate()")

   U_INTERNAL_ASSERT_EQUALS(str_URI_PROTECTED_MASK,0)
   U_INTERNAL_ASSERT_EQUALS(str_URI_PROTECTED_ALLOWED_IP,0)

   static ustringrep stringrep_storage[] = {
      { U_STRINGREP_FROM_CONSTANT("URI_PROTECTED_MASK") },
      { U_STRINGREP_FROM_CONSTANT("URI_PROTECTED_ALLOWED_IP") }
   };

   U_NEW_ULIB_OBJECT(str_URI_PROTECTED_MASK,       U_STRING_FROM_STRINGREP_STORAGE(0));
   U_NEW_ULIB_OBJECT(str_URI_PROTECTED_ALLOWED_IP, U_STRING_FROM_STRINGREP_STORAGE(1));
}

UString* UHttpPlugIn::getCGIEnvironment()
{
   U_TRACE(0, "UHttpPlugIn::getCGIEnvironment()")

   environment.setEmpty();

   if (UHTTP::UHTTP::http_info.host_len) environment.snprintf("HTTP_HEADER_HOST=%.*s\n", U_HTTP_HOST_TO_TRACE);

   if (index_alias   != U_NOT_FOUND &&
       alias.empty() == false)
      {
      U_ASSERT(UStringExt::endsWith(alias, valias[index_alias+1]))

      environment.snprintf_add("REQUEST_URI=%.*s", U_STRING_TO_TRACE(valias[index_alias]));
      }

   UString* result = (environment.empty() ? 0 : &environment);

   U_RETURN_POINTER(result, UString);
}

// Server-wide hooks

int UHttpPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "UHttpPlugIn::handlerConfig(%p)", &cfg)

   // ------------------------------------------------------------------------------------------------------------------------------------------------
   // ALIAS                         vector of URI redirection (request -> alias)
   //
   // VIRTUAL_HOST                  flag to activate practice of maintaining more than one server on one machine,
   //                               as differentiated by their apparent hostname
   // DIGEST_AUTHENTICATION         flag authentication method (yes = digest, no = basic)
   //
   // URI_PROTECTED_MASK            mask (DOS regexp) of URI protected from prying eyes
   // URI_PROTECTED_ALLOWED_IP      list of comma separated client address for IP-based access control (IPADDR[/MASK]) for URI_PROTECTED_MASK
   // ------------------------------------------------------------------------------------------------------------------------------------------------

   (void) cfg.loadVector(valias);

   if (cfg.loadTable())
      {
      virtual_host                        = cfg.readBoolean(*UServer_Base::str_VIRTUAL_HOST);
      UServer_Base::digest_authentication = cfg.readBoolean(*UServer_Base::str_DIGEST_AUTHENTICATION);

      uri_protected_mask                  = cfg[*str_URI_PROTECTED_MASK];
      uri_protected_allowed_ip            = cfg[*str_URI_PROTECTED_ALLOWED_IP];
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int UHttpPlugIn::handlerInit()
{
   U_TRACE(0, "UHttpPlugIn::handlerInit()")

   UHTTP::initForm();
   UHTTP::str_allocate();

   if (        Url::str_ftp  == 0)         Url::str_allocate();
   if (UMimeHeader::str_name == 0) UMimeHeader::str_allocate();

   if (uri_protected_allowed_ip.empty() == false)
      {
      vallow_IP = U_NEW(UVector<UIPAllow*>);

      if (UIPAllow::parseMask(uri_protected_allowed_ip, *vallow_IP) == 0)
         {
         delete vallow_IP;
                vallow_IP = 0;
         }
      }

   U_SRV_LOG_MSG("initialization of plugin success");

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

// Connection-wide hooks

int UHttpPlugIn::handlerRead()
{
   U_TRACE(0, "UHttpPlugIn::handlerRead()")

   U_INTERNAL_ASSERT_POINTER(UClientImage_Base::pClientImage)

   UClientImage_Base::pClientImage->reset();

   UHTTP::resetHTTPInfo();

   bool is_http_msg = UHTTP::readHTTP(UClientImage_Base::socket, *UClientImage_Base::rbuffer, *UClientImage_Base::body);

   // check if close connection... (read() == 0)

   if (UClientImage_Base::isClose()) U_RETURN(U_PLUGIN_HANDLER_ERROR); // return false at method UClientImage_Base::run()...

   if (UServer_Base::isLog()) UClientImage_Base::logRequest();

   uint32_t n, host_end;
   const char* content_type = 0;
   int nResponseCode = HTTP_NOT_IMPLEMENTED;

   if (is_http_msg)
      {
      if (UHTTP::http_info.version == 1)
         {
         // HTTP 1.1 want header "Date: ..."

         if (UServer_Base::isLog() == false || ULog::isSysLog()) (void) U_SYSCALL(gettimeofday, "%p,%p", &u_now, 0);

         // HTTP 1.1 want header "Host: ..."

         if (UHTTP::UHTTP::http_info.host_len == 0)
            {
            UHTTP::setHTTPBadRequest();

            goto send_response;
            }
         }

      U_INTERNAL_DUMP("UHTTP::http_info.version = %d alias = %.*S index_alias = %u", UHTTP::http_info.version, U_STRING_TO_TRACE(alias), index_alias)

      if (virtual_host)
         {
         // manage virtual host

         UString host(U_HTTP_HOST_TO_PARAM);

         // Host: hostname[:port]

         host_end = host.find(':');

         if (host_end != U_NOT_FOUND) host.size_adjust(host_end);

         U_INTERNAL_DUMP("host = %.*S", U_STRING_TO_TRACE(host))

         alias.snprintf("/%.*s%.*s", U_STRING_TO_TRACE(host), U_HTTP_URI_TO_TRACE);
         }

      if (valias.empty() == false)
         {
         // NB: check if needed to reset prev alias uri

         if (virtual_host == false) alias.size_adjust(0);

         // manage alias uri

         n = valias.size();
         UString uri(U_HTTP_URI_TO_PARAM);

         // gcc - warning: cannot optimize loop, the loop counter may overflow... ???

         for (index_alias = 0; index_alias < n; index_alias += 2)
            {
            if (uri == valias[index_alias])
               {
               (void) alias.append(valias[index_alias+1]);

               break;
               }
            }
         }

      if (alias.empty() == false)
         {
         UHTTP::setHTTPUri(alias.data(), alias.size());

         U_SRV_LOG_VAR("ALIAS: URI request changed to: %.*s", U_STRING_TO_TRACE(alias));
         }

      if (uri_protected_mask.empty() == false &&
          u_dosmatch_with_OR(U_HTTP_URI_TO_PARAM, U_STRING_TO_PARAM(uri_protected_mask), 0))
         {
         if (vallow_IP && UIPAllow::isAllowed(UClientImage_Base::socket->remoteIPAddress().getInAddr(), *vallow_IP) == false)
            {
            U_SRV_LOG_VAR("URI_PROTECTED: request '%.*s' denied by access list", U_HTTP_URI_TO_TRACE);

            UHTTP::setHTTPForbidden();

            goto send_response;
            }

         // check if it's OK via authentication (digest|basic)

         UString request_uri(U_HTTP_URI_QUERY_TO_PARAM);

         if (index_alias   != U_NOT_FOUND &&
             alias.empty() == false)
            {
            request_uri = valias[index_alias];

            if (UHTTP::http_info.query_len)
               {
               request_uri.push_back('?');

               (void) request_uri.append(U_HTTP_QUERY_TO_PARAM);
               }
            }

         if (UHTTP::processHTTPAuthorization(UServer_Base::digest_authentication, request_uri) == false)
            {
            UHTTP::setHTTPUnAuthorized(UServer_Base::digest_authentication);

            goto send_response;
            }
         }

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

   // HTTP/1.1 compliance: Sends 411 for missing Content-Length on POST requests
   //                      Sends 400 for broken Request-Line
   //                      Sends 501 for request-method != (GET|POST|HEAD)
   //                      Sends 505 for protocol != HTTP/1.0 or HTTP/1.1

   if (UHTTP::http_info.method_type)
      {
      if (UHTTP::http_info.uri_len == 0)
         {
         nResponseCode            = HTTP_BAD_REQUEST;
         content_type             = U_CTYPE_HTML;
         *UClientImage_Base::body = *UHTTP::str_frm_bad_request;
         }
      else if (UHTTP::http_info.szHeader == 0)
         {
         nResponseCode = HTTP_VERSION;
         }
      else if (UHTTP::isHttpPOST())
         {
         nResponseCode = HTTP_LENGTH_REQUIRED;
         }
      }

   *UClientImage_Base::wbuffer = UHTTP::getHTTPHeaderForResponse(nResponseCode, content_type, true, *UClientImage_Base::body);

send_response:

   (void) UClientImage_Base::genericHandlerWrite();

   U_RETURN(U_PLUGIN_HANDLER_ERROR);
}

int UHttpPlugIn::handlerRequest()
{
   U_TRACE(0, "UHttpPlugIn::handlerRequest()")

   U_INTERNAL_DUMP("method = %.*S uri = %.*S", U_HTTP_METHOD_TO_TRACE, U_HTTP_URI_TO_TRACE)

   // process the HTTP request

   UFile file;

   if (UHTTP::checkHTTPRequest(file))
      {
      if (UHTTP::isPHPRequest() ||
          UHTTP::isCGIRequest())
         {
         (void) UHTTP::processCGIRequest((UCommand*)0, getCGIEnvironment(), &file);
         }
      else
         {
         if (UHTTP::isHttpPOST()) {}                                          // NB: we don't want to process the form here (other plugin, USP or TSA)...
         else                     (void) UHTTP::processHTTPGetRequest(file);  // GET,HEAD
         }
      }

   int result = UHTTP::checkForHTTPConnectionClose(); // check for "Connection: close" in headers...

   U_RETURN(result);
}

// DEBUG

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* UHttpPlugIn::dump(bool reset) const
{
   *UObjectIO::os << "index_alias                                " << index_alias                        << '\n'
                  << "virtual_host                               " << virtual_host                       << '\n'
                  << "alias                    (UString          " << (void*)&alias                      << ")\n"
                  << "environment              (UString          " << (void*)&environment                << ")\n"
                  << "uri_protected_mask       (UString          " << (void*)&uri_protected_mask         << ")\n"
                  << "uri_protected_allowed_ip (UString          " << (void*)&uri_protected_allowed_ip   << ")\n"
                  << "valias                   (UVector<UString> " << (void*)&valias                     << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#endif
