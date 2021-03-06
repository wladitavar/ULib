// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    mod_proxy_service.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_PROXY_SERVICE_H
#define U_MOD_PROXY_SERVICE_H 1

#include <ulib/pcre/pcre.h>

class UCommand;
class UFileConfig;

class UModProxyService {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   static const UString* str_FOLLOW_REDIRECTS;
   static const UString* str_CLIENT_CERTIFICATE;
   static const UString* str_REMOTE_ADDRESS_IP;
   static const UString* str_WEBSOCKET;

   static void str_allocate();

   enum Error {
      INTERNAL_ERROR             = 1, // NB: we need to start from 1 because we use a vector...
      BAD_REQUEST                = 2,
      NOT_FOUND                  = 3,
      FORBIDDEN                  = 4,
      ERROR_PARSE_REQUEST        = 5, // user case start...
      ERROR_A_BAD_HEADER         = 6,
      ERROR_A_X509_MISSING_CRT   = 7,
      ERROR_A_X509_REJECTED      = 8,
      ERROR_A_X509_TAMPERED      = 9,
      ERROR_A_X509_NOBASICAUTH   = 10
   };

   // COSTRUTTORI

    UModProxyService();
   ~UModProxyService();

   // VARIE

   int     getPort() const           { return port; }
   UString getUser() const           { return user; }
   UString getServer() const;
   UString getPassword() const       { return password; }

   bool isWebSocket() const          { return websocket; }
   bool isReplaceResponse() const    { return (vreplace_response.empty() == false); }
   bool isFollowRedirects() const    { return follow_redirects; }
   bool isResponseForClient() const  { return response_client; }
   bool isRequestCertificate() const { return request_cert; }

   UString replaceResponse(const UString& msg);

   // SERVICES

   UCommand* command;
   UString environment;

   static UModProxyService* findService(const char* host, uint32_t host_len,
                                        const char* method, uint32_t method_len,
                                        UVector<UModProxyService*>& vservice);

   static void setMsgError(int err, UVector<UString>& vmsg_error);
   static void loadConfig(UFileConfig& cfg, UVector<UModProxyService*>& vservice, UVector<UString>* vmsg_error);

   static UModProxyService* findService(const UString& host, const UString& method, UVector<UModProxyService*>& vservice)
      { return findService(U_STRING_TO_PARAM(host), U_STRING_TO_PARAM(method), vservice); }

   // DEBUG

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   UVector<UString> vreplace_response;
   UVector<UIPAllow*>* vremote_address;
   UString host_mask, server, user, password, method_mask;
   UPCRE uri_mask;
   int port;
   bool request_cert, follow_redirects, response_client, websocket;

private:
   UModProxyService(const UModProxyService&)            {}
   UModProxyService& operator=(const UModProxyService&) { return *this; }
};

#endif
