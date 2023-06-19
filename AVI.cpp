#include "AVI.h"
#include "Globals.h"

/* 
Generate AVI format for recorded videos
s60sc 2020, 2022
*/

/* AVI file format:
header:
 310 bytes
per jpeg:
 4 byte 00dc marker
 4 byte jpeg size
 jpeg frame content
0-3 bytes filler to align on DWORD boundary
per PCM (audio file)
 4 byte 01wb marker
 4 byte pcm size
 pcm content
 0-3 bytes filler to align on DWORD boundary
footer:
 4 byte idx1 marker
 4 byte index size
 per jpeg:
  4 byte 00dc marker
  4 byte 0000
  4 byte jpeg location
  4 byte jpeg size
 per pcm:
  4 byte 01wb marker
  4 byte 0000
  4 byte pcm location
  4 byte pcm size
*/

// avi header data
static const uint8_t dcBuf[4] = {0x30, 0x30, 0x64, 0x63};   // 00dc
static const uint8_t wbBuf[4] = {0x30, 0x31, 0x77, 0x62};   // 01wb
static const uint8_t idx1Buf[4] = {0x69, 0x64, 0x78, 0x31}; // idx1
static const uint8_t zeroBuf[4] = {0x00, 0x00, 0x00, 0x00}; // 0000

static const uint8_t aviHeaderTemplate[AVI_HEADER_LEN] = { // AVI header template
  0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x41, 0x56, 0x49, 0x20, 0x4C, 0x49, 0x53, 0x54,
  0x16, 0x01, 0x00, 0x00, 0x68, 0x64, 0x72, 0x6C, 0x61, 0x76, 0x69, 0x68, 0x38, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x53, 0x54, 0x6C, 0x00, 0x00, 0x00,
  0x73, 0x74, 0x72, 0x6C, 0x73, 0x74, 0x72, 0x68, 0x30, 0x00, 0x00, 0x00, 0x76, 0x69, 0x64, 0x73,
  0x4D, 0x4A, 0x50, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x74, 0x72, 0x66,
  0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x18, 0x00, 0x4D, 0x4A, 0x50, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x4C, 0x49, 0x53, 0x54, 0x56, 0x00, 0x00, 0x00, 
  0x73, 0x74, 0x72, 0x6C, 0x73, 0x74, 0x72, 0x68, 0x30, 0x00, 0x00, 0x00, 0x61, 0x75, 0x64, 0x73,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x11, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x11, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x73, 0x74, 0x72, 0x66,
  0x12, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x11, 0x2B, 0x00, 0x00, 0x11, 0x2B, 0x00, 0x00,
  0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 
  0x4C, 0x49, 0x53, 0x54, 0x00, 0x00, 0x00, 0x00, 0x6D, 0x6F, 0x76, 0x69,
};

struct FrameSizeStruct {
  uint8_t frameWidth[2];
  uint8_t frameHeight[2];
};

// indexed by frame type - needs to be consistent with sensor.h framesize_t enum
static const FrameSizeStruct frameSizeData[] = {
  {{0x60, 0x00}, {0x60, 0x00}}, // 96X96
  {{0xA0, 0x00}, {0x78, 0x00}}, // qqvga 
  {{0xB0, 0x00}, {0x90, 0x00}}, // qcif 
  {{0xF0, 0x00}, {0xB0, 0x00}}, // hqvga 
  {{0xF0, 0x00}, {0xF0, 0x00}}, // 240X240
  {{0x40, 0x01}, {0xF0, 0x00}}, // qvga 
  {{0x90, 0x01}, {0x28, 0x01}}, // cif 
  {{0xE0, 0x01}, {0x40, 0x01}}, // hvga 
  {{0x80, 0x02}, {0xE0, 0x01}}, // vga 
  {{0x20, 0x03}, {0x58, 0x02}}, // svga 
  {{0x00, 0x04}, {0x00, 0x03}}, // xga 
  {{0x00, 0x05}, {0xD0, 0x02}}, // hd
  {{0x00, 0x05}, {0x00, 0x04}}, // sxga
  {{0x40, 0x06}, {0xB0, 0x04}}  // uxga 
};

#define IDX_ENTRY 16 // bytes per index entry

void AVI::prepAviIndex() {
   // prep buffer to store index data, gets appended to end of file
  if (idx.idxBuf == NULL) idx.idxBuf = (uint8_t*)ps_malloc((maxFrames+1)*IDX_ENTRY); // include some space for audio index
  memcpy(idx.idxBuf, idx1Buf, 4); // index header
  idx.idxPtr = CHUNK_HDR;  // leave 4 bytes for index size
  idx.moviSize = idx.indexLen = 0;
}

void AVI::buildAviHdr(uint8_t FPS, uint8_t frameType, uint16_t frameCnt) {
  // update AVI header template with file specific details
  size_t aviSize = idx.moviSize + AVI_HEADER_LEN + ((CHUNK_HDR+IDX_ENTRY) * (frameCnt+(haveSoundFile?1:0))); // AVI content size 
  // update aviHeader with relevant stats
  memcpy(aviHeader+4, &aviSize, 4);
  uint32_t usecs = (uint32_t)round(1000000.0f / FPS); // usecs_per_frame 
  memcpy(aviHeader+0x20, &usecs, 4); 
  memcpy(aviHeader+0x30, &frameCnt, 2);
  memcpy(aviHeader+0x8C, &frameCnt, 2);
  memcpy(aviHeader+0x84, &FPS, 1);
  uint32_t dataSize = idx.moviSize + ((frameCnt+(haveSoundFile?1:0)) * CHUNK_HDR) + 4; 
  memcpy(aviHeader+0x12E, &dataSize, 4); // data size 
  uint8_t withAudio = 2; // increase number of streams for audio
  if (true) memcpy(aviHeader+0x100, zeroBuf, 4); // no audio for timelapse
  else {
    if (haveSoundFile) memcpy(aviHeader+0x38, &withAudio, 1); 
    memcpy(aviHeader+0x100, &audSize, 4); // audio data size
  }
  // apply video framesize to avi header
  memcpy(aviHeader+0x40, frameSizeData[frameType].frameWidth, 2);
  memcpy(aviHeader+0xA8, frameSizeData[frameType].frameWidth, 2);
  memcpy(aviHeader+0x44, frameSizeData[frameType].frameHeight, 2);
  memcpy(aviHeader+0xAC, frameSizeData[frameType].frameHeight, 2);
  // apply audio details to avi header
  memcpy(aviHeader+0xF8, &SAMPLE_RATE, 4);
  uint32_t bytesPerSec = SAMPLE_RATE * 2;
  memcpy(aviHeader+0x104, &bytesPerSec, 4); // suggested buffer size
  memcpy(aviHeader+0x11C, &SAMPLE_RATE, 4);
  memcpy(aviHeader+0x120, &bytesPerSec, 4); // bytes per sec

  // reset state for next recording
  idx.moviSize = idx.idxOffset = idx.idxPtr = 0;
}

void AVI::buildAviIdx(size_t dataSize, bool isVid) {
   // build AVI video index into buffer - 16 bytes per frame
  // called from saveFrame() for each frame
  idx.moviSize += dataSize;
  if (isVid) memcpy(idx.idxBuf+idx.idxPtr, dcBuf, 4);
  else memcpy(idx.idxBuf+idx.idxPtr, wbBuf, 4);
  memcpy(idx.idxBuf+idx.idxPtr+4, zeroBuf, 4);
  memcpy(idx.idxBuf+idx.idxPtr+8, &idx.idxOffset, 4); 
  memcpy(idx.idxBuf+idx.idxPtr+12, &dataSize, 4); 
  idx.idxOffset += dataSize + CHUNK_HDR;
  idx.idxPtr += IDX_ENTRY; 
}

size_t AVI::writeAviIndex(byte* clientBuf, size_t buffSize) {
  // write completed index to avi file
  // called repeatedly from closeAvi() until return 0
  if (idx.idxPtr < idx.indexLen) {
    if (idx.indexLen-idx.idxPtr > buffSize) {
      memcpy(clientBuf, idx.idxBuf+idx.idxPtr, buffSize);
      idx.idxPtr += buffSize;
      return buffSize;
    } else {
      // final part of index
      size_t final = idx.indexLen-idx.idxPtr;
      memcpy(clientBuf, idx.idxBuf+idx.idxPtr, final);
      idx.idxPtr = idx.indexLen;
      return final;    
    }
  }
  return idx.idxPtr = 0;
}

void AVI::finalizeAviIndex(uint16_t frameCnt) {
  // update index with size
  uint32_t sizeOfIndex = (frameCnt+(haveSoundFile?1:0))*IDX_ENTRY;
  memcpy(idx.idxBuf+4, &sizeOfIndex, 4); // size of index 
  idx.indexLen = sizeOfIndex + CHUNK_HDR;
  idx.idxPtr = 0; // pointer to index buffer
}

struct FrameStruct {
  const char* frameSizeStr;
  const uint16_t frameWidth;
  const uint16_t frameHeight;
  const uint16_t defaultFPS;
  const uint8_t scaleFactor; // (0..4)
  const uint8_t sampleRate; // (1..N)
};

static const FrameStruct frameData[] = {
  {"96X96", 96, 96, 30, 1, 1},   // 2MP sensors
  {"QQVGA", 160, 120, 30, 1, 1},
  {"QCIF", 176, 144, 30, 1, 1}, 
  {"HQVGA", 240, 176, 30, 2, 1}, 
  {"240X240", 240, 240, 30, 2, 1}, 
  {"QVGA", 320, 240, 30, 2, 1}, 
  {"CIF", 400, 296, 30, 2, 1},  
  {"HVGA", 480, 320, 30, 2, 1}, 
  {"VGA", 640, 480, 20, 3, 1}, 
  {"SVGA", 800, 600, 20, 3, 1}, 
  {"XGA", 1024, 768, 5, 3, 1},   
  {"HD", 1280, 720, 5, 3, 1}, 
  {"SXGA", 1280, 1024, 5, 3, 1}, 
  {"UXGA", 1600, 1200, 5, 3, 1},  
  {"FHD", 920, 1080, 5, 3, 1},    // 3MP Sensors
  {"P_HD", 720, 1280, 5, 3, 1},
  {"P_3MP", 864, 1536, 5, 3, 1},
  {"QXGA", 2048, 1536, 5, 4, 1},
  {"QHD", 2560, 1440, 5, 4, 1},   // 5MP Sensors
  {"WQXGA", 2560, 1600, 5, 4, 1},
  {"P_FHD", 1080, 1920, 5, 3, 1},
  {"QSXGA", 2560, 1920, 4, 4, 1}
};

int AVI::setup(framesize_t frameSize)
{
  fsizePtr = frameSize; 
  FPS = frameData[fsizePtr].defaultFPS;
  return 0;
}

#define FILE_EXT "avi"
#define TLTEMP "/current.tl"
void AVI::open() {
  if (frameCntTL > 0)
  {
    close();
  }
  dateFormat(partName, sizeof(partName), true);
  SD_MMC.mkdir(partName); // make date folder if not present
  dateFormat(partName, sizeof(partName), false);
  int tlen = snprintf(TLname, FILE_NAME_LEN - 1, "%s_%s_%u.%s", 
    partName, frameData[fsizePtr].frameSizeStr, tlPlaybackFPS, FILE_EXT);
  if (tlen > FILE_NAME_LEN - 1);// LOG_WRN("file name truncated");
  if (SD_MMC.exists(TLTEMP)) SD_MMC.remove(TLTEMP);
  tlFile = SD_MMC.open(TLTEMP, FILE_WRITE);
  tlFile.write(aviHeader, AVI_HEADER_LEN); // space for header
  prepAviIndex();
  //LOG_INF("Started time lapse file %s, duration %u mins, for %u frames",  TLname, tlDurationMins, requiredFrames);
  frameCntTL = 1;
}

bool AVI::record(camera_fb_t* fb) {
  if (!fb) return false;
  if (frameCntTL == 0) {    
    open();
  }
  uint8_t hdrBuff[CHUNK_HDR];
  memcpy(hdrBuff, dcBuf, 4); 
  // align end of jpeg on 4 byte boundary for AVI
  uint16_t filler = (4 - (fb->len & 0x00000003)) & 0x00000003; 
  uint32_t jpegSize = fb->len + filler;
  memcpy(hdrBuff+4, &jpegSize, 4);
  tlFile.write(hdrBuff, CHUNK_HDR); // jpeg frame details
  tlFile.write(fb->buf, jpegSize);
  buildAviIdx(jpegSize, true); // save avi index for frame
  frameCntTL++;
  if (frameCntTL > maxFrames) {
    close();
  }
}

bool AVI::close() {
  Serial.println("Close");
  buildAviHdr(tlPlaybackFPS, fsizePtr, --frameCntTL);
  // add index
  finalizeAviIndex(frameCntTL);
  size_t idxLen = 0;
  do {
    idxLen = writeAviIndex(iSDbuffer, RAMSIZE);
    tlFile.write(iSDbuffer, idxLen);
  } while (idxLen > 0);
  // add header
  tlFile.seek(0, SeekSet); // start of file
  tlFile.write(aviHeader, AVI_HEADER_LEN);
  tlFile.close(); 
  SD_MMC.rename(TLTEMP, TLname);
  frameCntTL = 0;
  //LOG_DBG("Finished time lapse");
}