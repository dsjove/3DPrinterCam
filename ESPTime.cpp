#include "ESPTime.h"

#include "ping/ping_sock.h"
#include "esp32-hal.h"
#include <WiFi.h>
#include "WifiConnection.h"
#include "NetworkConfig.h"
#include "ESPTime.h"
#include "Hardware.h"

#include <HardwareSerial.h>

static const char timezone[64] = "GMT0";
static const char ntpServer[64] = "pool.ntp.org";

bool timeSynchronized = false;

bool ESPTime::getLocalNTP() {
  if (timeSynchronized) return true;

  configTzTime(timezone, ntpServer);
  if (getEpoch() > 10000) {
    showLocalTime("NTP");    
    return true;
  }
  log_i("Not yet synced with NTP");
  return false;
}

void ESPTime::syncToBrowser(uint32_t browserUTC) {
  // Synchronize to browser clock if out of sync
  struct timeval tv;
  tv.tv_sec = browserUTC;
  settimeofday(&tv, NULL);
  setenv("TZ", timezone, 1);
  tzset();
  showLocalTime("browser");
}

void ESPTime::showLocalTime(const char* timeSrc) {
  time_t currEpoch = getEpoch();
  char timeFormat[20];
  strftime(timeFormat, sizeof(timeFormat), "%d/%m/%Y %H:%M:%S", localtime(&currEpoch));
  log_i("Got current time from %s: %s with tz: %s", timeSrc, timeFormat, timezone);
  timeSynchronized = true;
}

time_t ESPTime::getEpoch() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
}

void ESPTime::dateFormat(char* inBuff, size_t inBuffLen, bool isFolder) {
  time_t currEpoch = getEpoch();
  if (isFolder) 
    strftime(inBuff, inBuffLen, "/%Y%m%d", localtime(&currEpoch));
  else 
    strftime(inBuff, inBuffLen, "/%Y%m%d/%Y%m%d_%H%M%S", localtime(&currEpoch));
}