#pragma once
#include <WiFi.h>
#include "ping/ping_sock.h"

class WifiConnection {
  public:
    WifiConnection();
    int setup();

  private:
    char hostName[32]; //Host name for ddns
    char ST_SSID[32]; //Router ssid
    char ST_Pass[64]; //Router passd

    // leave following blank for dhcp
    char ST_ip[16]  = ""; // Static IP
    char ST_sn[16]  = ""; // subnet normally 255.255.255.0
    char ST_gw[16]  = ""; // gateway to internet, normally router IP
    char ST_ns1[16] = ""; // DNS Server, can be router IP (needed for SNTP)
    char ST_ns2[16] = ""; // alternative DNS Server, can be blank

    // Access point Config Portal SSID and Pass
    char AP_SSID[32] = "";
    char AP_Pass[64] = "";
    char AP_ip[16]  = ""; //Leave blank to use 192.168.4.1
    char AP_sn[16]  = "";
    char AP_gw[16]  = "";

    const char timezone[64] = "GMT0";
    const char ntpServer[64] = "pool.ntp.org";
    const bool allowAP = true;  // set to true to allow AP to startup if cannot connect to STA (router)
    const int wifiTimeoutSecs = 30; // how often to check wifi status
    
    bool APstarted = false;
    esp_ping_handle_t pingHandle = NULL;
    bool timeSynchronized = false;
    bool dataFilesChecked = false;

    bool startWifi(bool firstcall);
    void setWifiAP();
    void setWifiSTA();
    static WifiConnection* singleton;
    static void staticWiFiEvent(WiFiEvent_t event);
    void onWiFiEvent(WiFiEvent_t event);

    void startPing();
    static void staticPingSuccess(esp_ping_handle_t hdl, void *args);
    void pingSuccess(esp_ping_handle_t hdl, void *args);
    static void staticPingTimeout(esp_ping_handle_t hdl, void *args);
    void pingTimeout(esp_ping_handle_t hdl, void *args);
    bool getLocalNTP();
    bool checkDataFiles();
    void doAppPing();
    void showLocalTime(const char* timeSrc);
};