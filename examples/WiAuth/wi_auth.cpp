// wi_auth.cpp
   
#include <ulib/net/server/usp_macro.h>
   
#include <ulib/net/server/plugin/mod_ssi.h>
   
   class WiAuthUser;
   class WiAuthNodog;
   class WiAuthAccessPoint;
   
   static int ap_port;
   static int priority;
   
   static URDB* db_ap;
   static URDB* db_user;
   
   static UString* ap;
   static UString* ip;
   static UString* uid;
   static UString* mac;
   static UString* label;
   static UString* token;
   static UString* policy;
   static UString* address;
   static UString* gateway;
   static UString* dir_reg;
   static UString* auth_domain;
   static UString* portal_name;
   static UString* request_uri;
   static UString* client_address;
   
   static WiAuthUser*         user_rec;
   static WiAuthNodog*       nodog_rec;
   static WiAuthAccessPoint*    ap_rec;
   
   #define NUM_ACCESS_POINT_ATTRIBUTE 4
   
   #define U_LOGGER(fmt,args...) ULog::logger(portal_name->data(), priority, "%.*s: " fmt, U_STRING_TO_TRACE(*request_uri) , ##args)
   
   class WiAuthAccessPoint {
   public:
      // Check for memory error
      U_MEMORY_TEST
   
      // Allocator e Deallocator
      U_MEMORY_ALLOCATOR
      U_MEMORY_DEALLOCATOR
   
      UString label;
      UVector<UString> mac_white_list, group_accounts;
      bool noconsume;
   
      // COSTRUTTORE
   
      WiAuthAccessPoint()
         {
         U_TRACE_REGISTER_OBJECT(5, WiAuthAccessPoint, "")
   
         noconsume = false;
         }
   
      ~WiAuthAccessPoint()
         {
         U_TRACE_UNREGISTER_OBJECT(5, WiAuthAccessPoint)
         }
   
      // SERVICES
   
      UString toString()
         {
         U_TRACE(5, "WiAuthAccessPoint::toString()")
   
         UString x(U_CAPACITY),
                 vec1 = mac_white_list.join(" "),
                 vec2 = group_accounts.join(" ");
   
         x.snprintf("%u %.*s \"%.*s\" \"%.*s\"", noconsume, U_STRING_TO_TRACE(label), U_STRING_TO_TRACE(vec1), U_STRING_TO_TRACE(vec2));
   
         U_RETURN_STRING(x);
         }
   
      void fromVector(const UVector<UString>& vec, uint32_t index)
         {
         U_TRACE(5, "WiAuthAccessPoint::fromVector(%p,%u)", &vec, index)
   
         U_ASSERT_EQUALS(vec.empty(), false)
   
         noconsume = (vec[index].strtol() == 1);
         label     =  vec[index+1];
   
                                                   mac_white_list.clear();
         if (vec[index+2].empty() == false) (void) mac_white_list.split(U_STRING_TO_PARAM(vec[index+2]));
   
                                                   group_accounts.clear();
         if (vec[index+3].empty() == false) (void) group_accounts.split(U_STRING_TO_PARAM(vec[index+3]));
         }
   
   #ifdef DEBUG
      const char* dump(bool breset) const
         {
         *UObjectIO::os << "noconsume                        " << noconsume              << '\n'
                        << "label          (UString>         " << (void*)&label          << ")\n"
                        << "group_accounts (UVector<UString> " << (void*)&group_accounts << ")\n"
                        << "mac_white_list (UVector<UString> " << (void*)&mac_white_list << ')';
   
         if (breset)
            {
            UObjectIO::output();
   
            return UObjectIO::buffer_output;
            }
   
         return 0;
         }
   #endif
   private:
      WiAuthAccessPoint(const WiAuthAccessPoint&)            {}
      WiAuthAccessPoint& operator=(const WiAuthAccessPoint&) { return *this; }
   };
   
   static bool  user_exist;
   static bool nodog_exist;
   
   class WiAuthNodog {
   public:
      // Check for memory error
      U_MEMORY_TEST
   
      // Allocator e Deallocator
      U_MEMORY_ALLOCATOR
      U_MEMORY_DEALLOCATOR
   
      UString value, hostname;
      UVector<UString> access_point;
      int port;
      bool down;
   
      // COSTRUTTORE
   
      WiAuthNodog() : value(U_CAPACITY)
         {
         U_TRACE_REGISTER_OBJECT(5, WiAuthNodog, "")
         }
   
      ~WiAuthNodog()
         {
         U_TRACE_UNREGISTER_OBJECT(5, WiAuthNodog)
         }
   
      // SERVICES
   
      UString toString()
         {
         U_TRACE(5, "WiAuthNodog::toString()")
   
         UString ap_list = access_point.join(" ");
   
         value.setBuffer(U_CAPACITY);
   
         value.snprintf("%u %u %.*s [%.*s]", down, port, U_STRING_TO_TRACE(hostname), U_STRING_TO_TRACE(ap_list));
   
         U_RETURN_STRING(value);
         }
   
      void fromString()
         {
         U_TRACE(5, "WiAuthNodog::fromString()")
   
         U_ASSERT_EQUALS(value.empty(), false)
   
         istrstream is(value.data(), value.size());
   
         is >> down
            >> port;
   
         is.get(); // skip ' '
   
         hostname.get(is);
   
         is.get(); // skip ' '
         is.get(); // skip '['
   
         access_point.clear();
         access_point.readVector(is, ']');
         }
   
      void addAccessPoint()
         {
         U_TRACE(5, "WiAuthNodog::addAccessPoint()")
   
         U_ASSERT_EQUALS(label->empty(), false)
   
         UString empty_list = U_STRING_FROM_CONSTANT("()"),
                 attr1      = U_STRING_FROM_CONSTANT("0"),  // noconsume
                 attr2      = *label,                       // label
                 attr3      = empty_list,                   // mac_white_list
                 attr4      = empty_list;                   // group_accounts
   
         access_point.push_back(attr1);
         access_point.push_back(attr2);
         access_point.push_back(attr3);
         access_point.push_back(attr4);
         }
   
      void setValue()
         {
         U_TRACE(5, "WiAuthNodog::setValue()")
   
         U_ASSERT_EQUALS(address->empty(), false)
   
         value       = (*db_ap)[*address];
         nodog_exist = (value.empty() == false);
   
         if (nodog_exist) fromString();
         }
   
      void store(int op)
         {
         U_TRACE(5, "WiAuthNodog::store(%d)", op)
   
         UString x = toString();
   
         int result = db_ap->store(*address, x, op);
   
         if (result)
            {
            U_SRV_LOG("store with flag %d on db WiAuthNodog failed with error %d", op, result);
            }
   #  ifdef DEBUG
         else
            {
            U_ASSERT_EQUALS(value, (*db_ap)[*address])
            }
   #  endif
         }
   
      bool setRecord()
         {
         U_TRACE(5, "WiAuthNodog::setRecord()")
   
         U_ASSERT_EQUALS(address->empty(), false)
   
         setValue();
   
         int op = -1;
   
         if (nodog_exist)
            {
            if ( ap->empty() == false &&
                *ap != hostname)
               {
               U_LOGGER("*** AP %.*s HOSTNAME NOT EQUAL ***", U_STRING_TO_TRACE(*ap));
               }
   
            if (down)
               {
               op = RDB_REPLACE;
   
               char* _ptr = value.data();
   
               *_ptr = '0';
               down  = false;
               }
   
            if (label->empty()                   == false &&
                access_point.isContained(*label) == false)
               {
               op = RDB_REPLACE;
   
               goto add_ap;
               }
            }
         else
            {
            U_ASSERT_EQUALS(ap->empty(), false)
            U_INTERNAL_ASSERT(u_isHostName(U_STRING_TO_PARAM(*ap)))
   
            op       = RDB_INSERT;
            port     = ap_port;
            down     = false;
            hostname = *ap;
   
            access_point.clear();
   add_ap:
            addAccessPoint();
   
            toString();
            }
   
         if (op != -1) store(op);
   
         U_RETURN(nodog_exist);
         }
   
   #ifdef DEBUG
      const char* dump(bool breset) const
         {
         *UObjectIO::os << "down                           " << down                 << '\n'
                        << "port                           " << port                 << '\n'
                        << "value        (UString          " << (void*)&value        << ")\n"
                        << "hostname     (UString>         " << (void*)&hostname     << ")\n"
                        << "access_point (UVector<UString> " << (void*)&access_point << ')';
   
         if (breset)
            {
            UObjectIO::output();
   
            return UObjectIO::buffer_output;
            }
   
         return 0;
         }
   #endif
   private:
      WiAuthNodog(const WiAuthNodog&)            {}
      WiAuthNodog& operator=(const WiAuthNodog&) { return *this; }
   };
   
   class WiAuthUser {
   public:
      // Check for memory error
      U_MEMORY_TEST
   
      // Allocator e Deallocator
      U_MEMORY_ALLOCATOR
      U_MEMORY_DEALLOCATOR
   
      UString value, _ip, _mac, _user, _token, _policy, _gateway, _auth_domain, nodog;
      uint64_t traffic_done, traffic_available, traffic_consumed;
      time_t time_done, time_available, login_time, last_modified;
      uint32_t access_point_index, agent, group_account;
      bool connected, consume;
   
      // COSTRUTTORE
   
      WiAuthUser() : value(U_CAPACITY)
         {
         U_TRACE_REGISTER_OBJECT(5, WiAuthUser, "")
   
         consume            = true;
         connected          = false;
         traffic_done       = traffic_available = traffic_consumed = 0;
         time_available     = time_done = login_time = last_modified = 0;
         access_point_index = agent = group_account = 0;
         }
   
      ~WiAuthUser()
         {
         U_TRACE_UNREGISTER_OBJECT(5, WiAuthUser)
         }
   
      // SERVICES
   
      UString toString()
         {
         U_TRACE(5, "WiAuthUser::toString()")
   
         value.setBuffer(U_CAPACITY);
   
         value.snprintf("%.*s "
                        "%u %u %u %u %u "
                        "%ld %llu %ld %llu %llu "
                        "%ld %ld "
                        "\"%.*s\" %.*s %.*s %.*s %.*s %.*s %.*s",
                        U_STRING_TO_TRACE(_ip),
                        connected, access_point_index, consume, group_account, agent,
                           time_done,
                        traffic_done,
                           time_available,
                        traffic_available, traffic_consumed,
                        login_time, last_modified,
                        U_STRING_TO_TRACE(_user), U_STRING_TO_TRACE(_auth_domain), U_STRING_TO_TRACE(_mac),
                        U_STRING_TO_TRACE(_gateway), U_STRING_TO_TRACE(_policy), U_STRING_TO_TRACE(_token), U_STRING_TO_TRACE(nodog));
   
         U_RETURN_STRING(value);
         }
   
      void fromString()
         {
         U_TRACE(5, "WiAuthUser::fromString()")
   
         U_ASSERT_EQUALS(value.empty(), false)
   
         istrstream is(value.data(), value.size());
   
         _ip.get(is);
   
         is.get(); // skip ' '
   
         is >> connected
            >> access_point_index
            >> consume
            >> group_account
            >> agent
            >>    time_done
            >> traffic_done
            >>    time_available
            >> traffic_available
            >> traffic_consumed
            >> login_time
            >> last_modified;
   
         is.get(); // skip ' '
   
         _user.get(is);
   
         is.get(); // skip ' '
   
         _auth_domain.get(is);
   
         is.get(); // skip ' '
   
         _mac.get(is);
   
         is.get(); // skip ' '
   
         _gateway.get(is);
   
         is.get(); // skip ' '
   
         _policy.get(is);
   
         is.get(); // skip ' '
   
         _token.get(is);
   
         is.get(); // skip ' '
   
         nodog.get(is);
         }
   
      void setValue()
         {
         U_TRACE(5, "WiAuthUser::setValue()")
   
         U_ASSERT_EQUALS(uid->empty(), false)
   
         value      = (*db_user)[*uid];
         user_exist = (value.empty() == false);
   
         if (user_exist) fromString();
         }
   
      UString getAP()
         {
         U_TRACE(5, "WiAuthUser::getAP()")
   
         *label = nodog_rec->access_point[(access_point_index * NUM_ACCESS_POINT_ATTRIBUTE) + 1];
   
         UString x(100U);
   
         x.snprintf("%.*s@%.*s", U_STRING_TO_TRACE(*label), U_STRING_TO_TRACE(nodog));
   
         U_RETURN_STRING(x);
         }
   
      void store(int op)
         {
         U_TRACE(5, "WiAuthUser::store(%d)", op)
   
         UString x = toString();
   
         int result = db_user->store(*uid, x, op);
   
         if (result)
            {
            U_SRV_LOG("store with flag %d on db WiAuthUser failed with error %d", op, result);
            }
   #  ifdef DEBUG
         else
            {
            U_ASSERT_EQUALS(value, (*db_user)[*uid])
            }
   #  endif
         }
   
      void setRecord()
         {
         U_TRACE(5, "WiAuthUser::setRecord()")
   
         U_INTERNAL_DUMP("user_exist = %b", user_exist)
   
         U_ASSERT_EQUALS(uid->empty(), false)
   
         if (user_exist == false)
            {
            U_ASSERT((*db_user)[*uid].empty())
   
            connected    = false;
            time_done    = 0;
            traffic_done = traffic_consumed = 0;
   
            UString pathname(U_CAPACITY), content;
   
            pathname.snprintf("%.*s/%.*s.reg", U_STRING_TO_TRACE(*dir_reg), U_STRING_TO_TRACE(*uid));
   
            content = UFile::contentOf(pathname);
   
            if (content.empty() == false)
               {
               UVector<UString> vec(content);
   
               _user = vec[0] + ' ' + vec[1];
               }
            }
   
         _ip           = *ip;
         _mac          = *mac;
         nodog         = *address;
         _token        = *token;
         _gateway      = *gateway;
         _policy       = *policy;
         _auth_domain  = *auth_domain;
   
         agent         = UHTTP::getUserAgent();
         consume       = true;
         login_time    = last_modified = 0;
         group_account = access_point_index = 0;
         }
   
   #ifdef DEBUG
      const char* dump(bool breset) const
         {
         *UObjectIO::os << "agent                " << agent                 << '\n'
                        << "consume              " << consume               << '\n'
                        << "connected            " << connected             << '\n'
                        << "time_done            " << time_done             << '\n'
                        << "login_time           " << login_time            << '\n'
                        << "group_account        " << group_account         << '\n'
                        << "traffic_done         " << traffic_done          << '\n'
                        << "last_modified        " << last_modified         << '\n'
                        << "time_available       " << time_available        << '\n'
                        << "traffic_consumed     " << traffic_consumed      << '\n'
                        << "traffic_available    " << traffic_available     << '\n'
                        << "access_point_index   " << access_point_index    << '\n'
                        << "_ip          (UString " << (void*)&_ip          << ")\n"
                        << "_mac         (UString " << (void*)&_mac         << ")\n"
                        << "_user        (UString " << (void*)&_user        << ")\n"
                        << "nodog        (UString " << (void*)&nodog        << ")\n"
                        << "value        (UString " << (void*)&value        << ")\n"
                        << "_token       (UString " << (void*)&_token       << ")\n"
                        << "_policy      (UString " << (void*)&_policy      << ")\n"
                        << "_gateway     (UString " << (void*)&_gateway     << ")\n"
                        << "_auth_domain (UString " << (void*)&_auth_domain << ')';
   
         if (breset)
            {
            UObjectIO::output();
   
            return UObjectIO::buffer_output;
            }
   
         return 0;
         }
   #endif
   private:
      WiAuthUser(const WiAuthUser&)            {}
      WiAuthUser& operator=(const WiAuthUser&) { return *this; }
   };
   
   static uint32_t csize;
   static const char* cptr;
   
   static void check_if_user_exist(UStringRep* key, UStringRep* data)
   {
      U_TRACE(5, "::check_if_user_exist(%.*S,%.*S)", U_STRING_TO_TRACE(*key), U_STRING_TO_TRACE(*data))
   
      if (memcmp(cptr, data->data(), csize) == 0)
         {
         user_rec->value._assign(data);
   
         user_rec->fromString();
   
         if (user_rec->_ip.size() == csize)
            {
            U_ASSERT_EQUALS(user_rec->_ip, *client_address)
   
            user_exist = true;
   
            db_user->stopCallForAllEntry();
            }
         }
   }
   
   static bool check_if_user_is_connected()
   {
      U_TRACE(5, "::check_if_user_is_connected()")
   
      user_exist = false;
   
      if (uid->empty() == false) user_rec->setValue();
      else
         {
         cptr  = client_address->data();
         csize = client_address->size();
   
         db_user->callForAllEntry(check_if_user_exist);
         }
   
      if (user_exist)
         {
         if (user_rec->nodog != *client_address &&
             user_rec->agent != UHTTP::getUserAgent())
            {
            U_LOGGER("*** USER IP %.*s BIND WITH AP %.*s HAS DIFFERENT AGENT ***", U_STRING_TO_TRACE(*client_address), U_STRING_TO_TRACE(user_rec->nodog));
            }
   
         ap->clear();
   
         *address = user_rec->nodog;
   
         if (nodog_rec->setRecord() == false)
            {
            U_LOGGER("*** USER IP %.*s BIND WITH AP %.*s NOT EXISTENT ***", U_STRING_TO_TRACE(*client_address), U_STRING_TO_TRACE(*address));
            }
   
         U_RETURN(user_rec->connected);
         }
   
      U_RETURN(false);
   }
   
   #define GET_admin                 0
   #define GET_card_activation       1
   #define GET_error_ap              2
   #define GET_get_config            3
   #define GET_get_users_info        4
   #define GET_info                  5
   #define GET_logged                6
   #define GET_logged_login_request  7
   #define GET_login                 8
   #define GET_login_request         9
   #define GET_login_validate       10
   #define GET_logout               11
   #define GET_logout_page          12
   #define GET_polling_attivazione  13
   #define GET_postlogin            14
   #define GET_recovery             15
   #define GET_registrazione        16
   #define GET_reset_policy         17
   #define GET_start_ap             18
   #define GET_stato_utente         19
   #define GET_status_ap            20
   #define GET_status_network       21
   #define GET_unifi                22
   #define GET_unifi_login_request  23
   #define GET_view_user            24
   #define GET_webif_ap             25
   
   static UVector<UString>* GET_request;
   static         UString*  GET_request_str;
   
   #define POST_login_request        0
   #define POST_logout               1
   #define POST_registrazione        2
   #define POST_uploader             3
   
   static UVector<UString>* POST_request;
   static         UString*  POST_request_str;
   
   #define VIRTUAL_HOST "wifi-aaa.comune.fi.it"
   
   static UCache*  cache;
   static uint32_t num_ap;
   static UString* body;
   static UString* ap_ref;
   static UString* output;
   static UString* dir_root;
   static UString* telefono;
   static UString* ip_server;
   static UString* login_url;
   static UString* environment;
   static UString* virtual_name;
   static UString* dir_template;
   static UString* ldap_card_param;
   static UString* ldap_user_param;
   static UString* redirect_default;
   static UString* registrazione_url;
   static UString* allowed_web_hosts;
   static UString* historical_log_dir;
   
   static UString* help_url;
   static UString* wallet_url;
   static UString* fmt_auth_cmd;
   static UString* url_banner_ap;
   static UString* url_banner_comune;
   
   static UFile* file_LOG;
   static UFile* url_banner_ap_default;
   static UFile* url_banner_comune_default;
   
   static UHashMap<UString>*       table;
   static UHttpClient<UTCPSocket>* client;
   
   static void count_num_ap(UStringRep* key, UStringRep* data)
   {
      U_TRACE(5, "::count_num_ap(%.*S,%.*S)", U_STRING_TO_TRACE(*key), U_STRING_TO_TRACE(*data))
   
      nodog_rec->value._assign(data);
   
      nodog_rec->fromString();
   
      num_ap += nodog_rec->access_point.size() / NUM_ACCESS_POINT_ATTRIBUTE;
   
      U_INTERNAL_DUMP("num_ap = %u", num_ap)
   }
   
   static void usp_init()
   {
      U_TRACE(5, "::usp_init()")
   
      U_INTERNAL_ASSERT_POINTER(U_LOCK_USER1)
      U_INTERNAL_ASSERT_POINTER(U_LOCK_USER2)
   
      UString pathdb_ap   = U_STRING_FROM_CONSTANT(U_LIBEXECDIR "/WiAuthAccessPoint.cdb"),
              pathdb_user = U_STRING_FROM_CONSTANT(U_LIBEXECDIR "/WiAuthUser.cdb");
   
      db_ap   = U_NEW(URDB(pathdb_ap,   false));
      db_user = U_NEW(URDB(pathdb_user, false));
   
      db_ap->setShared(U_LOCK_USER1);
      db_user->setShared(U_LOCK_USER2);
   
      bool result;
   
      db_ap->lock();
   
      result = db_ap->open(4 * 1024, false);
   
      db_ap->unlock();
   
         ap_rec = U_NEW(WiAuthAccessPoint);
       user_rec = U_NEW(WiAuthUser);
      nodog_rec = U_NEW(WiAuthNodog);
   
      if (UServer_Base::bssl == false)
         {
         db_ap->callForAllEntry(count_num_ap);
   
         U_SRV_LOG("db initialization of wi-auth access point WiAuthAccessPoint.cdb %s: num_ap %u", result ? "success" : "FAILED", num_ap);
   
         UFile::writeToTmpl("/tmp/WiAuthAccessPoint.init", db_ap->print());
         }
   
      db_user->lock();
   
      result = db_user->open(1024 * 1024, false);
   
      db_user->unlock();
   
      if (UServer_Base::bssl == false)
         {
         U_SRV_LOG("db initialization of wi-auth users WiAuthUser.cdb %s", result ? "success" : "FAILED");
   
         UFile::writeToTmpl("/tmp/WiAuthUser.init", db_user->print());
         }
   
      ap                 = U_NEW(UString);
      ip                 = U_NEW(UString);
      uid                = U_NEW(UString);
      mac                = U_NEW(UString);
      body               = U_NEW(UString);
      label              = U_NEW(UString);
      token              = U_NEW(UString);
      ap_ref             = U_NEW(UString(100U));
      output             = U_NEW(UString);
      policy             = U_NEW(UString);
      gateway            = U_NEW(UString);
      address            = U_NEW(UString);
      ip_server          = U_NEW(UString(UServer_Base::getIPAddress()));
      request_uri        = U_NEW(UString);
      auth_domain        = U_NEW(UString);
      client_address     = U_NEW(UString);
      allowed_web_hosts  = U_NEW(UString);
   
      environment        = U_NEW(UString(*USSIPlugIn::environment + "VIRTUAL_HOST=" + VIRTUAL_HOST));
   
      dir_reg            = U_NEW(UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("DIR_REG"),            environment)));
      dir_root           = U_NEW(UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("DIR_ROOT"),           environment)));
      portal_name        = U_NEW(UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("PORTAL_NAME"),        environment)));
      virtual_name       = U_NEW(UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("VIRTUAL_NAME"),       environment)));
      historical_log_dir = U_NEW(UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("HISTORICAL_LOG_DIR"), environment)));
   
      U_INTERNAL_ASSERT(portal_name->isNullTerminated())
   
      UString tmp1 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("LDAP_CARD_PARAM"), environment),
              tmp2 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("LDAP_USER_PARAM"), environment);
   
      ldap_card_param = U_NEW(UString(UStringExt::expandEnvironmentVar(tmp1, environment)));
      ldap_user_param = U_NEW(UString(UStringExt::expandEnvironmentVar(tmp2, environment)));
   
      priority = ULog::getPriorityForLogger(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("LOCAL_SYSLOG_SELECTOR"), environment).data());
   
      GET_request_str = U_NEW(U_STRING_FROM_CONSTANT(
                                 "/admin "
                                 "/card_activation "
                                 "/error_ap "
                                 "/get_config "
                                 "/get_users_info "
                                 "/info "
                                 "/logged "
                                 "/logged_login_request "
                                 "/login "
                                 "/login_request "
                                 "/login_validate "
                                 "/logout "
                                 "/logout_page "
                                 "/polling_attivazione "
                                 "/postlogin "
                                 "/recovery "
                                 "/registrazione "
                                 "/reset_policy "
                                 "/start_ap "
                                 "/stato_utente "
                                 "/status_ap "
                                 "/status_network "
                                 "/unifi "
                                 "/unifi_login_request "
                                 "/view_user "
                                 "/webif_ap"));
   
      GET_request = U_NEW(UVector<UString>(*GET_request_str));
   
      POST_request_str = U_NEW(U_STRING_FROM_CONSTANT(
                                 "/login_request "
                                 "/logout "
                                 "/registrazione "
                                 "/uploader"));
   
      POST_request = U_NEW(UVector<UString>(*POST_request_str));
   
      UString content = UFile::contentOf("$DIR_ROOT/etc/AllowedWebHosts.txt", O_RDONLY, false, environment);
   
      UVector<UString> vec(content);
   
      if (vec.empty() == false)
         {
         *allowed_web_hosts = vec.join(" ") + ' ';
   
         vec.clear();
         }
   
      cache    = U_NEW(UCache);
      content  = U_STRING_FROM_CONSTANT("$DIR_ROOT/etc/" VIRTUAL_HOST "/cache.tmpl");
      file_LOG = U_NEW(UFile(U_STRING_FROM_CONSTANT("$FILE_LOG"), environment));
   
      (void) UServer_Base::addLog(file_LOG);
   
      dir_template = U_NEW(UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("DIR_TEMPLATE"), environment)));
   
      if (cache->open(content, 96 * 1024, environment) == false) cache->loadContentOf(*dir_template);
   
      table = U_NEW(UHashMap<UString>);
   
      table->allocate();
   
      content = UFile::contentOf("$DIR_ROOT/etc/" VIRTUAL_HOST "/script.conf", O_RDONLY, false, environment);
   
      if (UFileConfig::loadProperties(*table, content.data(), content.end()))
         {
         telefono          = U_NEW(UString((*table)["TELEFONO"]));
         fmt_auth_cmd      = U_NEW(UString((*table)["FMT_AUTH_CMD"]));
         redirect_default  = U_NEW(UString((*table)["REDIRECT_DEFAULT"]));
   
         url_banner_ap     = U_NEW(UString(UStringExt::expandPath((*table)["URL_BANNER_AP"],               environment)));
         url_banner_comune = U_NEW(UString(UStringExt::expandPath((*table)["URL_BANNER_COMUNE"],           environment)));
   
         help_url          = U_NEW(UString(UStringExt::expandEnvironmentVar((*table)["HELP_URL"],          environment)));
         login_url         = U_NEW(UString(UStringExt::expandEnvironmentVar((*table)["LOGIN_URL"],         environment)));
         wallet_url        = U_NEW(UString(UStringExt::expandEnvironmentVar((*table)["WALLET_URL"],        environment)));
         registrazione_url = U_NEW(UString(UStringExt::expandEnvironmentVar((*table)["REGISTRAZIONE_URL"], environment)));
   
         UString x(U_CAPACITY);
   
         x.snprintf("$DIR_WEB/" VIRTUAL_HOST "%s/default", url_banner_ap->data());
   
         url_banner_ap_default = U_NEW(UFile(x, environment));
   
         x.snprintf("$DIR_WEB/" VIRTUAL_HOST "%s/default", url_banner_comune->data());
   
         url_banner_comune_default = U_NEW(UFile(x, environment));
   
         if (url_banner_ap_default->stat() == false)
            {
            delete url_banner_ap_default;
                   url_banner_ap_default = 0;
            }
   
         if (url_banner_comune_default->stat() == false)
            {
            delete url_banner_comune_default;
                   url_banner_comune_default = 0;
            }
         }
   
      table->clear();
   
      UDES3::setPassword("vivalatopa");
   
      client = U_NEW(UHttpClient<UTCPSocket>(0));
   }
   
   static void usp_end()
   {
      U_TRACE(5, "::usp_end()")
   
      if (db_ap)
         {
         if (UServer_Base::bssl == false)
            {
            UFile::writeToTmpl("/tmp/WiAuthAccessPoint.end", db_ap->print());
   
            (void) db_ap->closeReorganize();
   
            UFile::writeToTmpl("/tmp/WiAuthUser.end", db_user->print());
   
            (void) db_user->closeReorganize();
            }
   
   #  ifdef DEBUG
         delete ip;
         delete ap;
         delete uid;
         delete mac;
         delete body;
         delete db_ap;
         delete cache;
         delete label;
         delete token;
         delete ap_rec;
         delete ap_ref;
         delete policy;
         delete output;
         delete address;
         delete gateway;
         delete db_user;
         delete dir_reg;
         delete dir_root;
         delete user_rec;
         delete nodog_rec;
         delete ip_server;
         delete auth_domain;
         delete environment;
         delete request_uri;
         delete portal_name;
         delete GET_request;
         delete POST_request;
         delete dir_template;
         delete virtual_name;
         delete client_address;
         delete ldap_card_param;
         delete ldap_user_param;
         delete GET_request_str;
         delete POST_request_str;
         delete allowed_web_hosts;
   
         if (help_url)
            {
            delete telefono;
            delete help_url;
            delete login_url;
            delete wallet_url;
            delete fmt_auth_cmd;
            delete url_banner_ap;
            delete redirect_default;
            delete url_banner_comune;
            delete registrazione_url;
   
            if (url_banner_ap_default)     delete url_banner_ap_default;
            if (url_banner_comune_default) delete url_banner_comune_default;
            }
   
         table->clear();
         table->deallocate();
   
         delete table;
         delete client;
   #  endif
         }
   }
   
   static void quitUsersLogged()
   {
      U_TRACE(5, "::quitUsersLogged()")
   
      // TODO
   }
   
   static void setAccessPointAddress()
   {
      U_TRACE(5, "::setAccessPointAddress()")
   
      uint32_t pos = ap->find('@');
   
      U_ASSERT_DIFFERS(pos, U_NOT_FOUND)
   
      *label   = ap->substr(0U, pos).copy();
      *address = ap->substr(pos + 1).copy();
   }
   
   static void setAccessPointReference()
   {
      U_TRACE(5, "::setAccessPointReference()")
   
      UHTTP::processHTTPForm();
   
      // ------------------------------------------------
      // $7 -> ap (with localization => '@')
      // ------------------------------------------------
   
      UHTTP::getFormValue(*ap, 13);
   
      U_ASSERT_EQUALS(*ap, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("ap"), 0, 14))
   
      setAccessPointAddress();
   
      uint32_t certid  = 0;
      const char* _ptr = address->data();
   
      // Ex: 10.8.1.2
   
      for (uint32_t i = 0, dot_count = 0; dot_count < 3; ++i)
         {
         if (_ptr[i++] == '.')
            {
            ++dot_count;
   
                 if (dot_count == 2) certid  = 254 * strtol(_ptr+i, 0, 10);
            else if (dot_count == 3) certid +=       strtol(_ptr+i, 0, 10);
            }
         }
   
      ap_ref->snprintf("X%04dR%.*s", certid, U_STRING_TO_TRACE(*label));
   
      U_INTERNAL_ASSERT_POINTER(help_url)
      U_INTERNAL_ASSERT_POINTER(wallet_url)
      U_INTERNAL_ASSERT_POINTER(url_banner_ap)
      U_INTERNAL_ASSERT_POINTER(url_banner_comune)
   
      UString banner(U_CAPACITY), x(U_CAPACITY);
   
      if (url_banner_ap_default)
         {
         U_ASSERT(url_banner_ap_default->dir())
   
         x.snprintf("$DIR_WEB/" VIRTUAL_HOST "%s/%s", url_banner_ap->data(), ap_ref->data());
   
         banner = UStringExt::expandPath(x, environment);
         _ptr   = banner.data();
   
         if (UFile::access(_ptr) ||
             UFile::_mkdir(_ptr))
            {
            (void) banner.append(U_CONSTANT_TO_PARAM("/default"));
   
            _ptr = banner.data();
   
            if (UFile::access(_ptr) == false) (void) url_banner_ap_default->symlink(_ptr);
            }
         }
   
      if (url_banner_comune_default)
         {
         U_ASSERT(url_banner_comune_default->dir())
   
         x.snprintf("$DIR_WEB/" VIRTUAL_HOST "%s/%s", url_banner_comune->data(), ap_ref->data());
   
         banner = UStringExt::expandPath(x, environment);
         _ptr   = banner.data();
   
         if (UFile::access(_ptr) ||
             UFile::_mkdir(_ptr))
            {
            (void) banner.append(U_CONSTANT_TO_PARAM("/default"));
   
            _ptr = banner.data();
   
            if (UFile::access(_ptr) == false) (void) url_banner_comune_default->symlink(_ptr);
            }
         }
   }
   
   static bool setAccessPoint(bool localization, bool start)
   {
      U_TRACE(5, "::setAccessPoint(%b,%b)", localization, start)
   
      uint32_t pos;
      UString hostname, public_address;
   
      // $1 -> ap
      // $2 -> public address to contact the access point
      // $3 -> pid (0 => start)
   
      UHTTP::processHTTPForm();
   
      UHTTP::getFormValue(hostname,       1);
      UHTTP::getFormValue(public_address, 3);
   
      U_ASSERT_EQUALS(hostname,       UHTTP::getFormValue(U_CONSTANT_TO_PARAM("ap"),     0, 4))
      U_ASSERT_EQUALS(public_address, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("public"), 0, 4))
   
      if (public_address.empty()) address->clear();
      else
         {
         pos = public_address.find(':');
   
         U_INTERNAL_ASSERT_DIFFERS(pos, U_NOT_FOUND)
   
         *address = public_address.substr(0U, pos).copy();
   
         if (u_isIPv4Addr(U_STRING_TO_PARAM(*address)) == false)
            {
            U_LOGGER("*** ADDRESS AP %.*s NOT VALID ***", U_STRING_TO_TRACE(*address));
   
            U_RETURN(false);
            }
   
         ap_port = public_address.substr(pos+1).strtol();
         }
   
      if (localization)
         {
         pos = hostname.find('@');
   
         U_ASSERT_DIFFERS(pos, U_NOT_FOUND)
   
         *label = hostname.substr(0U, pos).copy();
         *ap    = hostname.substr(pos + 1).copy();
         }
      else
         {
         U_ASSERT_EQUALS(hostname.find('@'), U_NOT_FOUND)
   
         label->clear();
   
         *ap = hostname;
         }
   
      if (u_isHostName(U_STRING_TO_PARAM(*ap)) == false)
         {
         U_LOGGER("*** AP HOSTNAME %.*s NOT VALID ***", U_STRING_TO_TRACE(*ap));
   
         U_RETURN(false);
         }
   
      bool restart = nodog_rec->setRecord();
   
      if (start)
         {
         UString pid(100U);
   
         UHTTP::getFormValue(pid, 5);
   
         U_ASSERT_EQUALS(pid, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("pid"), 0, 6))
   
         U_LOGGER("%.*s %s", U_STRING_TO_TRACE(*ap), (pid.strtol() ? "*** AP CRASHED ***" : "started"));
   
         if (restart) quitUsersLogged();
         }
   
      U_RETURN(true);
   }
   
   static void setAlternativeResponse()
   {
      U_TRACE(5, "::setAlternativeResponse()")
   
      U_http_is_connection_close = U_YES;
   
      USSIPlugIn::alternative_response = 1;
   
      U_INTERNAL_DUMP("body = %.*S", U_STRING_TO_TRACE(*body))
   
      if (body->empty())
         {
         u_http_info.nResponseCode = HTTP_NO_CONTENT;
   
         UHTTP::setHTTPResponse(0, 0);
         }
      else
         {
         u_http_info.nResponseCode = HTTP_OK;
   
         UHTTP::setHTTPResponse(UHTTP::str_ctype_html, body);
         }
   }
   
   static void setAlternativeInclude(const char* title_txt, const char* head_html, const char* body_style, const char* name, uint32_t len, ...)
   {
      U_TRACE(5, "::setAlternativeInclude(%S,%S,%S,%.*S,%u)", title_txt, head_html, body_style, len, name, len)
   
      UString _template = cache->contentOf("%.*s/%.*s", U_STRING_TO_TRACE(*dir_template), len, name);
   
      U_INTERNAL_ASSERT(_template.isNullTerminated())
   
      USSIPlugIn::alternative_include->setBuffer(_template.size() + 256 * 1024);
   
      va_list argp;
      va_start(argp, len);
   
      USSIPlugIn::alternative_include->vsnprintf(_template.data(), argp);
   
      va_end(argp);
   
      u_http_info.nResponseCode = HTTP_NO_CONTENT;
   
      if (title_txt == 0) title_txt = "Firenze WiFi";
   
      UString buffer(U_CAPACITY);
   
                      buffer.snprintf("'TITLE_TXT=%s'\n", title_txt ? title_txt : "Firenze WiFi");
      if (head_html)  buffer.snprintf_add("'HEAD_HTML=%s'\n", head_html);
      if (body_style) buffer.snprintf_add("BODY_STYLE=%s\n",  body_style);
   
      (void) UClientImage_Base::environment->append(buffer);
   }
   
   static void setMessagePage(const char* title_txt, const char* message)
   {
      U_TRACE(5, "::setMessagePage(%S,%S)", title_txt, message)
   
      setAlternativeInclude(title_txt, 0, 0, U_CONSTANT_TO_PARAM("message_page.tmpl"), title_txt, message);
   }
   
   static void setMessagePageWithVar(const char* title_txt, const char* fmt, ...)
   {
      U_TRACE(5, "::setMessagePageWithVar(%S,%S)", title_txt, fmt)
   
      UString buffer(U_CAPACITY);
   
      va_list argp;
      va_start(argp, fmt);
   
      buffer.vsnprintf(fmt, argp);
   
      va_end(argp);
   
      setMessagePage(title_txt, buffer.data());
   }
   
   static void setAlternativeRedirect(const char* fmt, ...)
   {
      U_TRACE(5, "::setAlternativeRedirect(%S)", fmt)
   
      UString format(U_CAPACITY), buffer(U_CAPACITY);
   
      format.snprintf(U_CTYPE_HTML "\r\nRefresh: 0; url=%s\r\n", fmt);
   
      va_list argp;
      va_start(argp, fmt);
   
      buffer.vsnprintf(format.data(), argp);
   
      va_end(argp);
   
      U_http_is_connection_close = U_YES;
      u_http_info.nResponseCode  = HTTP_MOVED_TEMP;
   
      UHTTP::setHTTPResponse(&buffer, 0);
   
      USSIPlugIn::alternative_response = 1;
   }
   
   static void anomalia(uint32_t n)
   {
      U_TRACE(5, "::anomalia(%u)", n)
   
      switch (n)
         {
         case 8: // ask_to_LDAP
            {
            U_LOGGER("*** LDAP NON DISPONIBILE (anomalia 008) ***", 0);
   
            setMessagePage("Servizio LDAP non disponibile", "Servizio LDAP non disponibile (anomalia 008)");
            }
         break;
         }
   }
   
   static bool ask_to_LDAP(UString* pinput, const char* fmt, ...)
   {
      U_TRACE(5, "::ask_to_LDAP(%p,%S)", pinput, fmt)
   
      /*
      ldapsearch -LLL -b ou=cards,o=unwired-portal -x -D cn=admin,o=unwired-portal -w programmer -H ldap://127.0.0.1 waLogin=3386453924
      ---------------------------------------------------------------------------------------------------------------------------------
      dn: waCid=6bc07bf3-a09f-4815-8029-db68f32f4189,ou=cards,o=unwired-portal
      objectClass: top
      objectClass: waCard
      waCid: 6bc07bf3-a09f-4815-8029-db68f32f4189
      waPin: 3386453924
      waCardId: db68f32f4189
      waLogin: 3386453924
      waPassword: {MD5}ciwjVccK0u68vqupEXFukQ==
      waRevoked: FALSE
      waValidity: 0
      waPolicy: DAILY
      waTime: 7200
      waTraffic: 314572800
      ---------------------------------------------------------------------------------------------------------------------------------
      */
   
      static int fd_stderr = UServices::getDevNull("/tmp/ask_to_LDAP.err");
   
      UString buffer(U_CAPACITY);
   
      va_list argp;
      va_start(argp, fmt);
   
      buffer.vsnprintf(fmt, argp);
   
      va_end(argp);
   
      UCommand cmd(buffer);
   
      bool result = cmd.execute(pinput, output, -1, fd_stderr);
   
      UServer_Base::logCommandMsgError(cmd.getCommand());
   
      if (pinput == 0) table->clear();
   
      if (result)
         {
         if (pinput == 0 &&
             output->empty() == false)
            {
            (void) UFileConfig::loadProperties(*table, output->data(), output->end());
            }
   
         U_RETURN(true);
         }
   
      if (UCommand::exit_value == 255) anomalia(8); // Can't contact LDAP server (-1)
   
      U_RETURN(false);
   }
   
   static bool runAuthCmd(const char* _uid, const char* _password)
   {
      U_TRACE(5, "::runAuthCmd(%S,%S)", _uid, _password)
   
      static int fd_stderr = UServices::getDevNull("/tmp/auth_cmd.err");
   
      UString cmd(U_CAPACITY);
   
      cmd.snprintf(fmt_auth_cmd->data(), _uid, _password);
   
      *output = UCommand::outputCommand(cmd, 0, -1, fd_stderr);
   
      UServer_Base::logCommandMsgError(cmd.data());
   
      if (UCommand::exit_value ||
          output->empty())
         {
         U_LOGGER("*** AUTH_CMD fail EXIT_VALUE=%d RESPONSE=%.*S ***", UCommand::exit_value, U_STRING_TO_TRACE(*output));
   
         if (UCommand::exit_value == 1) setMessagePage("Utente e/o Password errato/i", "Credenziali errate!");
         else                           setMessagePage("Errore", "Richiesta autorizzazione ha esito errato");
   
         U_RETURN(false);
         }
   
      U_RETURN(true);
   }
   
   static UString signData(const char* fmt, ...)
   {
      U_TRACE(5, "::signData(%S)", fmt)
   
      UString buffer1(U_CAPACITY),
              buffer2(U_CAPACITY),
              signed_data(U_CAPACITY);
   
      va_list argp;
      va_start(argp, fmt);
   
      buffer1.vsnprintf(fmt, argp);
   
      va_end(argp);
   
        UDES3::encode(buffer1, buffer2);
      UBase64::encode(buffer2, signed_data);
   
      U_RETURN_STRING(signed_data);
   }
   
   static void loginWithProblem()
   {
      U_TRACE(5, "::loginWithProblem()")
   
      U_LOGGER("*** FAILURE: UID %.*s IP %.*s MAC %.*s AP %.*s ***",
                  U_STRING_TO_TRACE(*uid), U_STRING_TO_TRACE(*ip), U_STRING_TO_TRACE(*mac), U_STRING_TO_TRACE(*ap));
   
      setMessagePageWithVar("Login", "Problema in fase di autenticazione. "
                                     "Si prega di riprovare, se il problema persiste contattare: %.*s", U_STRING_TO_TRACE(*telefono));
   }
   
   /* Example
   --------------------------------------------------------------------------------------------------------------------------------------------------------- 
   2012/08/08 14:56:00 op: PASS_AUTH, uid: 3343793489, ap: 00@10.8.1.2, ip: 172.16.1.172, mac: 00:14:a5:6e:9c:cb, timeout: 2033, traffic: 3042 policy: DAILY
   --------------------------------------------------------------------------------------------------------------------------------------------------------- 
   */
   
   static void write_to_LOG(const char* op, time_t time_chunk, uint64_t traffic_chunk)
   {
      U_TRACE(5, "::write_to_LOG(%S,%ld,%llu)", op, time_chunk, traffic_chunk)
   
      char buffer[4096U];
   
      *ap = user_rec->getAP();
   
      (void) U_SYSCALL(write, "%d,%p,%u", file_LOG->getFd(), buffer,
                           u__snprintf(buffer, sizeof(buffer),
                              "%6D op: %s, uid: %.*s, ap: %.*s, ip: %.*s, mac: %.*s, timeout: %ld, traffic: %llu policy: %.*s\n",
                              op,
                              U_STRING_TO_TRACE(*uid),
                              U_STRING_TO_TRACE(*ap),
                              U_STRING_TO_TRACE(user_rec->_ip),
                              U_STRING_TO_TRACE(user_rec->_mac),
                              time_chunk, traffic_chunk,
                              U_STRING_TO_TRACE(user_rec->_policy)));
   }
   
   static UString sendRequestToNodog(const char* fmt, ...)
   {
      U_TRACE(5, "::sendRequestToNodog(%S)", fmt)
   
      U_INTERNAL_ASSERT( user_exist)
      U_INTERNAL_ASSERT(nodog_exist)
   
      UString buffer(U_CAPACITY), url(U_CAPACITY), result;
   
      va_list argp;
      va_start(argp, fmt);
   
      buffer.vsnprintf(fmt, argp);
   
      va_end(argp);
   
      url.snprintf("http://%.*s:%u/%.*s", U_STRING_TO_TRACE(user_rec->nodog), nodog_rec->port, U_STRING_TO_TRACE(buffer));
   
      // NB: we need PREFORK_CHILD > 2
   
      if (client->connectServer(url) &&
          client->sendRequest(result))
         {
         result = client->getContent();
         }
      else
         {
                      nodog_rec->down = true;
         char* _ptr = nodog_rec->value.data();
              *_ptr = '0';
   
         nodog_rec->store(RDB_REPLACE);
         }
   
      U_RETURN_STRING(result);
   }
   
   static void askNodogToLogoutUser()
   {
      U_TRACE(5, "::askNodogToLogoutUser()")
   
      UString signed_data = signData("ip=%.*s&mac=%.*s", U_STRING_TO_TRACE(user_rec->_ip), U_STRING_TO_TRACE(user_rec->_mac));
   
      (void) sendRequestToNodog("logout?%.*s", U_STRING_TO_TRACE(signed_data));
   }
   
   static void redirectToNoDogWithTicket()
   {
      U_TRACE(5, "::redirectToNoDogWithTicket()")
   
      uint32_t i = 0, n = nodog_rec->access_point.size() / NUM_ACCESS_POINT_ATTRIBUTE;
   
      for (; i < n; ++i)
         {
         ap_rec->fromVector(nodog_rec->access_point, i);
   
         if (*label == ap_rec->label)
            {
            user_rec->access_point_index = i;
   
            // TODO: check for group account
   
            if (user_rec->group_account) user_rec->consume = false;
            else                         user_rec->consume = (ap_rec->noconsume == false);
   
            break;
            }
         }
   
      if (i == n)
         {
         U_LOGGER("*** LABEL %.*s NOT EXISTENT ON DB WiAuthAccessPoint.cdb ***", U_STRING_TO_TRACE(*label));
   
         loginWithProblem();
   
         return;
         }
   
      time_t time_chunk;
      uint64_t traffic_chunk;
   
      if (user_rec->consume == false ||
          user_rec->_policy == "FLAT")
         {
            time_chunk = U_ONE_DAY_IN_SECOND;
         traffic_chunk = 4 * 1024 * 1024; // 4G
         }
      else
         {
         if (user_rec->time_done >= user_rec->time_available)
            {
            setMessagePage("Tempo consumato", "Hai consumato il tempo disponibile del servizio!");
   
            return;
            }
   
         if (user_rec->traffic_done >= user_rec->traffic_available)
            {
            setMessagePage("Traffico consumato", "Hai consumato il traffico disponibile del servizio!");
   
            return;
            }
   
            time_chunk = user_rec->time_available    - user_rec->time_done;
         traffic_chunk = user_rec->traffic_available - user_rec->traffic_done;
         }
   
      write_to_LOG(user_rec->_auth_domain.c_str(), time_chunk, traffic_chunk);
   
      UString signed_data = signData("\n"
         "Action Permit\n"
         "Mode Login\n"
         "Redirect http://" VIRTUAL_HOST "/postlogin?%.*s\n"
         "Mac %.*s\n"
         "Timeout %lu\n"
         "Traffic %llu\n"
         "Token %.*s\n"
         "User %.*s",
         U_HTTP_QUERY_TO_TRACE,
         U_STRING_TO_TRACE(*mac),
         time_chunk, traffic_chunk,
         U_STRING_TO_TRACE(user_rec->_token), U_STRING_TO_TRACE(*uid));
   
      setAlternativeRedirect("http://%.*s/ticket?ticket=%.*s", U_STRING_TO_TRACE(*gateway), U_STRING_TO_TRACE(signed_data));
   }
   
   static int checkLoginRequest()
   {
      U_TRACE(5, "::checkLoginRequest()")
   
      // $1 -> mac
      // $2 -> ip
      // $3 -> => UID <=
      // $4 -> gateway
      // $5 -> timeout
      // $6 -> token
      // $7 -> ap (with localization => '@')
   
      UHTTP::getFormValue(*mac, 1);
      UHTTP::getFormValue(*ip,  3);
      UHTTP::getFormValue(*uid, 5);
      UHTTP::getFormValue(*ap, 13);
   
      U_ASSERT_EQUALS(*mac, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("mac"),      0, 14))
      U_ASSERT_EQUALS(*ip,  UHTTP::getFormValue(U_CONSTANT_TO_PARAM("ip"),       0, 14))
      U_ASSERT_EQUALS(*uid, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("redirect"), 0, 14))
      U_ASSERT_EQUALS(*ap,  UHTTP::getFormValue(U_CONSTANT_TO_PARAM("ap"),       0, 14))
   
      setAccessPointAddress();
   
      if (check_if_user_is_connected())
         {
         // TODO: wait for user logout ???
   
         loginWithProblem();
   
         U_RETURN(-1);
         }
   
      if ( user_exist == false ||
          nodog_exist == false)
         {
         loginWithProblem();
   
         U_RETURN(-1);
         }
   
      U_ASSERT_EQUALS(*address, user_rec->nodog)
   
      UHTTP::getFormValue(*gateway,  7);
      UHTTP::getFormValue(*token,   11);
   
      U_ASSERT_EQUALS(*gateway, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("gateway"), 0, 14))
      U_ASSERT_EQUALS(*token,   UHTTP::getFormValue(U_CONSTANT_TO_PARAM("token"),   0, 14))
   
      if ((*ip      != user_rec->_ip)    ||
          (*mac     != user_rec->_mac)   ||
          (*token   != user_rec->_token) ||
          (*gateway != user_rec->_gateway))
         {
         U_LOGGER("*** DIFFERENCE: IP(%.*s=>%.*s) MAC(%.*s=>%.*s) TOKEN(%.*s=>%.*s) GATEWAY(%.*s=>%.*s) ***",
                     U_STRING_TO_TRACE(*ip),      U_STRING_TO_TRACE(user_rec->_ip),
                     U_STRING_TO_TRACE(*mac),     U_STRING_TO_TRACE(user_rec->_mac),
                     U_STRING_TO_TRACE(*token),   U_STRING_TO_TRACE(user_rec->_token),
                     U_STRING_TO_TRACE(*gateway), U_STRING_TO_TRACE(user_rec->_gateway));
   
         U_RETURN(1);
         }
   
      U_RETURN(0);
   }  
   
extern "C" {
extern U_EXPORT int runDynamicPage(UClientImage_Base* client_image);
       U_EXPORT int runDynamicPage(UClientImage_Base* client_image)
{
   U_TRACE(0, "::runDynamicPage(%p)", client_image)
   
   if (client_image == 0)         { usp_init();  U_RETURN(0); }
   
   if (client_image == (void*)-1) {              U_RETURN(0); }
   
   if (client_image == (void*)-2) { usp_end();   U_RETURN(0); }
   
   uint32_t usp_sz;
   char usp_buffer[10 * 4096];
   bool usp_as_service = (client_image == (UClientImage_Base*)-3);
   
   *request_uri    = UHTTP::getRequestURI(false);
   *client_address = UServer_Base::getClientAddress();
   
   if (UHTTP::isHttpGET())
      {
      switch (GET_request->findSorted(*request_uri))
         {
         case GET_admin:
            {
            setAlternativeRedirect("https://%.*s/admin.html", U_STRING_TO_TRACE(*ip_server));
            }
         break;
   
         case GET_card_activation:
            {
            /*
            if (*ip_server == *client_address)
               {
               }
            */
            }
         break;
   
         case GET_error_ap:
            {
            // $1 -> ap (with localization => '@')
            // $2 -> public address to contact the access point
   
            if (setAccessPoint(true, false))
               {
               U_LOGGER("*** ON AP %.*s FIREWALL IS NOT ALIGNED ***", U_STRING_TO_TRACE(*ap));
               }
   
            body->clear();
   
            setAlternativeResponse();
            }
         break;
   
         case GET_get_config:
            {
            // $1 -> ap (without localization => '@')
            // $2 -> key
   
            UString key;
   
            UHTTP::processHTTPForm();
   
            UHTTP::getFormValue(key, 3);
   
            U_ASSERT_EQUALS(key, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("key"), 0, 4))
   
            ap_ref->snprintf("%.*s", U_STRING_TO_TRACE(key));
   
            UFileConfig cfg;
            UString pathname(U_CAPACITY);
   
            pathname.snprintf("%w/ap/%.*s/nodog.conf", U_STRING_TO_TRACE(*ap_ref));
   
            if (cfg.open(pathname)) *body = cfg.getData();
            else
               {
               pathname.snprintf("%w/ap/default/nodog.conf", 0);
   
               *body = UFile::contentOf(pathname);
               }
   
            setAlternativeResponse();
            }
         break;
   
         case GET_get_users_info:
            {
            /*
            if (*ip_server == *client_address)
               {
               }
            */
            }
         break;
   
         case GET_info:
            {
            UHTTP::processHTTPForm();
   
            uint32_t num_args = UHTTP::form_name_value->size() / 2;
   
            U_INTERNAL_DUMP("num_args = %u", num_args)
   
            for (int32_t i = 0, n = num_args / 2; i < n; i += 8)
               {
               // ----------------------------------------------------------------------------------------------------------------------------------
               // $1 -> mac
               // $2 -> ip
               // $3 -> gateway
               // $4 -> ap (with localization => '@')
               // $5 -> => UID <=
               // $6 -> logout
               // $7 -> connected
               // $8 -> traffic
               // ----------------------------------------------------------------------------------------------------------------------------------
               // /info?Mac=00%3A14%3Aa5%3A6e%3A9c%3Acb&ip=172.16.1.172&gateway=172.16.1.254%3A5280&ap=00%4010.8.1.2&User=3343793489&logout=0&conn...
               // ----------------------------------------------------------------------------------------------------------------------------------
   
               UHTTP::getFormValue(*mac,     i+1);
               UHTTP::getFormValue(*ip,      i+3);
               UHTTP::getFormValue(*gateway, i+5);
               UHTTP::getFormValue(*ap,      i+7);
               UHTTP::getFormValue(*uid,     i+9);
   
               U_ASSERT_EQUALS(*mac,     UHTTP::getFormValue(U_CONSTANT_TO_PARAM("Mac"),     i, i+16))
               U_ASSERT_EQUALS(*ip,      UHTTP::getFormValue(U_CONSTANT_TO_PARAM("ip"),      i, i+16))
               U_ASSERT_EQUALS(*gateway, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("gateway"), i, i+16))
               U_ASSERT_EQUALS(*ap,      UHTTP::getFormValue(U_CONSTANT_TO_PARAM("ap"),      i, i+16))
               U_ASSERT_EQUALS(*uid,     UHTTP::getFormValue(U_CONSTANT_TO_PARAM("User"),    i, i+16))
   
               setAccessPointAddress();
   
               if (check_if_user_is_connected() == false)
                  {
                  U_LOGGER("*** INFO PARAM: CLIENT %.*s IP %.*s AP %.*s MAC %.*s IS NOT CONNECTED ***",
                           U_STRING_TO_TRACE(*uid), U_STRING_TO_TRACE(*ip), U_STRING_TO_TRACE(*ap), U_STRING_TO_TRACE(*mac));
   
                  continue;
                  }
   
               if ((*ip      != user_rec->_ip)  ||
                   (*mac     != user_rec->_mac) ||
                   (*gateway != user_rec->_gateway))
                  {
                  U_LOGGER("*** INFO PARAM DIFFERENCE: AP %.*s CLIENT %.*s IP(%.*s=>%.*s) MAC(%.*s=>%.*s) GATEWAY(%.*s=>%.*s) ***",
                              U_STRING_TO_TRACE(*ap), U_STRING_TO_TRACE(*uid),
                              U_STRING_TO_TRACE(*ip),      U_STRING_TO_TRACE(user_rec->_ip),
                              U_STRING_TO_TRACE(*mac),     U_STRING_TO_TRACE(user_rec->_mac),
                              U_STRING_TO_TRACE(*gateway), U_STRING_TO_TRACE(user_rec->_gateway));
   
                  continue;
                  }
   
               UString logout, connected, traffic;
   
               UHTTP::getFormValue(logout,    i+11);
               UHTTP::getFormValue(connected, i+13);
               UHTTP::getFormValue(traffic,   i+15);
   
               U_ASSERT_EQUALS(logout,    UHTTP::getFormValue(U_CONSTANT_TO_PARAM("logout"),    i, i+16))
               U_ASSERT_EQUALS(connected, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("connected"), i, i+16))
               U_ASSERT_EQUALS(traffic,   UHTTP::getFormValue(U_CONSTANT_TO_PARAM("traffic"),   i, i+16))
   
               time_t _logout    =    logout.strtol(),
                      _connected = connected.strtol(),
                      _traffic   =   traffic.strtol();
   
               if (_traffic == 0 &&
                   _logout  == 0) // NB: _logout == 0 mean NOT logout (only info)...
                  {
                  // NB: no traffic and no logout => logout implicito
   
                  U_LOGGER("*** INFO PARAM: CLIENT %.*s IP %.*s AP %.*s MAC %.*s IMPLICIT LOGOUT ***",
                           U_STRING_TO_TRACE(*uid), U_STRING_TO_TRACE(*ip), U_STRING_TO_TRACE(*ap), U_STRING_TO_TRACE(*mac));
   
                  askNodogToLogoutUser();
   
                  continue;
                  }
   
               time_t time_consumed = u_now->tv_sec - user_rec->last_modified;
   
               if (_connected)
                  {
                  time_t time_diff = time_consumed - _connected;
   
                  if (time_diff >  30 ||
                      time_diff < -30)
                     {
                     U_LOGGER("*** INFO PARAM: CLIENT %.*s IP %.*s AP %.*s MAC %.*s EXCEED DELTA TIME (30 sec) ***",
                              U_STRING_TO_TRACE(*uid), U_STRING_TO_TRACE(*ip), U_STRING_TO_TRACE(*ap), U_STRING_TO_TRACE(*mac));
                     }
                  }
   
               bool ask_for_logout = false;
   
               user_rec->traffic_consumed += _traffic;
   
               if (user_rec->consume)
                  {
                  user_rec->time_done    += _connected;
                  user_rec->traffic_done += _traffic;
   
                  if (user_rec->time_done    >= user_rec->time_available ||
                      user_rec->traffic_done >= user_rec->traffic_available)
                     {
                     ask_for_logout = true;
                     }
                  }
   
               if (_logout != 0) // LOGOUT (-1 => implicito)
                  {
                  ask_for_logout = false;
   
                  user_rec->connected = false;
   
                  time_t      time_chunk = user_rec->time_available    - user_rec->time_done;
                  uint64_t traffic_chunk = user_rec->traffic_available - user_rec->traffic_done;
   
                  write_to_LOG(_logout == -1 ? "EXIT" : "LOGOUT", time_chunk, traffic_chunk);
                  }
   
               user_rec->last_modified = u_now->tv_sec;
   
               user_rec->store(RDB_REPLACE);
   
               if (ask_for_logout) askNodogToLogoutUser();
               }
   
            body->clear();
   
            setAlternativeResponse();
            }
         break;
   
         case GET_logged:
            {
                uid->clear();
            address->clear();
   
            if (check_if_user_is_connected())
               {
   is_logged:
               *ap = user_rec->getAP();
   
               setAlternativeInclude(0, 0, 0, U_CONSTANT_TO_PARAM("logged.tmpl"),
                                       url_banner_ap->data(),
                                       help_url->data(), wallet_url->data(),
                                       ap->data(), "/logged_login_request",
                                       url_banner_comune->data());
               }
            else
               {
               setAlternativeRedirect("http://www.google.com", 0);
               }
            }
         break;
   
         case GET_logged_login_request:
            {
            setAlternativeInclude(0, 0, 0, U_CONSTANT_TO_PARAM("logged_login_request.tmpl"), 0);
            }
         break;
   
         case GET_login: // MAIN PAGE
            {
            // -----------------------------------------------------------------------------------------------------------------------------------------------
            // GET /login?mac=00%3A14%3AA5%3A6E%3A9C%3ACB&ip=192.168.226.2&redirect=http%3A%2F%2Fgoogle&gateway=192.168.226.1%3A5280&timeout=0&token=x&ap=lab2
            // -----------------------------------------------------------------------------------------------------------------------------------------------
            // $1 -> mac
            // $2 -> ip
            // $3 -> redirect
            // $4 -> gateway
            // $5 -> timeout
            // $6 -> token
            // $7 -> ap (with localization => '@')
            // -----------------------------------------------------------------------------
            // 00:e0:4c:d4:63:f5 10.30.1.105 http://google 10.30.1.131:5280 0 x ap@lab2
            // -----------------------------------------------------------------------------
   
            setAccessPointReference();
   
            uid->clear();
   
            if (check_if_user_is_connected()) goto is_logged;
   
            UString request(U_CAPACITY);
   
            request.snprintf("/login_request?%.*s", U_HTTP_QUERY_TO_TRACE);
   
            setAlternativeInclude(0, 0, 0, U_CONSTANT_TO_PARAM("login.tmpl"),
                                    url_banner_ap->data(), ap_ref->data(),
                                    help_url->data(), wallet_url->data(),
                                    ap->c_str(), request.data(),
                                    url_banner_comune->data(), ap_ref->data());
            }
         break;
   
         case GET_login_request:
            {
            // $1 -> mac
            // $2 -> ip
            // $3 -> redirect
            // $4 -> gateway
            // $5 -> timeout
            // $6 -> token
            // $7 -> ap (with localization => '@')
   
            UHTTP::processHTTPForm();
   
            UString redirect, timeout;
   
            UHTTP::getFormValue(*mac,     1);
            UHTTP::getFormValue(*ip,      3);
            UHTTP::getFormValue(redirect, 5);
            UHTTP::getFormValue(*gateway, 7);
            UHTTP::getFormValue(timeout,  9);
            UHTTP::getFormValue(*token,  11);
            UHTTP::getFormValue(*ap,     13);
   
            U_ASSERT_EQUALS(*mac,     UHTTP::getFormValue(U_CONSTANT_TO_PARAM("mac"),      0, 14))
            U_ASSERT_EQUALS(*ip,      UHTTP::getFormValue(U_CONSTANT_TO_PARAM("ip"),       0, 14))
            U_ASSERT_EQUALS(redirect, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("redirect"), 0, 14))
            U_ASSERT_EQUALS(*gateway, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("gateway"),  0, 14))
            U_ASSERT_EQUALS(timeout,  UHTTP::getFormValue(U_CONSTANT_TO_PARAM("timeout"),  0, 14))
            U_ASSERT_EQUALS(*token,   UHTTP::getFormValue(U_CONSTANT_TO_PARAM("token"),    0, 14))
            U_ASSERT_EQUALS(*ap,      UHTTP::getFormValue(U_CONSTANT_TO_PARAM("ap"),       0, 14))
   
            setAlternativeInclude(0, 0, 0, U_CONSTANT_TO_PARAM("login_request.tmpl"), login_url->data(),
                                  mac->c_str(), ip->c_str(), redirect.c_str(), gateway->c_str(), timeout.c_str(), token->c_str(), ap->c_str());
            }
         break;
   
         case GET_login_validate:
            {
            // ---------------------------------------------------------------------------------------------------
            // NB: come back from the gateway (NoDog) with the params given by POST of login_request (except 3)...
            // ---------------------------------------------------------------------------------------------------
            // $1 -> mac
            // $2 -> ip
            // $3 -> => UID <=
            // $4 -> gateway
            // $5 -> timeout
            // $6 -> token
            // $7 -> ap (with localization => '@')
   
            UHTTP::processHTTPForm();
   
            int result = checkLoginRequest();
   
            if (result == -1) break;
            if (result ==  1)
               {
               user_rec->_ip      = *ip;
               user_rec->_mac     = *mac;
               user_rec->_token   = *token;
               user_rec->_gateway = *gateway;
   
               user_rec->store(RDB_REPLACE);
               }
   
            redirectToNoDogWithTicket(); // redirect back to the gateway appending a signed ticket that will signal NoDog to unlock the firewall...
            }
         break;
   
         case GET_logout:
            {
                uid->clear();
            address->clear();
   
            if (check_if_user_is_connected() == false) setMessagePage("Utente non connesso", "Utente non connesso");
            else
               {
               askNodogToLogoutUser();
   
               setAlternativeInclude(0, 0, 0, U_CONSTANT_TO_PARAM("ringraziamenti.tmpl"),
                                       uid->data(), user_rec->time_available / 60, user_rec->traffic_available / (1024 * 1024));
               }
            }
         break;
   
         case GET_logout_page:
            {
            setAlternativeInclude(0, 0, 0, U_CONSTANT_TO_PARAM("logout_page.tmpl"), 0);
            }
         break;
   
         case GET_polling_attivazione:
         break;
   
         case GET_postlogin:
            {
            UHTTP::processHTTPForm();
   
            uint32_t num_args = UHTTP::form_name_value->size() / 2;
   
            U_INTERNAL_DUMP("num_args = %u", num_args)
   
            switch (num_args)
               {
               case 7:
                  {
                  // ----------------------------------------------------------------------------------------------
                  // NB: come back from the gateway (NoDog) with the params given by redirectToNoDogWithTicket()...
                  // ----------------------------------------------------------------------------------------------
                  // $1 -> mac
                  // $2 -> ip
                  // $3 -> => UID <=
                  // $4 -> gateway
                  // $5 -> timeout
                  // $6 -> token
                  // $7 -> ap (with localization => '@')
                  // ----------------------------------------------------------------------------------------------
   
                  int result = checkLoginRequest();
   
                  if (result == -1) break;
   
                  user_rec->connected  = true;
                  user_rec->login_time = user_rec->last_modified = u_now->tv_sec; 
   
                  user_rec->store(RDB_REPLACE);
   
                  write_to_LOG("LOGIN", 0, 0);
   
                  // NB: send as response the message of waiting for redirect to google...
   
                  UString buffer(100U);
                  const char* ptr1 = uid->c_str();
                  const char* ptr2 = redirect_default->data();
   
                  buffer.snprintf("onload=\"doOnLoad('postlogin?uid=%s','%s')\"", ptr1, ptr2); 
   
                  U_http_is_connection_close = U_YES;
   
                  setAlternativeInclude(0,
                                        "<script type=\"text/javascript\" src=\"js/logout_popup.js\"></script>",
                                        buffer.data(),
                                        U_CONSTANT_TO_PARAM("postlogin.tmpl"), ptr1, ptr2, ptr2);
                  }
               break;
   
               case 1:
                  {
                  // ----------------------------
                  // $1 -> uid
                  // ----------------------------
   
                  UHTTP::getFormValue(*uid, 1);
   
                  U_ASSERT_EQUALS(*uid, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("uid"), 0, 2))
   
                  U_http_is_connection_close = U_YES;
   
                  const char* _ptr = uid->c_str();
   
                  setAlternativeInclude("Logout popup", 0, 0, U_CONSTANT_TO_PARAM("logout_popup.tmpl"), _ptr, _ptr);
                  }
               break;
               }
            }
         break;
   
         case GET_recovery:
            {
            /*
            if (*ip_server == *client_address)
               {
               }
            */
            }
         break;
   
         case GET_registrazione:
            {
            // $1 -> ap (with localization => '@')
   
            U_http_is_connection_close = U_YES;
   
            UString tutela_dati = cache->contentOf("%.*s/tutela_dati.txt", U_STRING_TO_TRACE(*dir_template));
   
            U_INTERNAL_ASSERT(tutela_dati.isNullTerminated())
   
            setAlternativeInclude("Registrazione utente",
                                  "<link type=\"text/css\" href=\"css/livevalidation.css\" rel=\"stylesheet\">"
                                  "<script type=\"text/javascript\" src=\"js/livevalidation_standalone.compressed.js\"></script>",
                                  0, U_CONSTANT_TO_PARAM("registrazione.tmpl"), registrazione_url->data(), tutela_dati.data());
            }
         break;
   
         case GET_reset_policy:
            {
            /*
            if (*ip_server == *client_address)
               {
               }
            */
            }
         break;
   
         case GET_start_ap:
            {
            // $1 -> ap (with localization => '@')
            // $2 -> public address to contact the access point
            // $3 -> pid (0 => start)
   
            (void) setAccessPoint(true, true);
   
            *body = *allowed_web_hosts;
   
            setAlternativeResponse();
            }
         break;
   
         case GET_stato_utente:
         break;
   
         case GET_status_ap:
            {
            // $1 -> ap (with localization => '@')
            // $2 -> public address to contact the access point
   
            if (virtual_name->equal(U_HTTP_VHOST_TO_PARAM) &&
                setAccessPoint(true, false))
               {
               }
            }
         break;
   
         case GET_status_network:
            {
            /*
            if (*ip_server == *client_address)
               {
               }
            */
            }
         break;
   
         case GET_unifi:
            {
            setAlternativeInclude(0, 0, 0, U_CONSTANT_TO_PARAM("unifi_page.tmpl"),
                                       url_banner_ap->data(),
                                       help_url->data(), wallet_url->data(),
                                       "unifi", "/unifi_login_request",
                                       url_banner_comune->data());
            }
         break;
   
         case GET_unifi_login_request:
            {
            setAlternativeInclude(0, 0, 0, U_CONSTANT_TO_PARAM("unifi_login_request.tmpl"), 0);
            }
         break;
   
         case GET_view_user:
            {
            /*
            if (*ip_server == *client_address)
               {
               }
            */
            }
         break;
   
         case GET_webif_ap:
            {
            // $1 -> ap (with localization => '@')
            // $2 -> public address to contact the access point
   
            if (virtual_name->equal(U_HTTP_VHOST_TO_PARAM) &&
                setAccessPoint(true, false))
               {
               UString dest(U_CAPACITY);
   
               dest.snprintf("%.*s/client/%s:%u.srv", U_STRING_TO_TRACE(*dir_root), client_address->data(), UHTTP::getUserAgent());
   
               (void) UFile::writeTo(dest, *address);
   
               setAlternativeRedirect("http://" VIRTUAL_HOST "/cgi-bin/webif/status-basic.sh?cat=Status", 0);
               }
            }
         break;
   
         default:
            u_http_info.nResponseCode = HTTP_BAD_REQUEST;
         break;
         }
      }
   else if (UHTTP::isHttpPOST())
      {
      switch (POST_request->contains(*request_uri))
         {
         case POST_login_request:
            {
            // $1  -> mac
            // $2  -> ip
            // $3  -> redirect
            // $4  -> gateway
            // $5  -> timeout
            // $6  -> token
            // $7  -> ap (with localization => '@')
            // $8  -> realm
            // $9  -> uid
            // $10 -> password
            // $11 -> bottone
   
            UHTTP::processHTTPForm();
   
            UString realm;
   
            UHTTP::getFormValue(realm, 15);
   
            U_ASSERT_EQUALS(realm, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("realm"), 0, 22))
   
            if (realm != "10_piazze" &&
                realm != "auth_service")
               {
               setMessagePage("Errore", "Errore Autorizzazione - dominio sconosciuto");
   
               break;
               }
   
            UString password;
   
            UHTTP::getFormValue(*uid,     17);
            UHTTP::getFormValue(password, 19);
   
            U_ASSERT_EQUALS(*uid,     UHTTP::getFormValue(U_CONSTANT_TO_PARAM("uid"),  0, 22))
            U_ASSERT_EQUALS(password, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("pass"), 0, 22))
   
            if (    uid->empty() ||
                password.empty())
               {
               setMessagePage("Impostare utente e/o password", "Impostare utente e/o password"); 
   
               break;
               }
   
            UString wiauth_card_basedn = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("WIAUTH_CARD_BASEDN"), environment);
   
            if (ask_to_LDAP(0, "ldapsearch -LLL -b %.*s %.*s waLogin=%.*s",
                              U_STRING_TO_TRACE(wiauth_card_basedn),
                              U_STRING_TO_TRACE(*ldap_card_param),
                              U_STRING_TO_TRACE(*uid)) == false)
               {
               break;
               }
   
            UString password_on_ldap = (*table)["waPassword"]; // waPassword: {MD5}ciwjVccK0u68vqupEXFukQ==
   
            if (U_STRNEQ(password_on_ldap.data(), "{MD5}") == false)
               {
               // if realm is 'auth_service' and not MD5 password check credential by AUTH command...
   
               if (realm == "10_piazze")
                  {
                  setMessagePage("Utente e/o Password errato/i", "Credenziali errate!");
   
                  break;
                  }
   
               if (runAuthCmd(uid->c_str(), password.c_str()) == false) break;
   
               *policy      = U_STRING_FROM_CONSTANT("DAILY");
               *auth_domain = U_STRING_FROM_CONSTANT("AUTH_") + *output;
               }
            else
               {
               UString passwd(33U);
   
               // Check 1: Wrong user and/or password
   
               UServices::generateDigest(U_HASH_MD5, 0, (unsigned char*)U_STRING_TO_PARAM(password), passwd, true);
   
               if (strncmp(password_on_ldap.c_pointer(U_CONSTANT_SIZE("{MD5}")), U_STRING_TO_PARAM(passwd)))
                  {
                  setMessagePage("Utente e/o Password errato/i", "Credenziali errate!");
   
                  break;
                  }
   
               // Check 2: Activation required
   
               if ((*table)["waUsedBy"].empty()) // waUsedBy: 3343793489
                  {
                  setMessagePage("Attivazione non effettuata", "Per utilizzare il servizio e' richiesta l'attivazione");
   
                  break;
                  }
   
               // Check 3: Card revoked
   
               if ((*table)["waRevoked"] != "FALSE") // waRevoked: FALSE
                  {
                  setMessagePage("Carta revocata", "La tua carta e' revocata!");
   
                  break;
                  }
   
               UString NOT_AFTER = (*table)["waNotAfter"]; // waNotAfter: 20371231235959Z
   
               if (NOT_AFTER.empty() == false)
                  {
                  u_gettimeofday();
   
                  // Check 4: Expired validity
   
                  if (UTimeDate::getSecondFromTime(NOT_AFTER.data(), true, "%4u%2u%2u%2u%2u%2uZ") <= u_now->tv_sec)
                     {
                     setMessagePage("Validita' scaduta", "La tua validita' e' scaduta!");
   
                     break;
                     }
   
                  *auth_domain = U_STRING_FROM_CONSTANT("PASS_AUTH");
                  }
               else
                  {
                  *auth_domain = U_STRING_FROM_CONSTANT("FIRST_PASS_AUTH");
   
                  // Update card with a new generated waNotAfter
   
                  UString DN       = (*table)["dn"],         // dn: waCid=80e415bc-4be0-4385-85ee-970aa1f52ef6,ou=cards,o=unwired-portal
                          VALIDITY = (*table)["waValidity"]; // waValidity: 0
   
                  if (VALIDITY == "0") NOT_AFTER = U_STRING_FROM_CONSTANT("20371231235959Z");
                  else
                     {
                     UTimeDate t;
   
                     t.addDays(VALIDITY.strtol());
   
                     NOT_AFTER = t.strftime("%Y%m%d%H%M%SZ");
                     }
   
                  UString input(U_CAPACITY);
   
                  input.snprintf("dn: %.*s\n"
                                 "changetype: modify\n"
                                 "add: waNotAfter\n"
                                 "waNotAfter: %.*s\n"
                                 "-",
                                 U_STRING_TO_TRACE(DN),
                                 U_STRING_TO_TRACE(NOT_AFTER));
   
                  if (ask_to_LDAP(&input, "ldapmodify -c %.*s", U_STRING_TO_TRACE(*ldap_card_param)) == false) break;
                  }
   
               *policy = (*table)["waPolicy"]; // waPolicy: DAILY
               }
   
            UHTTP::getFormValue(*ip, 3);
   
            U_ASSERT_EQUALS(*ip, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("ip"), 0, 22))
   
            if (*ip != *client_address)
               {
               U_LOGGER("*** QUERY PARAM IP %.*s FROM AP %.*s IS DIFFERENT FROM CLIENT ADDRESS %.*s ***",
                           U_STRING_TO_TRACE(*ip), U_STRING_TO_TRACE(*ap), U_STRING_TO_TRACE(*client_address));
               }
   
            UHTTP::getFormValue(*mac,      1);
            UHTTP::getFormValue(*gateway,  7);
            UHTTP::getFormValue(*token,   11);
            UHTTP::getFormValue(*ap,      13);
   
            U_ASSERT_EQUALS(*mac,     UHTTP::getFormValue(U_CONSTANT_TO_PARAM("mac"),     0, 22))
            U_ASSERT_EQUALS(*gateway, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("gateway"), 0, 22))
            U_ASSERT_EQUALS(*token,   UHTTP::getFormValue(U_CONSTANT_TO_PARAM("token"),   0, 22))
            U_ASSERT_EQUALS(*ap,      UHTTP::getFormValue(U_CONSTANT_TO_PARAM("ap"),      0, 22))
   
            setAccessPointAddress();
   
            if (check_if_user_is_connected())
               {
               // Check change of connection context for same user id (RENEW)
   
               if ((*ip      == user_rec->_ip)  &&
                   (*mac     == user_rec->_mac) &&
                   (*address == user_rec->nodog))
                  {
                  setMessagePage("Login", "Sei già loggato! (login_request)");
   
                  break;
                  }
   
               U_LOGGER("*** RENEW: IP(%.*s=>%.*s) MAC(%.*s=>%.*s) ADDRESS(%.*s=>%.*s) ***",
                           U_STRING_TO_TRACE(*ip),      U_STRING_TO_TRACE(user_rec->_ip),
                           U_STRING_TO_TRACE(*mac),     U_STRING_TO_TRACE(user_rec->_mac),
                           U_STRING_TO_TRACE(*address), U_STRING_TO_TRACE(user_rec->nodog));
   
               askNodogToLogoutUser();
               }
   
            user_rec->setRecord();
   
            // NB: waTime, waTraffic sono valori di *** CONSUMO ***
   
            if (*policy == "FLAT")
               {
               user_rec->time_available    = 0;
               user_rec->traffic_available = 0;
               }
            else
               {
               user_rec->time_available    = (*table)["waTime"].strtol();     // waTime: 7200
               user_rec->traffic_available = (*table)["waTraffic"].strtoll(); // waTraffic: 314572800
               }
   
            user_rec->store(user_exist ? RDB_REPLACE : RDB_INSERT);
   
            UString signed_data = signData("uid=%.*s", U_STRING_TO_TRACE(*uid));
   
            // NB: in questo modo l'utente ripassa dal firewall e NoDog lo rimanda da noi (login_validate) con i dati rinnovati...
   
            setAlternativeRedirect("http://www.google.com/login_validate?%.*s", U_STRING_TO_TRACE(signed_data));
            }
         break;
   
         case POST_logout:
            {
            // ----------------------------
            // $1 -> uid
            // ----------------------------
   
            UHTTP::processHTTPForm();
   
            UHTTP::getFormValue(*uid, 1);
   
            U_ASSERT_EQUALS(*uid, UHTTP::getFormValue(U_CONSTANT_TO_PARAM("uid"), 0, 2))
   
            if (check_if_user_is_connected() == false) setMessagePage("Utente non connesso", "Utente non connesso");
            else
               {
               askNodogToLogoutUser();
   
               U_http_is_connection_close = U_YES;
   
               setAlternativeInclude(0,
                                     "<script type=\"text/javascript\" src=\"js/logout_popup.js\"></script>",
                                     "'onload=\"CloseItAfterSomeTime()\"'",
                                     U_CONSTANT_TO_PARAM("logout_notify.tmpl"), uid->c_str());
               }
            }
         break;
   
         case POST_registrazione:
            {
            /* we can manage this with the main bash script...
            ---------------------------------------------------------------------------------------------------------------
            // $1  -> nome
            // $2  -> cognome
            // $3  -> luogo_di_nascita
            // $4  -> data_di_nascita
            // $5  -> email
            // $6  -> cellulare_prefisso
            // $7  -> telefono_cellulare
            // $8  -> password
            // $9  -> password_conferma
            // $10 -> submit
   
            UHTTP::processHTTPForm();
   
            UString nome, cognome, luogo_di_nascita, data_di_nascita, email,
                    cellulare_prefisso, telefono_cellulare, password, password_conferma;
   
            UHTTP::getFormValue(nome,                 1);
            UHTTP::getFormValue(cognome,              3);
            UHTTP::getFormValue(luogo_di_nascita,     5);
            UHTTP::getFormValue(data_di_nascita,      7);
            UHTTP::getFormValue(email,                9);
            UHTTP::getFormValue(cellulare_prefisso,  11);
            UHTTP::getFormValue(telefono_cellulare,  13);
            UHTTP::getFormValue(password,            15);
            UHTTP::getFormValue(password_conferma,   17);
   
            if (password != password_conferma) setMessagePage("Conferma Password errata", "Conferma Password errata");
            else
               {
               ....
   
               setAlternativeInclude("Registrazione effettuata", 0, 0, U_CONSTANT_TO_PARAM("post_registrazione.tmpl"),
                                     caller_id->data(), password.data(), "polling_attivazione", caller_id->data(), password.c_str());
               }
            ---------------------------------------------------------------------------------------------------------------
            */
            }
         break;
   
         case POST_uploader:
            {
            // $1 -> path file uploaded
   
            UHTTP::processHTTPForm();
   
            UString content, source(100U);
   
            UHTTP::getFormValue(source, 1);
   
            content = UFile::contentOf(source);
   
            U_INTERNAL_ASSERT(source.isNullTerminated())
   
            (void) UFile::_unlink(source.data());
   
            if (content.size() > (2 * 1024))
               {
               UString dest(U_CAPACITY), basename = UStringExt::basename(source);
   
               dest.snprintf("%.*s/%.*s", U_STRING_TO_TRACE(*historical_log_dir), U_STRING_TO_TRACE(basename));
   
               (void) UFile::writeTo(dest, content);
               }
   
            body->clear();
   
            setAlternativeResponse();
            }
         break;
   
         default:
            u_http_info.nResponseCode = HTTP_BAD_REQUEST;
         break;
         }
      }
   else
      {
      u_http_info.nResponseCode = HTTP_BAD_METHOD;
      }
   
   U_INTERNAL_DUMP("u_http_info.nResponseCode = %d", u_http_info.nResponseCode)
   
   if (u_http_info.nResponseCode == 0) (void) UClientImage_Base::environment->append(U_CONSTANT_TO_PARAM("HTTP_RESPONSE_CODE=0\n"));
   
   return 0;
   
   U_RETURN(usp_as_service ? 0 : 200);
} }