#include "Globals.h"
#include <Esp.h>

#include "ping/ping_sock.h"

String AppHardware::toString() {
  static char buffer[256] = {0};
  if (_description.length() == 0) {
    sprintf(buffer, "%s:%s %s:%d:%d", 
      cameraBoard.c_str(), cameraModel.c_str(), 
      storageType.c_str(), physicalSize, totalBytes);
    _description = buffer;
  }
  return _description;
}

RuntimeStats::RuntimeStats(const char* caller) {
  //#define CHECK_MEM false // leave as false
  //#define FLUSH_DELAY 0 // for debugging crashes
  //delay(FLUSH_DELAY);
  //logPrint("%s > Free: heap %u, block: %u, pSRAM %u\n", caller, ESP.getFreeHeap(), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), ESP.getFreePsram());
  //LOG_INF("Free: heap %u, block: %u, pSRAM %u", ESP.getFreeHeap(), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), ESP.getFreePsram());
}

String RuntimeStats::toString() {
  return String();
}

time_t getEpoch() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
}

void dateFormat(char* inBuff, size_t inBuffLen, bool isFolder) {
  time_t currEpoch = getEpoch();
  if (isFolder) strftime(inBuff, inBuffLen, "/%Y%m%d", localtime(&currEpoch));
  else strftime(inBuff, inBuffLen, "/%Y%m%d/%Y%m%d_%H%M%S", localtime(&currEpoch));
}
