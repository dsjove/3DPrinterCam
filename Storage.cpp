#include "Storage.h"
#include "Globals.h"
#include <SD_MMC.h>

Storage::Storage()
{
}

#ifdef INCLUDE_SD
static void infoSD() {
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) ;//LOG_WRN("No SD card attached");
  else {
    char typeStr[8] = "UNKNOWN";
    if (cardType == CARD_MMC) strcpy(typeStr, "MMC");
    else if (cardType == CARD_SD) strcpy(typeStr, "SDSC");
    else if (cardType == CARD_SDHC) strcpy(typeStr, "SDHC");

    uint64_t cardSize, totBytes, useBytes = 0;
    cardSize = SD_MMC.cardSize() / ONEMEG;
    totBytes = SD_MMC.totalBytes() / ONEMEG;
    useBytes = SD_MMC.usedBytes() / ONEMEG;
    //LOG_INF("SD card type %s, Size: %lluMB, Used space: %lluMB, of total: %lluMB",
    //         typeStr, cardSize, useBytes, totBytes);
  }
}

static bool prepSD_MMC() {
  /* open SD card in MMC 1 bit mode
     MMC4  MMC1  ESP32 ESP32S3
      D2          12
      D3    ..    13
      CMD  CMD    15    38
      CLK  CLK    14    39
      D0   D0     2     40
      D1          4
  */
  bool formatIfMountFailed = true;
  bool res = false;
  if (psramFound()) heap_caps_malloc_extmem_enable(5); // small number to force vector into psram
  if (psramFound()) heap_caps_malloc_extmem_enable(4096);
#if CONFIG_IDF_TARGET_ESP32S3
#if !defined(SD_MMC_CLK)
  //LOG_ERR("SD card pins not defined");
  res = false;
#endif
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
#endif
  
  res = SD_MMC.begin("/sdcard", true, formatIfMountFailed);
#if defined(CAMERA_MODEL_AI_THINKER)
  pinMode(4, OUTPUT);
  digitalWrite(4, 0); // set lamp pin fully off as sd_mmc library still initialises pin 4 in 1 line mode
#endif 
  if (res) {
    infoSD();
    res = true;
  } else {
    //LOG_ERR("SD card mount failed");
    res = false;
  }
  return res;
}
#endif

int Storage::setup() {

  // start required storage device (SD card or flash file system)
  char startupFailure[50] = {0};
  static char fsType[10] = {0};
  bool res = false;
#ifdef INCLUDE_SD  
  if ((fs::SDMMCFS*)&STORAGE == &SD_MMC) {
    strcpy(fsType, "SD_MMC");
    res = prepSD_MMC();
    if (!res) sprintf(startupFailure, "Startup Failure: Check SD card inserted");
    else ;//checkFreeSpace();
    return res ? 0 : 1; 
  }
#endif

  // One of SPIFFS or LittleFS
  if (!strlen(fsType)) {
#ifdef _SPIFFS_H_
    if ((fs::SPIFFSFS*)&STORAGE == &SPIFFS) {
      strcpy(fsType, "SPIFFS");
      res = SPIFFS.begin(formatIfMountFailed);
    }
#endif
#ifdef _LITTLEFS_H_
    if ((fs::LittleFSFS*)&STORAGE == &LittleFS) {
      strcpy(fsType, "LittleFS");
      res = LittleFS.begin(formatIfMountFailed);
      ramLogPrep();
      // create data folder if not present
      if (res && !LittleFS.exists(DATA_DIR)) LittleFS.mkdir(DATA_DIR);
    }
#endif
    if (res) {  
      // list details of files on file system
      const char* rootDir = !strcmp(fsType, "LittleFS") ? "/data" : "/";
      File root = STORAGE.open(rootDir);
      File file = root.openNextFile();
      while (file) {
        //LOG_INF("File: %s, size: %u", file.path(), file.size());
        file = root.openNextFile();
      }
      //LOG_INF("%s: Total bytes %lld, Used bytes %lld", fsType, (uint64_t)(STORAGE.totalBytes()), (uint64_t)(STORAGE.usedBytes())); 
    }
  } else {
    sprintf(startupFailure, "Failed to mount %s", fsType);  
  }
  //LOG_INF("Sketch size %dkB", ESP.getSketchSize() / 1024);
  //debugMemory("startStorage");
  return res ? 0 : 1;
}