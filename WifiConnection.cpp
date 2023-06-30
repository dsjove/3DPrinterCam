#include "WifiConnection.h"
#include "NetworkConfig.h"
#include "Hardware.h"
#include <WiFi.h>
#include "ping/ping_sock.h"

WifiConnection::WifiConnection(const NetworkConfig& config, IWifiDelegate& delegate)
  : _config(config)
  , _delegate(delegate) {
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

String WifiStatus::toString() const {
  String result;
  result.reserve(64);
  result.concat('\'');
  result.concat(ssid);
  result.concat('\'');
  if (isAP) {
    result.concat('*');
  }
  result.concat(": ");
  result.concat(address);
  return result;
}

void WifiConnection::setup() {
  WiFi.mode(_config.allowAP ? WIFI_AP_STA : WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setHostname(_config.hostName);
  configWifiSTA();
  configWifiAP();

  startSTA();

  Serial.println(_status.toString());
}

void WifiConnection::configWifiSTA() {
  if (strlen(_config.ST_SSID)) {
    // set station with static ip if provided
    if (strlen(_config.ST_ip) > 1) {
      IPAddress _ip, _gw, _sn, _ns1, _ns2;
      if (!_ip.fromString(_config.ST_ip)) {
        log_i("Failed to parse IP: %s", _config.ST_ip);
      }
      else {
        _ip.fromString(_config.ST_ip);
        _gw.fromString(_config.ST_gw);
        _sn.fromString(_config.ST_sn);
        _ns1.fromString(_config.ST_ns1);
        _ns2.fromString(_config.ST_ns2);
        // set static ip
        WiFi.config(_ip, _gw, _sn, _ns1); // need DNS for SNTP
        log_i("Wifi Station set static IP");
      } 
    } 
    else {
      log_i("Wifi Station IP from DHCP");
    }
  }
  else {
      log_i("Wifi Station No STA");
  }
}

void WifiConnection::configWifiAP() {
  if (_config.allowAP) {
    // Set access point with static ip if provided
    if (strlen(_config.AP_ip) > 1) {
      //log_i("Set AP static IP :%s, %s, %s", _config.AP_ip, _config.AP_gw, _config.AP_sn);  
      IPAddress _ip, _gw, _sn, _ns1 ,_ns2;
      _ip.fromString(_config.AP_ip);
      _gw.fromString(_config.AP_gw);
      _sn.fromString(_config.AP_sn);
      // set static ip
      WiFi.softAPConfig(_ip, _gw, _sn);
      log_i("Wifi AP Station set static IP");
    } 
    else {
      log_i("Wifi AP Station IP from DHCP");
    }
  }
  else {
      log_i("Wifi Station No AP");
  }
}

void WifiConnection::startSTA() {
  if (strlen(_config.ST_SSID)) {
    _status.ssid = _config.ST_SSID;
    _status.isAP  = false;
    Serial.print("WIFI Connecting");
    WiFi.begin(_config.ST_SSID, _config.ST_Pass);
    delay(100);
    if (checkStatus()) {
      _status.address = WiFi.localIP().toString();
    }
    else {
      log_i("Could not start STA WIFI");
    }
  }
  else {
    startAP();
  }
}

bool WifiConnection::checkStatus() {
    wl_status_t wlStat;
    int i = 0;
    while ((wlStat = WiFi.status()) != WL_CONNECTED) {
      if (i > 10) {
        break;
      }
      delay(500);
      Serial.print(".");
      i++;
    }
    Serial.println();
    return wlStat == WL_CONNECTED;
}

void WifiConnection::startAP() {
  if (_config.allowAP) {
    Serial.println("WIFI AP Starting...");
    _status.ssid = _config.AP_SSID;
    _status.isAP  = true;
    bool started = WiFi.softAP(_config.AP_SSID, _config.AP_Pass);
    if (started) {
      _status.address = strlen(_config.AP_ip) ? _config.AP_ip : "192.168.4.1";
    }
    else {
      _status.address = "";
      log_i("Could not start AP WIFI");
    }
  }
  else {
      log_i("WIFI not enabled");
  }
}

