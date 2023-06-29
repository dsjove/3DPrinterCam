#include "Storage.h"
#include "AppHardware.h"
#include "Hardware.h"
#include <SD_MMC.h>

#define ONEMEG (1024 * 1024)

Storage::Storage() {
}

StorageStatus Storage::status() const {
  StorageStatus status;
  status.freeHeap = ESP.getFreeHeap();
  status.freePSRAM = ESP.getFreePsram();
  status.largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
#ifdef INCLUDE_SD
    status.freeStorage = SD_MMC.usedBytes() / ONEMEG;
#endif
  return status;
}

void Storage::setup(AppHardware& hardware) {
  bool formatIfMountFailed = true;
#ifdef INCLUDE_SD  
  if ((fs::SDMMCFS*)&STORAGE == &SD_MMC) {
    /* open SD card in MMC 1 bit mode
     MMC4  MMC1  ESP32 ESP32S3
      D2          12
      D3    ..    13
      CMD  CMD    15    38
      CLK  CLK    14    39
      D0   D0     2     40
      D1          4
    */
    if (psramFound()) heap_caps_malloc_extmem_enable(5); // small number to force vector into psram
    //fileVec.reserve(1000);
    if (psramFound()) heap_caps_malloc_extmem_enable(4096);
#if CONFIG_IDF_TARGET_ESP32S3
#if !defined(SD_MMC_CLK)
    return;
#endif
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
#endif
    bool res = SD_MMC.begin("/sdcard", true, formatIfMountFailed);
    if (res == false) {
      return;
    }
    uint8_t cardType = SD_MMC.cardType();
    switch (SD_MMC.cardType()) {
      case CARD_NONE:
        return;
      case CARD_MMC:
        hardware.storageType = "MMC";
        break;
      case CARD_SD:
        hardware.storageType = "SDSC";
        break;
      case CARD_SDHC:
        hardware.storageType = "SDHC";
        break;
    }
    hardware.physicalSize = SD_MMC.cardSize() / ONEMEG;
    hardware.totalBytes = SD_MMC.totalBytes() / ONEMEG;
#if defined(CAMERA_MODEL_AI_THINKER)
    pinMode(4, OUTPUT);
    digitalWrite(4, 0); // set lamp pin fully off as sd_mmc library still initialises pin 4 in 1 line mode
#endif
    return;
#endif
  }
#ifdef _SPIFFS_H_
  if ((fs::SPIFFSFS*)&STORAGE == &SPIFFS) {
    hardware.storageType = "SPIFFS";
    res = SPIFFS.begin(formatIfMountFailed);
    return;
  }
#endif
#ifdef _LITTLEFS_H_
  if ((fs::LittleFSFS*)&STORAGE == &LittleFS) {
    hardware.storageType = "LittleFS";
    res = LittleFS.begin(formatIfMountFailed);
    return;
  }
#endif
}