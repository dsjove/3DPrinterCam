#include "NetworkConfig.h"
#include "Globals.h"
#include <Esp.h>

NetworkConfig::NetworkConfig() { 
  sprintf(hostName, "%s_%012llX", APP_NAME, ESP.getEfuseMac());
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