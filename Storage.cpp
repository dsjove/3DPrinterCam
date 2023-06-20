#include "Storage.h"
#include "Globals.h"
#include <SD_MMC.h>

Storage::Storage() {
}

Storage::Stats Storage::stats() {
  Storage::Stats stats;
#ifdef INCLUDE_SD
  uint8_t cardType = SD_MMC.cardType();
  switch (SD_MMC.cardType()) {
    case CARD_NONE:
      return stats;
    case CARD_MMC:
      stats.type = "MMC";
      break;
    case CARD_SD:
      stats.type = "SDSC";
      break;
    case CARD_SDHC:
      stats.type = "SDHC";
      break;
  }
  stats.physicalSize = SD_MMC.cardSize() / ONEMEG;
  stats.totalBytes = SD_MMC.totalBytes() / ONEMEG;
  stats.usedBytes = SD_MMC.usedBytes() / ONEMEG;
#endif
#ifdef _SPIFFS_H_
  stats.type = "SPIFFS";
#endif
#ifdef _LITTLEFS_H_
  stats.type = "LittleFS";
#endif
  return stats;
}

int Storage::setup() {
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
    return 1;
#endif
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
#endif
    bool res = SD_MMC.begin("/sdcard", true, formatIfMountFailed);
    if (res == false) {
      return 1;
    }
#if defined(CAMERA_MODEL_AI_THINKER)
    pinMode(4, OUTPUT);
    digitalWrite(4, 0); // set lamp pin fully off as sd_mmc library still initialises pin 4 in 1 line mode
#endif
    return 0;
#endif
  }
#ifdef _SPIFFS_H_
  if ((fs::SPIFFSFS*)&STORAGE == &SPIFFS) {
    res = SPIFFS.begin(formatIfMountFailed);
    return res ? 0 : 1;
  }
#endif
#ifdef _LITTLEFS_H_
  if ((fs::LittleFSFS*)&STORAGE == &LittleFS) {
    res = LittleFS.begin(formatIfMountFailed);
    return res ? 0 : 1;
  }
#endif
  return 1;
}