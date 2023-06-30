#pragma once
#include "WString.h"

class NetworkConfig;

/**
  Setup WIFI and device time
**/

struct WifiStatus {
  String ssid;
  bool isAP = false;
  String address;
  bool timeSynchronized = false;

  String toString() const;
};

class WifiConnection {
  public:
    WifiConnection(const NetworkConfig& config);
    void setup();
    WifiStatus status() const { return _status; }

  private:
    const NetworkConfig& _config;
    WifiStatus _status;

    void configWifiSTA();
    void startSTA();
    void configWifiAP();
    void startAP();
    bool checkStatus();
    void getLocalNTP();
};
