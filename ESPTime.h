#pragma once
#include <WString.h>
#include <time.h>

class ESPTime {
  public:
    static void syncToBrowser(uint32_t browserUTC);
    static bool getLocalNTP();
    static void showLocalTime(const char* timeSrc);

    static time_t getEpoch();
    static void dateFormat(char* inBuff, size_t inBuffLen, bool isFolder);
};