// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    server.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/command.h>
#include <ulib/file_config.h>
#include <ulib/net/udpsocket.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/string_ext.h>

#ifdef U_STATIC_HANDLER_HTTP
#  include <ulib/plugin/mod_http.h>
#endif
#ifdef U_STATIC_HANDLER_RPC
#  include <ulib/plugin/mod_rpc.h>
#endif
#ifdef U_STATIC_HANDLER_SSI
#  include <ulib/plugin/mod_ssi.h>
#endif
#ifdef U_STATIC_HANDLER_NOCAT
#  include <ulib/plugin/mod_nocat.h>
#endif
#ifdef U_STATIC_HANDLER_FCGI
#  include <ulib/plugin/mod_fcgi.h>
#endif
#ifdef U_STATIC_HANDLER_SCGI
#  include <ulib/plugin/mod_scgi.h>
#endif
#ifdef U_STATIC_HANDLER_SOAP
#  include <ulib/plugin/mod_soap.h>
#endif
#ifdef U_STATIC_HANDLER_PROXY
#  include <ulib/plugin/mod_proxy.h>
#endif
#ifdef U_STATIC_HANDLER_STREAM
#  include <ulib/plugin/mod_stream.h>
#endif
#ifdef U_STATIC_HANDLER_SOCKET
#  include <ulib/plugin/mod_socket.h>
#endif
#ifdef U_STATIC_HANDLER_TSA
#  include <ulib/plugin/mod_tsa.h>
#endif
#ifdef U_STATIC_HANDLER_GEOIP
#  include <ulib/plugin/mod_geoip.h>
#endif
#ifdef U_STATIC_HANDLER_ECHO
#  include <ulib/plugin/mod_echo.h>
#endif
#ifdef U_STATIC_HANDLER_SHIB
#  include <ulib/plugin/mod_shib.h>
#endif

#ifndef __MINGW32__
#  include <ulib/net/unixsocket.h>
#endif

#define U_DEFAULT_PORT     80
#define U_SOCKET_FLAGS     ptr->socket_flags
#define U_TOT_CONNECTION   ptr->tot_connection

int                        UServer_Base::port;
int                        UServer_Base::cgi_timeout;
int                        UServer_Base::verify_mode;
int                        UServer_Base::num_connection;
int                        UServer_Base::max_Keep_Alive;
int                        UServer_Base::preforked_num_kids;
bool                       UServer_Base::flag_loop;
bool                       UServer_Base::block_on_accept;
bool                       UServer_Base::flag_use_tcp_optimization;
char                       UServer_Base::mod_name[20];
ULog*                      UServer_Base::log;
ULock*                     UServer_Base::lock;
UString*                   UServer_Base::host;
USocket*                   UServer_Base::socket;
UProcess*                  UServer_Base::proc;
UEventFd*                  UServer_Base::handler_event;
UEventTime*                UServer_Base::ptime;
UServer_Base*              UServer_Base::pthis;
UVector<UString>*          UServer_Base::vplugin_name;
UVector<UIPAllow*>*        UServer_Base::vallow_IP;
UVector<UServerPlugIn*>*   UServer_Base::vplugin;
UServer_Base::shared_data* UServer_Base::ptr;

const UString* UServer_Base::str_USE_IPV6;
const UString* UServer_Base::str_PORT;
const UString* UServer_Base::str_MSG_WELCOME;
const UString* UServer_Base::str_COMMAND;
const UString* UServer_Base::str_ENVIRONMENT;
const UString* UServer_Base::str_RUN_AS_USER;
const UString* UServer_Base::str_PREFORK_CHILD;
const UString* UServer_Base::str_LOG_FILE;
const UString* UServer_Base::str_LOG_FILE_SZ;
const UString* UServer_Base::str_LOG_MSG_SIZE;
const UString* UServer_Base::str_CERT_FILE;
const UString* UServer_Base::str_KEY_FILE;
const UString* UServer_Base::str_PASSWORD;
const UString* UServer_Base::str_CA_FILE;
const UString* UServer_Base::str_CA_PATH;
const UString* UServer_Base::str_VERIFY_MODE;
const UString* UServer_Base::str_ALLOWED_IP;
const UString* UServer_Base::str_SOCKET_NAME;
const UString* UServer_Base::str_DOCUMENT_ROOT;
const UString* UServer_Base::str_PLUGIN;
const UString* UServer_Base::str_PLUGIN_DIR;
const UString* UServer_Base::str_REQ_TIMEOUT;
const UString* UServer_Base::str_CGI_TIMEOUT;
const UString* UServer_Base::str_VIRTUAL_HOST;
const UString* UServer_Base::str_DIGEST_AUTHENTICATION;
const UString* UServer_Base::str_URI;
const UString* UServer_Base::str_HOST;
const UString* UServer_Base::str_USER;
const UString* UServer_Base::str_SERVER;
const UString* UServer_Base::str_METHOD_NAME;
const UString* UServer_Base::str_RESPONSE_TYPE;
const UString* UServer_Base::str_IP_ADDRESS;
const UString* UServer_Base::str_MAX_KEEP_ALIVE;
const UString* UServer_Base::str_PID_FILE;
const UString* UServer_Base::str_USE_TCP_OPTIMIZATION;

void UServer_Base::str_allocate()
{
   U_TRACE(0, "UServer_Base::str_allocate()")

   U_INTERNAL_ASSERT_EQUALS(str_USE_IPV6,0)
   U_INTERNAL_ASSERT_EQUALS(str_PORT,0)
   U_INTERNAL_ASSERT_EQUALS(str_MSG_WELCOME,0)
   U_INTERNAL_ASSERT_EQUALS(str_COMMAND,0)
   U_INTERNAL_ASSERT_EQUALS(str_ENVIRONMENT,0)
   U_INTERNAL_ASSERT_EQUALS(str_RUN_AS_USER,0)
   U_INTERNAL_ASSERT_EQUALS(str_PREFORK_CHILD,0)
   U_INTERNAL_ASSERT_EQUALS(str_LOG_FILE,0)
   U_INTERNAL_ASSERT_EQUALS(str_LOG_FILE_SZ,0)
   U_INTERNAL_ASSERT_EQUALS(str_LOG_MSG_SIZE,0)
   U_INTERNAL_ASSERT_EQUALS(str_CERT_FILE,0)
   U_INTERNAL_ASSERT_EQUALS(str_KEY_FILE,0)
   U_INTERNAL_ASSERT_EQUALS(str_PASSWORD,0)
   U_INTERNAL_ASSERT_EQUALS(str_CA_FILE,0)
   U_INTERNAL_ASSERT_EQUALS(str_CA_PATH,0)
   U_INTERNAL_ASSERT_EQUALS(str_VERIFY_MODE,0)
   U_INTERNAL_ASSERT_EQUALS(str_ALLOWED_IP,0)
   U_INTERNAL_ASSERT_EQUALS(str_SOCKET_NAME,0)
   U_INTERNAL_ASSERT_EQUALS(str_DOCUMENT_ROOT,0)
   U_INTERNAL_ASSERT_EQUALS(str_PLUGIN,0)
   U_INTERNAL_ASSERT_EQUALS(str_PLUGIN_DIR,0)
   U_INTERNAL_ASSERT_EQUALS(str_REQ_TIMEOUT,0)
   U_INTERNAL_ASSERT_EQUALS(str_CGI_TIMEOUT,0)
   U_INTERNAL_ASSERT_EQUALS(str_VIRTUAL_HOST,0)
   U_INTERNAL_ASSERT_EQUALS(str_DIGEST_AUTHENTICATION,0)
   U_INTERNAL_ASSERT_EQUALS(str_URI,0)
   U_INTERNAL_ASSERT_EQUALS(str_HOST,0)
   U_INTERNAL_ASSERT_EQUALS(str_USER,0)
   U_INTERNAL_ASSERT_EQUALS(str_SERVER,0)
   U_INTERNAL_ASSERT_EQUALS(str_METHOD_NAME,0)
   U_INTERNAL_ASSERT_EQUALS(str_RESPONSE_TYPE,0)
   U_INTERNAL_ASSERT_EQUALS(str_IP_ADDRESS,0)
   U_INTERNAL_ASSERT_EQUALS(str_MAX_KEEP_ALIVE,0)
   U_INTERNAL_ASSERT_EQUALS(str_PID_FILE,0)
   U_INTERNAL_ASSERT_EQUALS(str_USE_TCP_OPTIMIZATION,0)

   static ustringrep stringrep_storage[] = {
   { U_STRINGREP_FROM_CONSTANT("USE_IPV6") },
   { U_STRINGREP_FROM_CONSTANT("PORT") },
   { U_STRINGREP_FROM_CONSTANT("WELCOME_MSG") },
   { U_STRINGREP_FROM_CONSTANT("COMMAND") },
   { U_STRINGREP_FROM_CONSTANT("ENVIRONMENT") },
   { U_STRINGREP_FROM_CONSTANT("RUN_AS_USER") },
   { U_STRINGREP_FROM_CONSTANT("PREFORK_CHILD") },
   { U_STRINGREP_FROM_CONSTANT("LOG_FILE") },
   { U_STRINGREP_FROM_CONSTANT("LOG_FILE_SZ") },
   { U_STRINGREP_FROM_CONSTANT("LOG_MSG_SIZE") },
   { U_STRINGREP_FROM_CONSTANT("CERT_FILE") },
   { U_STRINGREP_FROM_CONSTANT("KEY_FILE") },
   { U_STRINGREP_FROM_CONSTANT("PASSWORD") },
   { U_STRINGREP_FROM_CONSTANT("CA_FILE") },
   { U_STRINGREP_FROM_CONSTANT("CA_PATH") },
   { U_STRINGREP_FROM_CONSTANT("VERIFY_MODE") },
   { U_STRINGREP_FROM_CONSTANT("ALLOWED_IP") },
   { U_STRINGREP_FROM_CONSTANT("SOCKET_NAME") },
   { U_STRINGREP_FROM_CONSTANT("DOCUMENT_ROOT") },
   { U_STRINGREP_FROM_CONSTANT("PLUGIN") },
   { U_STRINGREP_FROM_CONSTANT("PLUGIN_DIR") },
   { U_STRINGREP_FROM_CONSTANT("REQ_TIMEOUT") },
   { U_STRINGREP_FROM_CONSTANT("CGI_TIMEOUT") },
   { U_STRINGREP_FROM_CONSTANT("VIRTUAL_HOST") },
   { U_STRINGREP_FROM_CONSTANT("DIGEST_AUTHENTICATION") },
   { U_STRINGREP_FROM_CONSTANT("URI") },
   { U_STRINGREP_FROM_CONSTANT("HOST") },
   { U_STRINGREP_FROM_CONSTANT("USER") },
   { U_STRINGREP_FROM_CONSTANT("SERVER") },
   { U_STRINGREP_FROM_CONSTANT("METHOD_NAME") },
   { U_STRINGREP_FROM_CONSTANT("RESPONSE_TYPE") },
   { U_STRINGREP_FROM_CONSTANT("IP_ADDRESS") },
   { U_STRINGREP_FROM_CONSTANT("MAX_KEEP_ALIVE") },
   { U_STRINGREP_FROM_CONSTANT("PID_FILE") },
   { U_STRINGREP_FROM_CONSTANT("USE_TCP_OPTIMIZATION") }
   };

   U_NEW_ULIB_OBJECT(str_USE_IPV6,              U_STRING_FROM_STRINGREP_STORAGE(0));
   U_NEW_ULIB_OBJECT(str_PORT,                  U_STRING_FROM_STRINGREP_STORAGE(1));
   U_NEW_ULIB_OBJECT(str_MSG_WELCOME,           U_STRING_FROM_STRINGREP_STORAGE(2));
   U_NEW_ULIB_OBJECT(str_COMMAND,               U_STRING_FROM_STRINGREP_STORAGE(3));
   U_NEW_ULIB_OBJECT(str_ENVIRONMENT,           U_STRING_FROM_STRINGREP_STORAGE(4));
   U_NEW_ULIB_OBJECT(str_RUN_AS_USER,           U_STRING_FROM_STRINGREP_STORAGE(5));
   U_NEW_ULIB_OBJECT(str_PREFORK_CHILD,         U_STRING_FROM_STRINGREP_STORAGE(6));
   U_NEW_ULIB_OBJECT(str_LOG_FILE,              U_STRING_FROM_STRINGREP_STORAGE(7));
   U_NEW_ULIB_OBJECT(str_LOG_FILE_SZ,           U_STRING_FROM_STRINGREP_STORAGE(8));
   U_NEW_ULIB_OBJECT(str_LOG_MSG_SIZE,          U_STRING_FROM_STRINGREP_STORAGE(9));
   U_NEW_ULIB_OBJECT(str_CERT_FILE,             U_STRING_FROM_STRINGREP_STORAGE(10));
   U_NEW_ULIB_OBJECT(str_KEY_FILE,              U_STRING_FROM_STRINGREP_STORAGE(11));
   U_NEW_ULIB_OBJECT(str_PASSWORD,              U_STRING_FROM_STRINGREP_STORAGE(12));
   U_NEW_ULIB_OBJECT(str_CA_FILE,               U_STRING_FROM_STRINGREP_STORAGE(13));
   U_NEW_ULIB_OBJECT(str_CA_PATH,               U_STRING_FROM_STRINGREP_STORAGE(14));
   U_NEW_ULIB_OBJECT(str_VERIFY_MODE,           U_STRING_FROM_STRINGREP_STORAGE(15));
   U_NEW_ULIB_OBJECT(str_ALLOWED_IP,            U_STRING_FROM_STRINGREP_STORAGE(16));
   U_NEW_ULIB_OBJECT(str_SOCKET_NAME,           U_STRING_FROM_STRINGREP_STORAGE(17));
   U_NEW_ULIB_OBJECT(str_DOCUMENT_ROOT,         U_STRING_FROM_STRINGREP_STORAGE(18));
   U_NEW_ULIB_OBJECT(str_PLUGIN,                U_STRING_FROM_STRINGREP_STORAGE(19));
   U_NEW_ULIB_OBJECT(str_PLUGIN_DIR,            U_STRING_FROM_STRINGREP_STORAGE(20));
   U_NEW_ULIB_OBJECT(str_REQ_TIMEOUT,           U_STRING_FROM_STRINGREP_STORAGE(21));
   U_NEW_ULIB_OBJECT(str_CGI_TIMEOUT,           U_STRING_FROM_STRINGREP_STORAGE(22));
   U_NEW_ULIB_OBJECT(str_VIRTUAL_HOST,          U_STRING_FROM_STRINGREP_STORAGE(23));
   U_NEW_ULIB_OBJECT(str_DIGEST_AUTHENTICATION, U_STRING_FROM_STRINGREP_STORAGE(24));
   U_NEW_ULIB_OBJECT(str_URI,                   U_STRING_FROM_STRINGREP_STORAGE(25));
   U_NEW_ULIB_OBJECT(str_HOST,                  U_STRING_FROM_STRINGREP_STORAGE(26));
   U_NEW_ULIB_OBJECT(str_USER,                  U_STRING_FROM_STRINGREP_STORAGE(27));
   U_NEW_ULIB_OBJECT(str_SERVER,                U_STRING_FROM_STRINGREP_STORAGE(28));
   U_NEW_ULIB_OBJECT(str_METHOD_NAME,           U_STRING_FROM_STRINGREP_STORAGE(29));
   U_NEW_ULIB_OBJECT(str_RESPONSE_TYPE,         U_STRING_FROM_STRINGREP_STORAGE(30));
   U_NEW_ULIB_OBJECT(str_IP_ADDRESS,            U_STRING_FROM_STRINGREP_STORAGE(31));
   U_NEW_ULIB_OBJECT(str_MAX_KEEP_ALIVE,        U_STRING_FROM_STRINGREP_STORAGE(32));
   U_NEW_ULIB_OBJECT(str_PID_FILE,              U_STRING_FROM_STRINGREP_STORAGE(33));
   U_NEW_ULIB_OBJECT(str_USE_TCP_OPTIMIZATION,  U_STRING_FROM_STRINGREP_STORAGE(34));
}

UServer_Base::UServer_Base(UFileConfig* cfg)
{
   U_TRACE_REGISTER_OBJECT(0, UServer_Base, "%p", cfg)

   U_INTERNAL_ASSERT_EQUALS(pthis, 0)

   if (str_USE_IPV6 == 0) str_allocate();

   port              = U_DEFAULT_PORT;
   pthis             = this;
   UEventFd::op_mask = U_READ_IN;

   if (cfg) loadConfigParam(*cfg);
}

UServer_Base::~UServer_Base()
{
   U_TRACE_UNREGISTER_OBJECT(0, UServer_Base)

   U_INTERNAL_ASSERT_POINTER(socket)

   UClientImage_Base::resetBuffer();

                      UNotifier::erase(this,          false); // NB: to avoid to delete himself...
   if (handler_event) UNotifier::erase(handler_event, false);
                      UNotifier::clear();

   if (vplugin)
      {
      delete vplugin;
      delete vplugin_name;

#  ifdef HAVE_MODULES
      UPlugIn<void*>::clear();
#  endif
      }

   if (log)        delete log;
   if (ptr)        UFile::munmap(ptr, sizeof(shared_data) + sizeof(sem_t));
   if (lock)       delete lock;
   if (host)       delete host;
   if (proc)       delete proc;
   if (ptime)      delete ptime;
   if (vallow_IP)  delete vallow_IP;

   delete socket;

   UClientImage_Base::clear();
}

void UServer_Base::loadConfigParam(UFileConfig& cfg)
{
   U_TRACE(0, "UServer_Base::loadConfigParam(%p)", &cfg)

   U_ASSERT_EQUALS(cfg.empty(), false)

   // --------------------------------------------------------------------------------------------------------------------------------------
   // userver - configuration parameters
   // --------------------------------------------------------------------------------------------------------------------------------------
   // USE_IPV6      flag indicating the use of ipv6
   // SERVER        host name or ip address for the listening socket
   // PORT          port number             for the listening socket
   // SOCKET_NAME   file name               for the listening socket
   // IP_ADDRESS    public ip address of host for the interface connected to the Internet (autodetected if not specified)
   // ALLOWED_IP    list of comma separated client address for IP-based access control (IPADDR[/MASK])
   //
   // USE_TCP_OPTIMIZATION flag indicating the use of TCP/IP options to optimize data transmission (TCP_CORK, TCP_DEFER_ACCEPT, TCP_QUICKACK)
   //
   // PID_FILE      write pid on file indicated
   // WELCOME_MSG   message of welcome to send initially to client
   // RUN_AS_USER   downgrade the security to that of user account
   // DOCUMENT_ROOT The directory out of which you will serve your documents
   //
   // LOG_FILE      locations   for file log
   // LOG_FILE_SZ   memory size for file log
   // LOG_MSG_SIZE  limit length of print network message to LOG_MSG_SIZE chars (default 128)
   //
   // PLUGIN        list of plugins to load, a flexible way to add specific functionality to the server
   // PLUGIN_DIR    directory of plugins to load
   //
   // REQ_TIMEOUT   timeout for request from client
   // CGI_TIMEOUT   timeout for cgi execution
   //
   // MAX_KEEP_ALIVE Specifies the maximum number of requests that can be served through a Keep-Alive (Persistent) session.
   //                (Value <= 1 will disable Keep-Alive)
   //
   // CERT_FILE     server certificate
   // KEY_FILE      server private key
   // PASSWORD      password for server private key
   // CA_FILE       locations of trusted CA certificates used in the verification
   // CA_PATH       locations of trusted CA certificates used in the verification
   // VERIFY_MODE   mode of verification (SSL_VERIFY_NONE=0, SSL_VERIFY_PEER=1, SSL_VERIFY_FAIL_IF_NO_PEER_CERT=2, SSL_VERIFY_CLIENT_ONCE=4)
   //
   // PREFORK_CHILD number of child server processes created at startup ( 0 - serialize, no forking
   //                                                                     1 - classic, forking after client accept
   //                                                                    >1 - pool of serialized processes plus monitoring process)
   // --------------------------------------------------------------------------------------------------------------------------------------

   server        = cfg[*str_SERVER];
   as_user       = cfg[*str_RUN_AS_USER],
   log_file      = cfg[*str_LOG_FILE];
   allow_IP      = cfg[*str_ALLOWED_IP];
   name_sock     = cfg[*str_SOCKET_NAME];
   IP_address    = cfg[*str_IP_ADDRESS];
   document_root = cfg[*str_DOCUMENT_ROOT];

#ifdef HAVE_IPV6
   UClientImage_Base::bIPv6   = cfg.readBoolean(*str_USE_IPV6);
#endif
   flag_use_tcp_optimization  = cfg.readBoolean(*str_USE_TCP_OPTIMIZATION);

   port                       = cfg.readLong(*str_PORT, U_DEFAULT_PORT);
   cgi_timeout                = cfg.readLong(*str_CGI_TIMEOUT);
   max_Keep_Alive             = cfg.readLong(*str_MAX_KEEP_ALIVE);
   USocket::req_timeout       = cfg.readLong(*str_REQ_TIMEOUT);
   u_printf_string_max_length = cfg.readLong(*str_LOG_MSG_SIZE);

   if (cgi_timeout) UCommand::setTimeout(cgi_timeout);

   UClientImage_Base::setMsgWelcome(cfg[*str_MSG_WELCOME]);

#ifdef HAVE_SSL
   ca_file     = cfg[*str_CA_FILE];
   ca_path     = cfg[*str_CA_PATH];
   key_file    = cfg[*str_KEY_FILE];
   password    = cfg[*str_PASSWORD];
   cert_file   = cfg[*str_CERT_FILE];
   verify_mode = cfg.readLong(*str_VERIFY_MODE);
#endif

#ifndef __MINGW32__
   preforked_num_kids = cfg.readLong(*str_PREFORK_CHILD);
#endif

   // write pid on file...

   UString pid_file = cfg[*str_PID_FILE];

   if (pid_file.empty() == false) (void) UFile::writeTo(pid_file, UString(u_pid_str, u_pid_str_len));

   // open log

   if (log_file.empty() == false) openLog(log_file, cfg.readLong(*str_LOG_FILE_SZ));

   // load plugin modules and call server-wide hooks handlerConfig()...

   if (loadPlugins(cfg[*str_PLUGIN_DIR], cfg[*str_PLUGIN], &cfg) == U_PLUGIN_HANDLER_ERROR)
      {
      U_ERROR("plugins load FAILED. Going down...");
      }

   cfg.clear();
   cfg.deallocate();
}

// load plugin modules and call server-wide hooks handlerConfig()...

U_NO_EXPORT void UServer_Base::loadStaticLinkedModules(const char* name)
{
   U_TRACE(0, "UServer_Base::loadStaticLinkedModules(%S)", name)

   UString x(name);

   vplugin_name->push_back(x);

   UServerPlugIn* _plugin = 0;

#ifdef U_STATIC_HANDLER_HTTP
   if (x.equal(U_CONSTANT_TO_PARAM("mod_http")))   { _plugin = U_NEW(UHttpPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_RPC
   if (x.equal(U_CONSTANT_TO_PARAM("mod_rpc")))    { _plugin = U_NEW(URpcPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_SSI
   if (x.equal(U_CONSTANT_TO_PARAM("mod_ssi")))    { _plugin = U_NEW(USSIPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_NOCAT
   if (x.equal(U_CONSTANT_TO_PARAM("mod_nocat")))  { _plugin = U_NEW(UNoCatPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_FCGI
   if (x.equal(U_CONSTANT_TO_PARAM("mod_fcgi")))   { _plugin = U_NEW(UFCGIPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_SCGI
   if (x.equal(U_CONSTANT_TO_PARAM("mod_scgi")))   { _plugin = U_NEW(USCGIPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_SOAP
   if (x.equal(U_CONSTANT_TO_PARAM("mod_soap")))   { _plugin = U_NEW(USoapPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_PROXY
   if (x.equal(U_CONSTANT_TO_PARAM("mod_proxy")))  { _plugin = U_NEW(UProxyPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_STREAM
   if (x.equal(U_CONSTANT_TO_PARAM("mod_stream"))) { _plugin = U_NEW(UStreamPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_SOCKET
   if (x.equal(U_CONSTANT_TO_PARAM("mod_socket"))) { _plugin = U_NEW(UWebSocketPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_TSA
   if (x.equal(U_CONSTANT_TO_PARAM("mod_tsa")))    { _plugin = U_NEW(UTsaPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_GEOIP
   if (x.equal(U_CONSTANT_TO_PARAM("mod_geoip")))  { _plugin = U_NEW(UGeoIPPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_ECHO
   if (x.equal(U_CONSTANT_TO_PARAM("mod_echo")))   { _plugin = U_NEW(UEchoPlugIn); goto next; }
#endif
#ifdef U_STATIC_HANDLER_SHIB
   if (x.equal(U_CONSTANT_TO_PARAM("mod_shib")))   { _plugin = U_NEW(UShibPlugIn); goto next; }
#endif

   goto next;

next:
   U_INTERNAL_ASSERT_POINTER(_plugin)

   vplugin->push_back(_plugin);

   if (isLog()) ULog::log("[%s] link of static plugin ok\n", name);
}

int UServer_Base::loadPlugins(const UString& plugin_dir, const UString& plugin_list, UFileConfig* cfg)
{
   U_TRACE(0, "UServer_Base::loadPlugins(%.*S,%.*S,%p)", U_STRING_TO_TRACE(plugin_dir), U_STRING_TO_TRACE(plugin_list), cfg)

   UString name;
   bool bnostatic;
   uint32_t i, length;
   UServerPlugIn* _plugin = 0;
   UVector<UString> vec(plugin_list);
   int result = U_PLUGIN_HANDLER_ERROR;

   vplugin_name = U_NEW(UVector<UString>(10U));
   vplugin      = U_NEW(UVector<UServerPlugIn*>(10U));

   /* I do know that to include code in the middle of a function is hacky and dirty,
    * but this is the best solution that I could figure out. If you have some idea to
    * clean it up, please, don't hesitate and let me know.
    */

#  include "plugin/loader.autoconf.cpp"

   if (plugin_list.empty()) goto next;

   bnostatic = vplugin->empty();

#ifdef HAVE_MODULES
   U_INTERNAL_ASSERT(plugin_dir.isNullTerminated())

   if (plugin_dir.empty() == false) UPlugIn<void*>::setPluginDirectory(plugin_dir.data());
#endif

   for (i = 0, length = vec.size(); i < length; ++i)
      {
      name = vec[i];

      if (vplugin_name->find(name) != U_NOT_FOUND) continue;

#  ifdef HAVE_MODULES
      _plugin = UPlugIn<UServerPlugIn*>::create(U_STRING_TO_PARAM(name));
#  endif

      if (_plugin == 0)
         {
         U_SRV_LOG("load of plugin '%.*s' FAILED", U_STRING_TO_TRACE(name));

         goto end;
         }

      name.duplicate();

      if (isLog()) ULog::log("[%.*s] load of plugin success\n", U_STRING_TO_TRACE(name));

      if (bnostatic)
         {
         vplugin->push_back(_plugin);
         vplugin_name->push_back(name);
         }
      else
         {
         vplugin->insert(0, _plugin);
         vplugin_name->insert(0, name);
         }
      }

next:
   U_INTERNAL_ASSERT_EQUALS(vplugin->size(), vplugin_name->size())

   if (cfg == 0) goto ok;

   for (i = 0, length = vplugin->size(); i < length; ++i)
      {
      name    = (*vplugin_name)[i];
      _plugin = (*vplugin)[i];

      if (cfg->searchForObjectStream(U_STRING_TO_PARAM(name)))
         {
         cfg->clear();

         if (isLog()) (void) snprintf(mod_name, sizeof(mod_name), "[%.*s] ", U_STRING_TO_TRACE(name));

         result = _plugin->handlerConfig(*cfg);

         cfg->reset();

         if (result != U_PLUGIN_HANDLER_GO_ON) goto end;
         }
      }

ok:
   result = U_PLUGIN_HANDLER_FINISHED;

end:
   mod_name[0] = '\0';

   U_RETURN(result);
}

// manage plugin handler hooks...

#define U_PLUGIN_HANDLER(xxx)                                                             \
                                                                                          \
int UServer_Base::pluginsHandler##xxx()                                                   \
{                                                                                         \
   U_TRACE(0, "UServer_Base::pluginsHandler"#xxx"()")                                     \
                                                                                          \
   U_INTERNAL_ASSERT_POINTER(vplugin)                                                     \
                                                                                          \
   int result;                                                                            \
   UString name;                                                                          \
   UServerPlugIn* _plugin;                                                                \
                                                                                          \
   for (uint32_t i = 0, length = vplugin->size(); i < length; ++i)                        \
      {                                                                                   \
      _plugin = (*vplugin)[i];                                                            \
                                                                                          \
      if (isLog())                                                                        \
         {                                                                                \
         name = (*vplugin_name)[i];                                                       \
                                                                                          \
         (void) snprintf(mod_name, sizeof(mod_name), "[%.*s] ", U_STRING_TO_TRACE(name)); \
         }                                                                                \
                                                                                          \
      result = _plugin->handler##xxx();                                                   \
                                                                                          \
      if (result != U_PLUGIN_HANDLER_GO_ON) goto end;                                     \
      }                                                                                   \
                                                                                          \
   result = U_PLUGIN_HANDLER_FINISHED;                                                    \
                                                                                          \
end:                                                                                      \
   mod_name[0] = '\0';                                                                    \
                                                                                          \
   U_RETURN(result);                                                                      \
}

U_PLUGIN_HANDLER(Init)
U_PLUGIN_HANDLER(READ)
U_PLUGIN_HANDLER(Request)
U_PLUGIN_HANDLER(Reset)

void UServer_Base::runAsUser()
{
   U_TRACE(0, "UServer_Base::runAsUser()")

   U_INTERNAL_ASSERT(pthis->as_user.isNullTerminated())

   if (pthis->as_user.empty() == false)
      {
      const char* user = pthis->as_user.data();

      if (u_ranAsUser(user, false))
         {
         U_SRV_LOG("server run with user %S permission", user);
         }
      else
         {
#     ifndef __MINGW32__
         U_ERROR("set user %S context failed...", user);
#     endif
         }

      pthis->as_user.clear();
      }
}

void UServer_Base::setNotifier(bool bfork)
{
   U_TRACE(0, "UServer_Base::setNotifier(%b)", bfork)

   // NB: in the classic model we don't need event manager (loop: accept-fork) and the forked child don't accept new client...

   if (bfork)
      {
      UNotifier::init();

      if (isLog()) u_unatexit(&ULog::close); // NB: needed because all instance try to close the log... (inherits from its parent)

      if (isPreForked())
         {
                            UNotifier::insert(pthis);          // NB: we ask to notify for request of connection...
         if (handler_event) UNotifier::insert(handler_event);  // NB: we ask to notify for change of file system (inotify)...

         // NB: we can't use UInterrupt::erase() because it restore the old action (UInterrupt::init)...
         UInterrupt::setHandlerForSignal(SIGHUP, (sighandler_t)SIG_IGN);
         }
      else
         {
         U_ASSERT(isClassic()) // NB: we don't ask to notify for request of connection...
         }
      }
   else
      {
      if (preforked_num_kids == 0)
         {
         UNotifier::init();

                            UNotifier::insert(pthis);          // NB: we ask to notify for request of connection...
         if (handler_event) UNotifier::insert(handler_event);  // NB: we ask to notify for change of file system (inotify)...
         }
      }
}

void UServer_Base::init()
{
   U_TRACE(1, "UServer_Base::init()")

   U_INTERNAL_ASSERT_POINTER(UClientImage_Base::socket)

   if (name_sock.empty())
      {
#  ifdef HAVE_SSL
      if (UClientImage_Base::socket->isSSL())
         {
         block_on_accept = true;

         USSLSocket::method = (SSL_METHOD*) SSLv23_server_method();
         }
      else
#  endif
         {
         USocket::accept4_flags = SOCK_NONBLOCK | SOCK_CLOEXEC;

         if (isPreForked() == false) block_on_accept = true;
         }
      }
   else
      {
      block_on_accept = true;

#  ifndef __MINGW32__
      U_INTERNAL_ASSERT(socket->isIPC())

      UUnixSocket::setPath(name_sock.data()); // unix socket...
#  endif
      }

   if (socket->setServer(server, port, 1024) == false)
      {
      if (server.empty()) server = U_STRING_FROM_CONSTANT("*");

      U_ERROR("run as server with local address '%.*s:%d' FAILED...", U_STRING_TO_TRACE(server), port);
      }

   UEventFd::fd = socket->getFd();

   // get name host

   host = U_NEW(UString(server.empty() ? USocketExt::getNodeName() : server));

   if (port != 80)
      {
      host->push_back(':');

      (void) host->append(UStringExt::numberToString(port));
      }

   U_SRV_LOG("HOST registered as: %.*s", U_STRING_TO_TRACE(*host));

   // get IP address host (default source)

   if (IP_address.empty()      &&
       server.empty() == false &&
       u_isIPAddr(UClientImage_Base::bIPv6, U_STRING_TO_PARAM(server)))
      {
      IP_address = server;
      }

   /* The above code does NOT make a connection or send any packets (to 64.233.187.99 which is google).
    * Since UDP is a stateless protocol connect() merely makes a system call which figures out how to
    * route the packets based on the address and what interface (and therefore IP address) it should
    * bind to. Returns an array containing the family (AF_INET), local port, and local address (which
    * is what we want) of the socket.
    */

   UUDPSocket cClientSocket(UClientImage_Base::bIPv6);

   if (cClientSocket.connectServer(U_STRING_FROM_CONSTANT("64.233.187.99"), 1001))
      {
      UClientImage_Base::socket->setLocal((socket->cLocalAddress = cClientSocket.cLocalAddress));

      if (IP_address.empty()) IP_address = UString(socket->getLocalInfo());
      }

   U_SRV_LOG("SERVER IP ADDRESS registered as: %.*s", U_STRING_TO_TRACE(IP_address));

   // Instructs server to accept connections from the IP address IPADDR. A CIDR mask length can be
   // supplied optionally after a trailing slash, e.g. 192.168.0.0/24, in which case addresses that
   // match in the most significant MASK bits will be allowed. If no options are specified, all clients
   // are allowed. Unauthorized connections are rejected by closing the TCP connection immediately. A
   // warning is logged on the server but nothing is sent to the client.

   if (allow_IP.empty() == false)
      {
      vallow_IP = U_NEW(UVector<UIPAllow*>);

      if (UIPAllow::parseMask(allow_IP, *vallow_IP) == 0)
         {
         delete vallow_IP;
                vallow_IP = 0;
         }
      }

   // DOCUMENT_ROOT: The directory out of which you will serve your documents

   if (document_root.empty() ||
       document_root.equal(U_CONSTANT_TO_PARAM(".")))
      {
      (void) document_root.assign(u_cwd, u_cwd_len);
      }
   else
      {
      U_INTERNAL_ASSERT(document_root.isNullTerminated())

      (void) u_canonicalize_pathname(document_root.data());

      U_INTERNAL_DUMP("document_root = %S", document_root.data())
      }

   if (document_root.first_char() == '.') document_root = UFile::getRealPath(document_root.data());

   if (UFile::chdir(document_root.data(), false) == false)
      {
      U_ERROR("chdir to working directory (DOCUMENT_ROOT) %S FAILED. Going down...", document_root.data());
      }

   U_SRV_LOG("working directory (DOCUMENT_ROOT) changed to %S", document_root.data());

   if (preforked_num_kids)
      {
      // manage shared data...

      U_INTERNAL_ASSERT_EQUALS(ptr,0)

      ptr = (shared_data*) UFile::mmap(sizeof(shared_data) + sizeof(sem_t));

      U_INTERNAL_ASSERT_DIFFERS(ptr,MAP_FAILED)
      }

   // manage block on accept...

   if (block_on_accept == false)
      {
      (void) U_SYSCALL(fcntl, "%d,%d,%d", UEventFd::fd, F_SETFL, O_RDWR | O_NONBLOCK | O_CLOEXEC);
      }
   else if (isPreForked())
      {
      U_INTERNAL_ASSERT_EQUALS(lock,0)

      lock = U_NEW(ULock);

      U_INTERNAL_ASSERT_POINTER(lock)

      lock->init((sem_t*)((char*)ptr + sizeof(shared_data)));

      U_SOCKET_FLAGS = U_SYSCALL(fcntl, "%d,%d,%d", UEventFd::fd, F_GETFL, 0);

      U_INTERNAL_DUMP("O_NONBLOCK = %B, U_SOCKET_FLAGS = %B", O_NONBLOCK, U_SOCKET_FLAGS)
      }

#ifdef HAVE_SSL
   if (UClientImage_Base::socket->isSSL()) goto next; // For SSL skip tcp optimization
#endif

   if (flag_use_tcp_optimization)
      {
      U_ASSERT(name_sock.empty()) // no unix socket...

   // socket->setBufferRCV(128 * 1024);
   // socket->setBufferSND(128 * 1024);

      /* Let's say an application just issued a request to send a small block of data. Now, we could
       * either send the data immediately or wait for more data. Some interactive and client-server
       * applications will benefit greatly if we send the data right away. For example, when we are
       * sending a short request and awaiting a large response, the relative overhead is low compared
       * to the total amount of data transferred, and the response time could be much better if the
       * request is sent immediately. This is achieved by setting the TCP_NODELAY option on the socket,
       * which disables the Nagle algorithm.
       */

      socket->setTcpNoDelay(1U);

      /* Linux (along with some other OSs) includes a TCP_DEFER_ACCEPT option in its TCP implementation.
       * Set on a server-side listening socket, it instructs the kernel not to wait for the final ACK packet
       * and not to initiate the process until the first packet of real data has arrived. After sending the SYN/ACK,
       * the server will then wait for a data packet from a client. Now, only three packets will be sent over the
       * network, and the connection establishment delay will be significantly reduced, which is typical for HTTP.
       *
       * NB: Takes an integer value (seconds)
       */

      socket->setTcpDeferAccept(1U);

      /* Another way to prevent delays caused by sending useless packets is to use the TCP_QUICKACK option.
       * This option is different from TCP_DEFER_ACCEPT, as it can be used not only to manage the process of
       * connection establishment, but it can be used also during the normal data transfer process. In addition,
       * it can be set on either side of the client-server connection. Delaying sending of the ACK packet could
       * be useful if it is known that the user data will be sent soon, and it is better to set the ACK flag on
       * that data packet to minimize overhead. When the sender is sure that data will be immediately be sent
       * (multiple packets), the TCP_QUICKACK option can be set to 0. The default value of this option is 1 for
       * sockets in the connected state, which will be reset by the kernel to 1 immediately after the first use.
       * (This is a one-time option)
       */

      socket->setTcpQuickAck(0U);
      }

   // init plugin modules...

next:
   flag_loop = true;

   setProcessManager();

   if (vplugin &&
       pluginsHandlerInit() != U_PLUGIN_HANDLER_FINISHED)
      {
      U_ERROR("plugins initialization FAILED. Going down...");
      }

   runAsUser();

   setNotifier(false);

   if (USocket::req_timeout) ptime = U_NEW(UTimeoutConnection(USocket::req_timeout, 0L));
}

RETSIGTYPE UServer_Base::handlerForSigHUP(int signo)
{
   U_TRACE(0, "[SIGHUP] UServer_Base::handlerForSigHUP(%d)", signo)

   U_INTERNAL_ASSERT_POINTER(pthis)

   U_INTERNAL_ASSERT(proc->parent())

   U_SRV_LOG("--- SIGHUP (Interrupt) ---");

   pthis->handlerSignal(); // manage before regenering preforked pool of children...

   // NB: we can't use UInterrupt::erase() because it restore the old action (UInterrupt::init)...
   UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)SIG_IGN);

   // NB: for logrotate...
   if (isLog()) ULog::reopen();

   sendSigTERM();

#ifdef HAVE_LIBEVENT
   UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)UServer_Base::handlerForSigTERM); //  sync signal
#else
   UInterrupt::insert(             SIGTERM, (sighandler_t)UServer_Base::handlerForSigTERM); // async signal
#endif
}

RETSIGTYPE UServer_Base::handlerForSigTERM(int signo)
{
   U_TRACE(0, "[SIGTERM] UServer_Base::handlerForSigTERM(%d)", signo)

   U_SRV_LOG("--- SIGTERM (Interrupt) ---");

   U_INTERNAL_ASSERT(flag_loop)

   flag_loop = false;

   if (proc->parent())
      {
      // NB: we can't use UInterrupt::erase() because it restore the old action (UInterrupt::init)...
      UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)SIG_IGN);

      sendSigTERM();
      }
   else
      {
#  ifdef HAVE_LIBEVENT
      (void) UDispatcher::exit(0);
#  endif

      UInterrupt::erase(SIGTERM); // async signal
      }
}

int UServer_Base::handlerRead() // This method is called to accept a new connection on the server socket
{
   U_TRACE(1, "UServer_Base::handlerRead()")

   U_INTERNAL_ASSERT_POINTER(UClientImage_Base::socket)

   /* There may not always be a connection waiting after a SIGIO is delivered or select(2) or poll(2) return
    * a readability event because the connection might have been removed by an asynchronous network error or
    * another thread before accept() is called. If this happens then the call will block waiting for the next
    * connection to arrive. To ensure that accept() never blocks, the passed socket sockfd needs to have the
    * O_NONBLOCK flag set (see socket(7)).
    */

   if (lock)
      {
      U_ASSERT(isPreForked())

      bool block = (num_connection == 0);

      U_INTERNAL_DUMP("block = %b num_connection = %d tot_connection = %d", block, num_connection, U_TOT_CONNECTION)

      if (block != UFile::isBlocking(UEventFd::fd, U_SOCKET_FLAGS))
         {
         lock->lock();

         U_SOCKET_FLAGS = (block ? (U_SOCKET_FLAGS & ~O_NONBLOCK)
                                 : (U_SOCKET_FLAGS |  O_NONBLOCK));

         U_INTERNAL_DUMP("flags = %B", U_SOCKET_FLAGS)

         (void) U_SYSCALL(fcntl, "%d,%d,%d", UEventFd::fd, F_SETFL, U_SOCKET_FLAGS);

         lock->unlock();
         }
      }

   if (socket->acceptClient(UClientImage_Base::socket))
      {
      U_INTERNAL_ASSERT(UClientImage_Base::socket->isConnected())

      // Instructs server to accept connections from the IP address IPADDR. A CIDR mask length can be supplied optionally after
      // a trailing slash, e.g. 192.168.0.0/24, in which case addresses that match in the most significant MASK bits will be allowed.
      // If no options are specified, all clients are allowed. Unauthorized connections are rejected by closing the TCP connection
      // immediately. A warning is logged on the server but nothing is sent to the client.

      if (vallow_IP &&
          UIPAllow::isAllowed(UClientImage_Base::socket->remoteIPAddress().getInAddr(), *vallow_IP) == false)
         {
         U_SRV_LOG("new client connected from %S, connection denied by access list", UClientImage_Base::socket->getRemoteInfo());

         UClientImage_Base::socket->close();

         goto end;
         }

      handlerNewConnection();

      goto end;
      }

   U_INTERNAL_DUMP("flag_loop = %b block_on_accept = %b", flag_loop, block_on_accept)

   if (isLog()   &&
       flag_loop && // check for SIGTERM event...
       (block_on_accept || UClientImage_Base::socket->iState != -EAGAIN)) // NB: to avoid log spurious EAGAIN on accept() with epoll()...
      {
      char buffer[4096];

      const char* msg_error = UClientImage_Base::socket->getMsgError(buffer, sizeof(buffer));

      ULog::log("%saccept new client failed %#.*S\n", mod_name, u_strlen(msg_error), msg_error);
      }

end:
   U_RETURN(U_NOTIFIER_OK);
}

const char* UServer_Base::getNumConnection()
{
   U_TRACE(0, "UServer_Base::getNumConnection()")

   static char buffer[32];

   if (isPreForked()) (void) snprintf(buffer, sizeof(buffer), "(%d/%d)", num_connection, U_TOT_CONNECTION);
   else               (void) snprintf(buffer, sizeof(buffer), "%d",     (isClassic() ? U_TOT_CONNECTION : num_connection));

   U_RETURN(buffer);
}

void UServer_Base::handlerCloseConnection()
{
   U_TRACE(0, "UServer_Base::handlerCloseConnection()")

   U_INTERNAL_ASSERT_POINTER(UClientImage_Base::pClientImage)
// U_ASSERT_EQUALS(UNotifier::isHandler(UClientImage_Base::pClientImage), false)

   --num_connection;

   U_INTERNAL_DUMP("num_connection = %d", num_connection)

   if (preforked_num_kids)
      {
      U_TOT_CONNECTION--;

      U_INTERNAL_DUMP("tot_connection = %d", U_TOT_CONNECTION)
      }

   U_SRV_LOG("client closed connection from %.*s, %s clients still connected",
                     U_STRING_TO_TRACE(*(UClientImage_Base::pClientImage->logbuf)), getNumConnection());
}

bool UServer_Base::handlerTimeoutConnection(void* cimg)
{
   U_TRACE(0, "UServer_Base::handlerTimeoutConnection(%p)", cimg)

   U_INTERNAL_ASSERT_POINTER(cimg)
   U_INTERNAL_ASSERT_POINTER(pthis)

   // NB: check to avoid to delete himself...

   U_INTERNAL_DUMP("pthis = %p handler_event = %p", pthis, handler_event)

   if (cimg == pthis ||
       cimg == handler_event)
      {
      U_RETURN(false);
      }

   U_SRV_LOG_TIMEOUT((UClientImage_Base*)cimg);

   ((UClientImage_Base*)cimg)->resetSocket(USocket::BROKEN);

   U_RETURN(true);
}

void UServer_Base::handlerNewConnection()
{
   U_TRACE(0, "UServer_Base::handlerNewConnection()")

   U_INTERNAL_ASSERT_POINTER(pthis)
   U_INTERNAL_ASSERT_POINTER(UClientImage_Base::socket)

   ++num_connection;

   U_INTERNAL_DUMP("num_connection = %d", num_connection)

   if (preforked_num_kids)
      {
      // --------------------------------------------------------------------------------------------------------------------------
      // PREFORK_CHILD number of child server processes created at startup ( 0 - serialize, no forking
      //                                                                     1 - classic, forking after accept client
      //                                                                    >1 - pool of process serialize plus monitoring process)
      // --------------------------------------------------------------------------------------------------------------------------

      if (isClassic())
         {
         U_INTERNAL_ASSERT_POINTER(proc)

         if (max_Keep_Alive &&
             max_Keep_Alive <= U_TOT_CONNECTION)
            {
            --num_connection;

            U_SRV_LOG("new client connected from %S, connection denied by MAX_KEEP_ALIVE (%d)",
                              UClientImage_Base::socket->getRemoteInfo(), max_Keep_Alive);

            return;
            }

         if (proc->fork() &&
             proc->parent())
            {
            UClientImage_Base::socket->close();

            (void) UProcess::waitpid(); // per evitare piu' di 1 zombie...

            return;
            }

         if (proc->child()) setNotifier(true);
         }

      U_TOT_CONNECTION++;

      U_INTERNAL_DUMP("tot_connection = %d", U_TOT_CONNECTION)
      }

   pthis->newClientImage();

   U_INTERNAL_ASSERT_POINTER(UClientImage_Base::pClientImage)

   U_SRV_LOG("new client connected from %.*s, %s clients currently connected",
                     U_STRING_TO_TRACE(*(UClientImage_Base::pClientImage->logbuf)), getNumConnection());

   UClientImage_Base::run();
}

void UServer_Base::run()
{
   U_TRACE(0, "UServer_Base::run()")

   U_INTERNAL_ASSERT_POINTER(pthis)

   pthis->init();

   UInterrupt::syscall_restart = false;

   UNotifier::exit_loop_wait_event_for_signal = true;

#ifdef HAVE_LIBEVENT
   UInterrupt::setHandlerForSignal( SIGHUP, (sighandler_t)UServer_Base::handlerForSigHUP);  //  sync signal
   UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)UServer_Base::handlerForSigTERM); //  sync signal
#else
   UInterrupt::insert(              SIGHUP, (sighandler_t)UServer_Base::handlerForSigHUP);  // async signal
   UInterrupt::insert(             SIGTERM, (sighandler_t)UServer_Base::handlerForSigTERM); // async signal
#endif

   // --------------------------------------------------------------------------------------------------------------------------
   // PREFORK_CHILD number of child server processes created at startup ( 0 - serialize, no forking
   //                                                                     1 - classic, forking after accept client
   //                                                                    >1 - pool of process serialize plus monitoring process)
   // --------------------------------------------------------------------------------------------------------------------------

   /**
   * Main loop for the parent process with the new preforked implementation.
   * The parent is just responsible for keeping a pool of children and they accept connections themselves...
   **/

   if (isPreForked())
      {
      UTimeVal to_sleep(0L, 500 * 1000L);
      int pid, status, nkids = 0, n = max_Keep_Alive / preforked_num_kids;

      if (n) UNotifier::preallocate(n);

      while (flag_loop)
         {
         while (nkids < preforked_num_kids)
            {
            if (proc->fork() &&
                proc->parent())
               {
               ++nkids;

               U_INTERNAL_DUMP("up to %u children", nkids)

               U_SRV_LOG("started new child (pid %d), up to %u children", proc->pid(), nkids);

               U_INTERNAL_DUMP("block_on_accept = %b tot_connection = %d", block_on_accept, U_TOT_CONNECTION)
               }

            if (proc->child())
               {
               U_INTERNAL_DUMP("block_on_accept = %b tot_connection = %d", block_on_accept, U_TOT_CONNECTION)

               setNotifier(true);

               goto preforked_child;
               }

            /* Don't start them too quickly, or we might overwhelm a machine that's having trouble. */

            to_sleep.nanosleep();
            }

         /* wait for any children to exit, and then start some more */

         pid = UProcess::waitpid(-1, &status, 0);

         if (pid > 0 &&
             flag_loop) // check for SIGTERM event...
            {
            --nkids;

            U_INTERNAL_DUMP("down to %u children", nkids)

            U_SRV_LOG("child (pid %d) exited with value %d (%s), down to %u children", pid, status, UProcess::exitInfo(status), nkids);

            U_INTERNAL_DUMP("block_on_accept = %b tot_connection = %d", block_on_accept, U_TOT_CONNECTION)
            }

         /* Another little safety brake here: since children should not exit
          * too quickly, pausing before starting them should be harmless. */

         to_sleep.nanosleep();
         }

      goto end;
      }

   if (isLog() && isClassic()) ULog::log("waiting for connection\n");

preforked_child:

   while (flag_loop)
      {
      if (UInterrupt::event_signal_pending)
         {
         UInterrupt::callHandlerSignal();

         continue;
         }

      // check if we can go to blocking on accept()...

      if (block_on_accept &&
          isClientConnect() == false)
         {
         if (isLog() && isClassic() == false) ULog::log("waiting for connection\n");

         // here we go to block on accept(), plus fork() - eventually child start here...

         (void) pthis->handlerRead();

         U_INTERNAL_DUMP("flag_loop = %b", flag_loop)

         continue; // may be interrupt...
         }

      if (UNotifier::waitForEvent(ptime) == false) break; // no more events registered...
      }

   if (preforked_num_kids)
      {
end:
      if (proc->parent()) (void) proc->waitAll();
      else
         {
         // NB: needed because only the parent process must close the log...

         if (proc->child() && isLog()) log = 0;
         }
      }
}

UCommand* UServer_Base::loadConfigCommand(UFileConfig& cfg)
{
   U_TRACE(0, "UServer_Base::loadConfigCommand(%p)", &cfg)

   U_ASSERT_EQUALS(cfg.empty(), false)

   UCommand* cmd   = 0;
   UString command = cfg[*str_COMMAND];

   if (command.empty() == false)
      {
      if (U_ENDS_WITH(command, ".sh"))
         {
         UString buffer(U_PATH_MAX);

         buffer.snprintf("%s %.*s", U_PATH_SHELL, U_STRING_TO_TRACE(command));

         command = buffer;
         }

      UString environment = cfg[*str_ENVIRONMENT];

      const UString* penv = (environment.empty() ? 0 : &environment);

      cmd = U_NEW(UCommand(command, penv));
      }

   U_RETURN_POINTER(cmd,UCommand);
}

void UServer_Base::logCommandMsgError(const char* cmd)
{
   U_TRACE(0, "UServer_Base::logCommandMsgError(%S)", cmd)

   if (isLog())
      {
      UCommand::setMsgError(cmd);

      ULog::log("%s%.*s\n", mod_name, u_buffer_len, u_buffer);

      u_buffer_len = 0;
      }
}

// DEBUG

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* UServer_Base::dump(bool reset) const
{
   *UObjectIO::os << "ptr                       " << (void*)ptr                  << '\n'
                  << "port                      " << port                        << '\n'
                  << "flag_loop                 " << flag_loop                   << '\n'
                  << "verify_mode               " << verify_mode                 << '\n'
                  << "cgi_timeout               " << cgi_timeout                 << '\n'
                  << "verify_mode               " << verify_mode                 << '\n'
                  << "num_connection            " << num_connection              << '\n'
                  << "block_on_accept           " << block_on_accept             << '\n'
                  << "preforked_num_kids        " << preforked_num_kids          << '\n'
                  << "flag_use_tcp_optimization " << flag_use_tcp_optimization   << '\n'
                  << "log           (ULog       " << (void*)log                  << ")\n"
                  << "lock          (ULock      " << (void*)lock                 << ")\n"
                  << "socket        (USocket    " << (void*)socket               << ")\n"
                  << "host          (UString    " << (void*)host                 << ")\n"
                  << "server        (UString    " << (void*)&server              << ")\n"
                  << "log_file      (UString    " << (void*)&log_file            << ")\n"
                  << "ca_file       (UString    " << (void*)&ca_file             << ")\n"
                  << "ca_path       (UString    " << (void*)&ca_path             << ")\n"
                  << "allow_IP      (UString    " << (void*)&allow_IP            << ")\n"
                  << "key_file      (UString    " << (void*)&key_file            << ")\n"
                  << "password      (UString    " << (void*)&password            << ")\n"
                  << "cert_file     (UString    " << (void*)&cert_file           << ")\n"
                  << "name_sock     (UString    " << (void*)&name_sock           << ")\n"
                  << "IP_address    (UString    " << (void*)&IP_address          << ")\n"
                  << "document_root (UString    " << (void*)&document_root       << ")\n"
                  << "proc          (UProcess   " << (void*)proc                 << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#endif
