#include "NetworkConfig.h"
#include "Globals.h"
#include <Esp.h>

NetworkConfig::NetworkConfig() { 
  sprintf(hostName, "%s_%012llX", APP_NAME, ESP.getEfuseMac());
  strcpy(ST_SSID, DEFAULT_SSD);
  strcpy(ST_Pass, DEFAULT_SSD_PASS);
}

String NetworkConfig::toString() {
  static char buffer[256] = {0};
  if (_description.length() == 0) {
    sprintf(buffer, "%s", 
      hostName);
    _description = buffer;
  }
  return _description;
}