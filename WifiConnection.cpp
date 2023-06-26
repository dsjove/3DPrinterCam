#include "WifiConnection.h"
#include "NetworkConfig.h"
#include "Globals.h"

//TODO: remove logging and add to stats and delegate
#include <HardwareSerial.h>
#define LOG_INF Serial.printf
#define LOG_ERR LOG_INF
#define LOG_WRN LOG_INF

WifiConnection* WifiConnection::singleton = NULL;

WifiConnection::WifiConnection(const NetworkConfig& config, IWifiDelegate& delegate)
  : _config(config)
  , _delegate(delegate) {
  singleton = this;
}

void WifiConnection::setup() {
    // start wifi station (and wifi AP if allowed or station not defined)
    WiFi.mode(WIFI_AP_STA);
    WiFi.persistent(false); // prevent the flash storage WiFi credentials
    WiFi.setAutoReconnect(false); // Set whether module will attempt to reconnect to an access point in case it is disconnected
    WiFi.softAPdisconnect(true); // kill rogue AP on startup
    WiFi.setHostname(_config.hostName);
    delay(100);
    WiFi.onEvent(staticWiFiEvent);
    startWifi();
}

void WifiConnection::staticWiFiEvent(WiFiEvent_t event) {
  singleton->onWiFiEvent(event);
}

static const char* wifiStatusStr(wl_status_t wlStat) {
  switch (wlStat) {
    case WL_NO_SHIELD: return "wifi not initialised";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "not available, use AP";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "unable to connect";
    default: break;
  }
  return "Invalid WiFi.status";
}

static const char* getEncType(int ssidIndex) {
  switch (WiFi.encryptionType(ssidIndex)) {
    case (WIFI_AUTH_OPEN): return "Open";
    case (WIFI_AUTH_WEP): return "WEP";
    case (WIFI_AUTH_WPA_PSK): return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK): return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK): return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE): return "WPA2_ENTERPRISE";
    case (WIFI_AUTH_MAX): return "AUTH_MAX";
    default: break;
  }
  return "Unknown";
}

void WifiConnection::startWifi() {
  setWifiSTA();
  // connect to Wifi station
  uint32_t startAttemptTime = millis();
  // Stop trying on failure timeout, will try to reconnect later by ping
  wl_status_t wlStat;
  if (!strlen(_config.ST_SSID)) {
    wlStat = WL_NO_SSID_AVAIL;
  }
  else {
    while (
        wlStat = WiFi.status(), 
        wlStat != WL_CONNECTED && millis() - startAttemptTime < wifiInitialTimeout) {
      Serial.print(".");
      delay(wifiInitialDelay);
      Serial.flush();
    }
  }
  if (wlStat == WL_CONNECTED) {
    // show stats of requested SSID
    int numNetworks = WiFi.scanNetworks();
    for (int i=0; i < numNetworks; i++) {
      if (!strcmp(WiFi.SSID(i).c_str(), _config.ST_SSID)) {
        LOG_INF("\nWifi stats for %s - signal strength: %d dBm; Encryption: %s; channel: %u\n",  WiFi.SSID(i), WiFi.RSSI(i), getEncType(i), WiFi.channel(i));
        break;
      }
    }
  }
  else {
    LOG_WRN("SSID '%s' %s", _config.ST_SSID, wifiStatusStr(wlStat));
    if (_config.allowAP) {
      setWifiAP(); // AP allowed if no Station SSID eg on first time use 
    }
  }
  setupMdnsHost();
  _status.wifiStarted = wlStat == WL_CONNECTED ? true : false;
  startPing();
}

void WifiConnection::setupMdnsHost() { 
   // not on ESP32 as uses 6k of heap 
#if CONFIG_IDF_TARGET_ESP32S3
 // set up MDNS service 
  char mdnsName[15]; // max mdns host name length
  snprintf(mdnsName, 15, _config.hostName);
  if (MDNS.begin(mdnsName)) {
    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "udp", 83);
    // MDNS.addService("ftp", "tcp", 21);    
    LOG_INF("mDNS service: http://%s.local", mdnsName);
  }
  else {
    LOG_ERR("mDNS host: %s Failed", mdnsName);
  }
#endif
}

void WifiConnection::setWifiSTA() {
  // set station with static ip if provided
  if (strlen(_config.ST_ip) > 1) {
    IPAddress _ip, _gw, _sn, _ns1, _ns2;
    if (!_ip.fromString(_config.ST_ip)) {
      LOG_ERR("Failed to parse IP: %s", _config.ST_ip);
    }
    else {
      _ip.fromString(_config.ST_ip);
      _gw.fromString(_config.ST_gw);
      _sn.fromString(_config.ST_sn);
      _ns1.fromString(_config.ST_ns1);
      _ns2.fromString(_config.ST_ns2);
      // set static ip
      WiFi.config(_ip, _gw, _sn, _ns1); // need DNS for SNTP
      LOG_INF("Wifi Station set static IP");
    } 
  } 
  else {
    LOG_INF("Wifi Station IP from DHCP");
  }
  WiFi.begin(_config.ST_SSID, _config.ST_Pass);
}

void WifiConnection::setWifiAP() {
  if (!_status.apStarted) {
    // Set access point with static ip if provided
    if (strlen(_config.AP_ip) > 1) {
      //LOG_INF("Set AP static IP :%s, %s, %s", _config.AP_ip, _config.AP_gw, _config.AP_sn);  
      IPAddress _ip, _gw, _sn, _ns1 ,_ns2;
      _ip.fromString(_config.AP_ip);
      _gw.fromString(_config.AP_gw);
      _sn.fromString(_config.AP_sn);
      // set static ip
      WiFi.softAPConfig(_ip, _gw, _sn);
    } 
    //TODO: this is failing
    _status.apStarted = WiFi.softAP(_config.AP_SSID, _config.AP_Pass);
  }
}

void WifiConnection::onWiFiEvent(WiFiEvent_t event) {
  // callback to report on wifi events
  if (event == ARDUINO_EVENT_WIFI_READY);
  else if (event == ARDUINO_EVENT_WIFI_SCAN_DONE);  
  else if (event == ARDUINO_EVENT_WIFI_STA_START) LOG_INF("Wifi Station started, connecting to: %s", _config.ST_SSID);
  else if (event == ARDUINO_EVENT_WIFI_STA_STOP) LOG_INF("Wifi Station stopped %s", _config.ST_SSID);
  else if (event == ARDUINO_EVENT_WIFI_AP_START) {
    if (!strcmp(WiFi.softAPSSID().c_str(), _config.AP_SSID) || !strlen(_config.AP_SSID)) {
      LOG_INF("Wifi AP SSID: %s started, use 'http://%s' to connect", _config.AP_SSID, WiFi.softAPIP().toString().c_str());
      _status.apStarted = true;
    }
  }
  else if (event == ARDUINO_EVENT_WIFI_AP_STOP) {
    if (!strcmp(WiFi.softAPSSID().c_str(), _config.AP_SSID)) {
      LOG_INF("Wifi AP stopped: %s", _config.AP_SSID);
      _status.apStarted = false;
    }
  }
  else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) LOG_INF("Wifi Station IP, use 'http://%s' to connect", WiFi.localIP().toString().c_str()); 
  else if (event == ARDUINO_EVENT_WIFI_STA_LOST_IP) LOG_INF("Wifi Station lost IP");
  else if (event == ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
  else if (event == ARDUINO_EVENT_WIFI_STA_CONNECTED) LOG_INF("WiFi Station connection to %s, using hostname: %s", _config.ST_SSID, _config.hostName);
  else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) LOG_INF("WiFi Station disconnected");
  else if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) LOG_INF("WiFi AP client connection");
  else if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) LOG_INF("WiFi AP client disconnection");
  else LOG_WRN("WiFi Unhandled event %d", event);
}

void WifiConnection::startPing() {
  if (pingHandle != NULL) return;

  IPAddress ipAddr = WiFi.gatewayIP();
  ip_addr_t pingDest; 
  IP_ADDR4(&pingDest, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  esp_ping_config_t pingConfig = ESP_PING_DEFAULT_CONFIG();
  pingConfig.target_addr = pingDest;  
  pingConfig.count = ESP_PING_COUNT_INFINITE;
  pingConfig.interval_ms = wifiTimeoutSecs * 1000;
  pingConfig.timeout_ms = 5000;
#if CONFIG_IDF_TARGET_ESP32S3
  pingConfig.task_stack_size = 1024 * 6;
#else
  pingConfig.task_stack_size = 1024 * 4;
#endif
  pingConfig.task_prio = 1;
  // set ping task callback functions 
  esp_ping_callbacks_t cbs;
  cbs.on_ping_success = staticPingSuccess;
  cbs.on_ping_timeout = staticPingTimeout;
  cbs.on_ping_end = NULL; 
  cbs.cb_args = NULL;
  esp_ping_new_session(&pingConfig, &cbs, &pingHandle);
  esp_ping_start(pingHandle);
  LOG_INF("Started ping monitoring");
}

void WifiConnection::staticPingSuccess(esp_ping_handle_t hdl, void *args) {
  singleton->pingSuccess(hdl, args);
}

void WifiConnection::pingSuccess(esp_ping_handle_t hdl, void *args) {
  _delegate.ping(true);
}

void WifiConnection::staticPingTimeout(esp_ping_handle_t hdl, void *args) {
  singleton->pingTimeout(hdl, args);
}

void WifiConnection::pingTimeout(esp_ping_handle_t hdl, void *args) {
  if (strlen(_config.ST_SSID)) {
    wl_status_t wStat = WiFi.status();
    if (wStat != WL_NO_SSID_AVAIL && wStat != WL_NO_SHIELD) {
      LOG_WRN("Failed to ping gateway, restart wifi ...");
      startWifi();
    }
  }
  _delegate.ping(false);
}

void WifiConnection::stopPing() {
  if (pingHandle != NULL) {
    esp_ping_stop(pingHandle);
    esp_ping_delete_session(pingHandle);
    pingHandle = NULL;
  }
}

