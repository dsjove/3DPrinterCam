#include "NetworkConfig.h"
#include "Globals.h"
#include <Esp.h>

NetworkConfig::NetworkConfig() { 
  sprintf(hostName, "%s_%012llX", APP_NAME, ESP.getEfuseMac());
  strcpy(ST_SSID, DEFAULT_SSD);
  strcpy(ST_Pass, DEFAULT_SSD_PASS);
}
