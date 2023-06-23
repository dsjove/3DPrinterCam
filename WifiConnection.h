#pragma once
#include <WiFi.h>
#include "ping/ping_sock.h"

class NetworkConfig;

class WifiConnection {
  public:
    WifiConnection(const NetworkConfig& config);
    void setup();

  private:
    const NetworkConfig& _config;

    const char timezone[64] = "GMT0";
    const char ntpServer[64] = "pool.ntp.org";
    const int wifiTimeoutSecs = 30; // how often to check wifi status
    
    bool wifiStarted = false;
    bool APstarted = false;
    esp_ping_handle_t pingHandle = NULL;
    bool timeSynchronized = false;
    bool dataFilesChecked = false;

    void startWifi();
    void setWifiSTA();
    void setupMdnsHost();
    void setWifiAP();

    static WifiConnection* singleton;
    static void staticWiFiEvent(WiFiEvent_t event);
    void onWiFiEvent(WiFiEvent_t event);

    void startPing();
    static void staticPingSuccess(esp_ping_handle_t hdl, void *args);
    void pingSuccess(esp_ping_handle_t hdl, void *args);
    static void staticPingTimeout(esp_ping_handle_t hdl, void *args);
    void pingTimeout(esp_ping_handle_t hdl, void *args);
    void stopPing();
    void doAppPing();

    bool getLocalNTP();
    bool checkDataFiles();
    void showLocalTime(const char* timeSrc);
    void syncToBrowser(uint32_t browserUTC);
};