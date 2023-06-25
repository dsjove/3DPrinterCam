#pragma once
#include <WiFi.h>
#include "ping/ping_sock.h"

class NetworkConfig;

/**
  Setup WIFI and device time
**/

struct WifiStatus {
  String ssid;
  bool wifiStarted = false;
  bool apStarted = false;
};

class IWifiDelegate {
  public:
    virtual void ping(bool success) = 0;
};

class WifiConnection {
  public:
    WifiConnection(const NetworkConfig& config, IWifiDelegate& delegate);
    void setup();
    WifiStatus status() const { return _status; }

  private:
    const NetworkConfig& _config;
    IWifiDelegate& _delegate;

    const int wifiTimeoutSecs = 30; // how often to check wifi status
    const int wifiInitialTimeout = 5000;
    const int wifiInitialDelay = 500;

    static WifiConnection* singleton;
    esp_ping_handle_t pingHandle = NULL;
    
    WifiStatus _status;

    void startWifi();
    void setWifiSTA();
    void setupMdnsHost();
    void setWifiAP();

    static void staticWiFiEvent(WiFiEvent_t event);
    void onWiFiEvent(WiFiEvent_t event);

    void startPing();
    static void staticPingSuccess(esp_ping_handle_t hdl, void *args);
    void pingSuccess(esp_ping_handle_t hdl, void *args);
    static void staticPingTimeout(esp_ping_handle_t hdl, void *args);
    void pingTimeout(esp_ping_handle_t hdl, void *args);
    void stopPing();
};
