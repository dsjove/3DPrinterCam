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

  String toString() const;
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
    WifiStatus _status;

    void configWifiSTA();
    void startSTA();
    void configWifiAP();
    void startAP();
    bool checkStatus();
};
