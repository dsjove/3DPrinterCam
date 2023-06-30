#pragma once
#include <WString.h>

/**
  All the required values to setup the network to meet your
  environment.
**/

class NetworkConfig {
  public:
    NetworkConfig();

    const char timezone[64] = "GMT0";
    const char timeserver[64] = "pool.ntp.org";

  //TODO: make configurable from file
  //TODO: allow multiple connection definitions for retries

    char hostName[32] = ""; // Default Host name
    char ST_SSID[32]  = ""; //Default router ssid
    char ST_Pass[64] = ""; //Default router pass

    // leave following blank for dhcp
    const char ST_ip[16]  = ""; // Static IP
    const char ST_sn[16]  = ""; // subnet normally 255.255.255.0
    const char ST_gw[16]  = ""; // gateway to internet, normally router IP
    const char ST_ns1[16] = ""; // DNS Server, can be router IP (needed for SNTP)
    const char ST_ns2[16] = ""; // alternative DNS Server, can be blank

    // Access point Config Portal SSID and Pass
    const bool allowAP = true; // set to true to allow AP to startup if cannot reconnect to STA (router)
    char AP_SSID[32] = "";
    const char AP_Pass[64] = "";
    const char AP_ip[16]  = ""; //Leave blank to use 192.168.4.1
    const char AP_sn[16]  = "";
    const char AP_gw[16]  = "";
};