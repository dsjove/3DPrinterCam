#pragma once
#include <stdint.h>
#include "esp_camera.h"
#include <SD_MMC.h>

/**
  Sames captures images in AVI and jpeg files.
**/

struct AVIStatus {
  time_t aviStart = 0;
  time_t aviEnd = 0;
  int aviFrameCoount = 0;
  time_t lastSnap = 0;
};

class AVI {
  public:
    AVI();
    void setup(framesize_t frameSize);
    AVIStatus status() const { return _status; }
    void open();
    bool snap(camera_fb_t* fb);
    bool record(camera_fb_t* fb);
    bool close();
    void detectIdle();

  private:
    struct IDXs {
      size_t idxPtr = 0;
      size_t idxOffset = 0;
      size_t moviSize = 0;
      size_t indexLen = 0;
      uint8_t* idxBuf = NULL;
    };
    IDXs idx;
    const int tlPlaybackFPS = 1;  // rate to playback the timelapse, min 1 
    const int maxFrames = 20000; // maximum number of frames in video before auto close 
    const uint32_t SAMPLE_RATE = 16000; // sample rate used
    const bool haveSoundFile = false;
    const size_t audSize = 0;
    void prepAviIndex();
    void buildAviHdr(uint8_t FPS, uint8_t frameType, uint16_t frameCnt);
    void buildAviIdx(size_t dataSize, bool isVid);
    size_t writeAviIndex(uint8_t* clientBuf, size_t buffSize);
    void finalizeAviIndex(uint16_t frameCnt);

    #define RAMSIZE (1024 * 8) // set this to multiple of SD card sector size (512 or 1024 bytes)
    #define CHUNK_HDR 8 // bytes per jpeg hdr in AVI 
    uint8_t iSDbuffer[(RAMSIZE + CHUNK_HDR) * 2];
    #define AVI_HEADER_LEN 310 // AVI header length
    uint8_t aviHeader[AVI_HEADER_LEN];
    uint8_t fsizePtr = 0; // index to frameData[] for record
    int frameCntTL = 0;
    File tlFile;
    File last;
    uint8_t FPS = 0;

    AVIStatus _status;
};
