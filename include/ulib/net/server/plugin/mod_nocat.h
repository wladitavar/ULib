// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    mod_nocat.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_NOCAT_H
#define U_MOD_NOCAT_H 1

#include <ulib/url.h>
#include <ulib/timer.h>
#include <ulib/command.h>
#include <ulib/net/ping.h>
#include <ulib/container/vector.h>
#include <ulib/container/hash_map.h>
#include <ulib/net/server/server_plugin.h>

class UIptAccount;
class UNoCatPlugIn;

class UModNoCatPeer : public UIPAddress, UEventTime {
public:

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   enum Status { PEER_DENY, PEER_PERMIT };

   // COSTRUTTORI

            UModNoCatPeer();
   virtual ~UModNoCatPeer()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UModNoCatPeer)
      }

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime();

   // DEBUG

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   UCommand fw;
   UString ip, mac, token, user, ifname, label, gateway;
   uint64_t traffic_done, traffic_available, traffic_remain;
   time_t connected, expire, logout, ctime, time_no_traffic, time_remain;
   uint32_t ctraffic, index_AUTH, index_device, index_network;
   int status;
   bool allowed;

   bool checkPeerInfo(bool btraffic);

private:
   UModNoCatPeer(const UModNoCatPeer&) : UIPAddress(), UEventTime() {}
   UModNoCatPeer& operator=(const UModNoCatPeer&)                   { return *this; }

   friend class UNoCatPlugIn;
};

// NB: sizeof(UModNoCatPeer) 32bit == 240 (=> U_STACK_TYPE_5)

// override the default...
template <> inline void u_destroy(UIPAddress** ptr, uint32_t n) { U_TRACE(0,"u_destroy<UIPAddress*>(%p,%u)", ptr, n) }

class U_EXPORT UNoCatPlugIn : public UServerPlugIn, UEventTime {
public:

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   static const UString* str_ROUTE_ONLY;
   static const UString* str_DNS_ADDR;
   static const UString* str_INCLUDE_PORTS;
   static const UString* str_EXCLUDE_PORTS;
   static const UString* str_ALLOWED_WEB_HOSTS;
   static const UString* str_EXTERNAL_DEVICE;
   static const UString* str_INTERNAL_DEVICE;
   static const UString* str_LOCAL_NETWORK;
   static const UString* str_LOCAL_NETWORK_LABEL;
   static const UString* str_AUTH_SERVICE_URL;
   static const UString* str_LOGIN_TIMEOUT;
   static const UString* str_FW_CMD;
   static const UString* str_DECRYPT_CMD;
   static const UString* str_DECRYPT_KEY;
   static const UString* str_CHECK_TYPE;
   static const UString* str_Action;
   static const UString* str_Permit;
   static const UString* str_Deny;
   static const UString* str_Mode;
   static const UString* str_Redirect;
   static const UString* str_renew;
   static const UString* str_Mac;
   static const UString* str_Timeout;
   static const UString* str_Token;
   static const UString* str_User;
   static const UString* str_allowed;
   static const UString* str_anonymous;
   static const UString* str_Traffic;
   static const UString* str_GATEWAY_PORT;
   static const UString* str_FW_ENV;
   static const UString* str_IPHONE_SUCCESS;
   static const UString* str_CHECK_EXPIRE_INTERVAL;
   static const UString* str_ALLOWED_MEMBERS;
   static const UString* str_without_mac;
   static const UString* str_without_label;
   static const UString* str_allowed_members_default;

   static const UString* str_UserDownloadRate;
   static const UString* str_UserUploadRate;

   static void str_allocate();

   enum CheckType {
      U_CHECK_NONE      = 0x000,
      U_CHECK_ARP_CACHE = 0x001,
      U_CHECK_ARP_PING  = 0x002,
      U_CHECK_TRAFFIC   = 0x004
   };

   // COSTRUTTORI

            UNoCatPlugIn();
   virtual ~UNoCatPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg);
   virtual int handlerInit();
   virtual int handlerFork();

   // Connection-wide hooks

   virtual int handlerRequest();
   virtual int handlerReset();

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime();

   // DEBUG

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   static UString* ip;
   static UString* mode;
   static UString* host;
   static UString* input;
   static UString* label;
   static UString* fw_cmd;
   static UString* extdev;
   static UString* intdev;
   static UString* hostname;
   static UString* localnet;
   static UString* location;
   static UString* auth_login;
   static UString* decrypt_key;
   static UString* allowed_members;
   static UCommand* fw;
   static UCommand* uclient;
   static UCommand* uploader;
   static UVector<Url*>* vauth_url;
   static UVector<Url*>* vinfo_url;
   static UVector<UString>* vauth;
   static UVector<UString>* vauth_ip;
   static UVector<UString>* vLoginValidate;
   static UVector<UString>* vInternalDevice;
   static UVector<UString>* vLocalNetwork;
   static UVector<UString>* vLocalNetworkLabel;
   static UVector<UString>* vLocalNetworkGateway;
   static UVector<UIPAllow*>* vLocalNetworkMask;

   static vPF unatexit;
   static UPing** sockp;
   static UString* fw_env;
   static fd_set addrmask;
   static fd_set* paddrmask;
   static UIptAccount* ipt;
   static bool flag_check_system;
   static UString* status_content;
   static int fd_stderr, check_type;
   static UVector<UString>* openlist;
   static UVector<UIPAddress*>** vaddr;
   static const UString* label_to_match;
   static UHashMap<UModNoCatPeer*>* peers;
   static uint32_t total_connections, login_timeout, nfds, num_radio;
   static time_t last_request_firewall, last_request_check, check_expire, next_event_time;

   // VARIE

   static uint32_t getIndexAUTH(const char* ip_address) __pure;

   static void           checkOldPeer(UModNoCatPeer* peer);
   static UModNoCatPeer* creatNewPeer(uint32_t index_AUTH);
   static void             setNewPeer(UModNoCatPeer* peer, uint32_t index_AUTH);

   static void setPeerListInfo();
   static void checkSystem(bool blog);
   static void addPeerInfo(UModNoCatPeer* peer, time_t logout);
   static bool checkAuthMessage(UModNoCatPeer* peer, const UString& msg);
   static void setStatusContent(UModNoCatPeer* peer, const UString& label);
   static void setRedirectLocation(UModNoCatPeer* peer, const UString& redirect, const Url& auth);
   static bool sendMsgToPortal(UCommand& cmd, uint32_t index_AUTH, const UString& msg, UString* poutput);

   static void sendMsgToPortal(UCommand& cmd, const UString& msg)
      {
      U_TRACE(0, "UNoCatPlugIn::sendMsgToPortal(%p,%.*S)", &cmd, U_STRING_TO_TRACE(msg))

      for (uint32_t i = 0, n = vinfo_url->size(); i < n; ++i) (void) sendMsgToPortal(cmd, i, msg, 0);
      }

   static UModNoCatPeer* getPeer(uint32_t i) __pure;
          UModNoCatPeer* getPeerFromMAC(const UString& mac);

   static UString getIPAddress(const char* ptr, uint32_t len);
   static UString getSignedData(const char* ptr, uint32_t len);

   static void   deny(UModNoCatPeer* peer, bool disconnected);
   static void permit(UModNoCatPeer* peer, uint32_t UserDownloadRate, uint32_t UserUploadRate);

   static void getTraffic();
   static void setHTTPResponse(const UString& content);
   static void notifyAuthOfUsersInfo(uint32_t index_AUTH);
   static void getPeerStatus(UStringRep* key, void* value);
   static void checkPeerInfo(UStringRep* key, void* value);
   static void checkPeerStatus(UStringRep* key, void* value);
   static void getPeerListInfo(UStringRep* key, void* value);
   static void executeCommand(UModNoCatPeer* peer, int type);
   static void setFireWallCommand(UCommand& cmd, const UString& script, const UString& mac, const char* ip);

   static bool isPingAsyncPending()
      {
      U_TRACE(0, "UNoCatPlugIn::isPingAsyncPending()")

      U_INTERNAL_DUMP("check_type = %B nfds = %u paddrmask = %p", check_type, nfds, paddrmask)

      bool result = (((check_type & U_CHECK_ARP_PING) != 0) && nfds && paddrmask == 0);

      U_RETURN(result);
      }

private:
   UNoCatPlugIn(const UNoCatPlugIn&) : UServerPlugIn(), UEventTime() {}
   UNoCatPlugIn& operator=(const UNoCatPlugIn&)                      { return *this; }

   friend class UModNoCatPeer;
};

#endif
