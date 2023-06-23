#pragma once
#include <WString.h>

class NetworkConfig {
  public:
    NetworkConfig();

    char hostName[32] = ""; // Default Host name
    const char ST_SSID[32]  = ""; //Default router ssid
    const char ST_Pass[64] = ""; //Default router passd

    // leave following blank for dhcp
    const char ST_ip[16]  = ""; // Static IP
    const char ST_sn[16]  = ""; // subnet normally 255.255.255.0
    const char ST_gw[16]  = ""; // gateway to internet, normally router IP
    const char ST_ns1[16] = ""; // DNS Server, can be router IP (needed for SNTP)
    const char ST_ns2[16] = ""; // alternative DNS Server, can be blank

    // Access point Config Portal SSID and Pass
    const bool allowAP = true; // set to true to allow AP to startup if cannot reconnect to STA (router)
    const char AP_SSID[32] = "";
    const char AP_Pass[64] = "";
    const char AP_ip[16]  = ""; //Leave blank to use 192.168.4.1
    const char AP_sn[16]  = "";
    const char AP_gw[16]  = "";

    String toString();

  private:
    String _description;
};