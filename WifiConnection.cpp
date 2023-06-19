#include "WifiConnection.h"
#include "Globals.h"

WifiConnection* WifiConnection::singleton = NULL;

WifiConnection::WifiConnection()
{
  singleton = this;
}

int WifiConnection::setup()
{
  return startWifi(true) ? 0 : 1;
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
  }
  return "Invalid WiFi.status";
}

static const char* getEncType(int ssidIndex) {
  switch (WiFi.encryptionType(ssidIndex)) {
    case (WIFI_AUTH_OPEN):
      return "Open";
    case (WIFI_AUTH_WEP):
      return "WEP";
    case (WIFI_AUTH_WPA_PSK):
      return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK):
      return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK):
      return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE):
      return "WPA2_ENTERPRISE";
    case (WIFI_AUTH_MAX):
      return "AUTH_MAX";
    default:
      return "Not listed";
  }
  return "n/a";
}

bool WifiConnection::startWifi(bool firstcall) {
  // start wifi station (and wifi AP if allowed or station not defined)
  if (firstcall) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.persistent(false); // prevent the flash storage WiFi credentials
    WiFi.setAutoReconnect(false); // Set whether module will attempt to reconnect to an access point in case it is disconnected
    WiFi.softAPdisconnect(true); // kill rogue AP on startup
    WiFi.setHostname(hostName);
    delay(100);
    WiFi.onEvent(staticWiFiEvent);
  }
  setWifiSTA();
  // connect to Wifi station
  uint32_t startAttemptTime = millis();
  // Stop trying on failure timeout, will try to reconnect later by ping
  wl_status_t wlStat;
  if (!strlen(ST_SSID)) wlStat = WL_NO_SSID_AVAIL;
  else {
    while (wlStat = WiFi.status(), wlStat != WL_CONNECTED && millis() - startAttemptTime < 5000)  {
      Serial.print(".");
      delay(500);
      Serial.flush();
    }
  }
  if (wlStat == WL_NO_SSID_AVAIL || allowAP) setWifiAP(); // AP allowed if no Station SSID eg on first time use 
  if (wlStat != WL_CONNECTED) ;//LOG_WRN("SSID %s %s", ST_SSID, wifiStatusStr(wlStat));
#if CONFIG_IDF_TARGET_ESP32S3
  setupMdnsHost(); // not on ESP32 as uses 6k of heap
#endif
  // show stats of requested SSID
  int numNetworks = WiFi.scanNetworks();
  for (int i=0; i < numNetworks; i++) {
    if (!strcmp(WiFi.SSID(i).c_str(), ST_SSID))
      ;//LOG_INF("Wifi stats for %s - signal strength: %d dBm; Encryption: %s; channel: %u",  ST_SSID, WiFi.RSSI(i), getEncType(i), WiFi.channel(i));
  }
  if (pingHandle == NULL) startPing();
  return wlStat == WL_CONNECTED ? true : false;
}

void WifiConnection::setWifiSTA()
{
  // set station with static ip if provided
  if (strlen(ST_ip) > 1) {
    IPAddress _ip, _gw, _sn, _ns1, _ns2;
    if (!_ip.fromString(ST_ip)) ;//LOG_ERR("Failed to parse IP: %s", ST_ip);
    else {
      _ip.fromString(ST_ip);
      _gw.fromString(ST_gw);
      _sn.fromString(ST_sn);
      _ns1.fromString(ST_ns1);
      _ns2.fromString(ST_ns2);
      // set static ip
      WiFi.config(_ip, _gw, _sn, _ns1); // need DNS for SNTP
      //LOG_INF("Wifi Station set static IP");
    } 
  } else ;//LOG_INF("Wifi Station IP from DHCP");
  WiFi.begin(ST_SSID, ST_Pass);
  //debugMemory("setWifiSTA");
}

void WifiConnection::setWifiAP() {
  if (!APstarted) {
    // Set access point with static ip if provided
    if (strlen(AP_ip) > 1) {
      //LOG_INF("Set AP static IP :%s, %s, %s", AP_ip, AP_gw, AP_sn);  
      IPAddress _ip, _gw, _sn, _ns1 ,_ns2;
      _ip.fromString(AP_ip);
      _gw.fromString(AP_gw);
      _sn.fromString(AP_sn);
      // set static ip
      WiFi.softAPConfig(_ip, _gw, _sn);
    } 
    WiFi.softAP(AP_SSID, AP_Pass);
    //debugMemory("setWifiAP");
  }
}

void WifiConnection::staticWiFiEvent(WiFiEvent_t event) {
  singleton->onWiFiEvent(event);
}

void WifiConnection::onWiFiEvent(WiFiEvent_t event)
{
  // callback to report on wifi events
  if (event == ARDUINO_EVENT_WIFI_READY);
  else if (event == ARDUINO_EVENT_WIFI_SCAN_DONE);  
  else if (event == ARDUINO_EVENT_WIFI_STA_START) ;//LOG_INF("Wifi Station started, connecting to: %s", ST_SSID);
  else if (event == ARDUINO_EVENT_WIFI_STA_STOP) ;//LOG_INF("Wifi Station stopped %s", ST_SSID);
  else if (event == ARDUINO_EVENT_WIFI_AP_START) {
    if (!strcmp(WiFi.softAPSSID().c_str(), AP_SSID) || !strlen(AP_SSID)) {
      //LOG_INF("Wifi AP SSID: %s started, use 'http://%s' to connect", WiFi.softAPSSID().c_str(), WiFi.softAPIP().toString().c_str());
      APstarted = true;
    }
  }
  else if (event == ARDUINO_EVENT_WIFI_AP_STOP) {
    if (!strcmp(WiFi.softAPSSID().c_str(), AP_SSID)) {
      //LOG_INF("Wifi AP stopped: %s", AP_SSID);
      APstarted = false;
    }
  }
  else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) ;//LOG_INF("Wifi Station IP, use 'http://%s' to connect", WiFi.localIP().toString().c_str()); 
  else if (event == ARDUINO_EVENT_WIFI_STA_LOST_IP) ;//LOG_INF("Wifi Station lost IP");
  else if (event == ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
  else if (event == ARDUINO_EVENT_WIFI_STA_CONNECTED) ;//LOG_INF("WiFi Station connection to %s, using hostname: %s", ST_SSID, hostName);
  else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) ;//LOG_INF("WiFi Station disconnected");
  else if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) ;//LOG_INF("WiFi AP client connection");
  else if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) ;//LOG_INF("WiFi AP client disconnection");
  else ;//LOG_WRN("WiFi Unhandled event %d", event);
}

void WifiConnection::startPing() {
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
  //LOG_INF("Started ping monitoring");
  //debugMemory("startPing");
}

void WifiConnection::staticPingSuccess(esp_ping_handle_t hdl, void *args) {
  singleton->pingSuccess(hdl, args);
}

void WifiConnection::pingSuccess(esp_ping_handle_t hdl, void *args) {
  if (!timeSynchronized) getLocalNTP();
  if (!dataFilesChecked) dataFilesChecked = checkDataFiles();
#ifdef INCLUDE_MQTT
  if (mqtt_active) startMqttClient();
#endif
  doAppPing();
}

void WifiConnection::staticPingTimeout(esp_ping_handle_t hdl, void *args) {
  singleton->pingTimeout(hdl, args);
}

void WifiConnection::pingTimeout(esp_ping_handle_t hdl, void *args) {
  if (strlen(ST_SSID)) {
    wl_status_t wStat = WiFi.status();
    if (wStat != WL_NO_SSID_AVAIL && wStat != WL_NO_SHIELD) {
      //LOG_WRN("Failed to ping gateway, restart wifi ...");
      startWifi(false);
    }
  }
  doAppPing();
}

void WifiConnection::showLocalTime(const char* timeSrc) {
  time_t currEpoch = getEpoch();
  char timeFormat[20];
  strftime(timeFormat, sizeof(timeFormat), "%d/%m/%Y %H:%M:%S", localtime(&currEpoch));
  //LOG_INF("Got current time from %s: %s with tz: %s", timeSrc, timeFormat, timezone);
  timeSynchronized = true;
}

bool WifiConnection::getLocalNTP() {
  // get current time from NTP server and apply to ESP32
  //LOG_INF("Using NTP server: %s", ntpServer);
  configTzTime(timezone, ntpServer);
  if (getEpoch() > 10000) {
    showLocalTime("NTP");    
    return true;
  }
  else {
    //LOG_WRN("Not yet synced with NTP");
    return false;
  }
}

bool WifiConnection::checkDataFiles() {
  /*
  // Download any missing data files
  if (!fp.exists(DATA_DIR)) fp.mkdir(DATA_DIR);
  bool res = false;
  if (strlen(GITHUB_URL)) {
    res = wgetFile(GITHUB_URL, CONFIG_FILE_PATH, true);
    if (res) res = wgetFile(GITHUB_URL, INDEX_PAGE_PATH);      
    if (res) res = appDataFiles();
  }
  return res;
  */
  return true;
}

void WifiConnection::doAppPing() {
  /*
  doIOExtPing();
  // check for night time actions
  if (isNight(nightSwitch)) {
    if (wakeUse && wakePin) {
     // to use LDR on wake pin, connect it between pin and 3V3
     // uses internal pulldown resistor as voltage divider
     // but may need to add external pull down between pin
     // and GND to alter required light level for wakeup
     digitalWrite(PWDN_GPIO_NUM, 1); // power down camera
     goToSleep(wakePin, true);
    }
    if (lampNight) setLamp(lampLevel);
  } else if (lampNight) setLamp(0);
  */
}