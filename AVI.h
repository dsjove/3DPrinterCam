#include <string.h>
#include <stdint.h>
#include "esp_camera.h"
#include <SD_MMC.h>

class AVI {
  public:
    int setup(framesize_t frameSize);
    void open();
    bool record(camera_fb_t* fb);
    bool close();

  private:
    struct IDXs
    {
      size_t idxPtr = 0;
      size_t idxOffset = 0;
      size_t moviSize = 0;
      size_t indexLen = 0;
      uint8_t* idxBuf = NULL;
    };
    IDXs idx;
    const int maxFrames = 20000; // maximum number of frames in video before auto close 
    const uint32_t SAMPLE_RATE = 16000; // sample rate used
    const bool haveSoundFile = false;
    const size_t audSize = 0;
    void prepAviIndex();
    void buildAviHdr(uint8_t FPS, uint8_t frameType, uint16_t frameCnt);
    void buildAviIdx(size_t dataSize, bool isVid);
    size_t writeAviIndex(uint8_t* clientBuf, size_t buffSize);
    void finalizeAviIndex(uint16_t frameCnt);

    #define ONEMEG (1024 * 1024)
    #define MAX_JPEG ONEMEG/2 // UXGA jpeg frame buffer at highest quality 375kB rounded up
    #define RAMSIZE (1024 * 8) // set this to multiple of SD card sector size (512 or 1024 bytes)
    #define CHUNK_HDR 8 // bytes per jpeg hdr in AVI 
    uint8_t iSDbuffer[(RAMSIZE + CHUNK_HDR) * 2];
    #define AVI_HEADER_LEN 310 // AVI header length
    uint8_t aviHeader[AVI_HEADER_LEN];
    uint8_t fsizePtr; // index to frameData[] for record

    #define FILE_NAME_LEN 64
    char partName[FILE_NAME_LEN] = "";
    char TLname[FILE_NAME_LEN] = "";
    int frameCntTL = 0;
    File tlFile;
    
    uint8_t FPS;
    int tlPlaybackFPS = 1;  // rate to playback the timelapse, min 1 
};
