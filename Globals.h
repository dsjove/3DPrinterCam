#pragma once
#include <WString.h>
#include <time.h>

#include "MyConfig.h"

#if defined(CAMERA_MODEL_AI_THINKER)
#define CAM_BOARD "CAMERA_MODEL_AI_THINKER"
#elif defined(CAMERA_MODEL_ESP32S3_EYE)
#define CAM_BOARD "CAMERA_MODEL_ESP32S3_EYE"
#elif defined(CAMERA_MODEL_XIAO_ESP32S3)
#define CAM_BOARD "CAMERA_MODEL_XIAO_ESP32S3"
#else
#define CAM_BOARD "OTHER"
#endif

#include "camera_pins.h"

#define ONEMEG (1024 * 1024)

class AppHardware {
  public:
    String cameraBoard = CAM_BOARD;
    String cameraModel;
    String storageType;
    uint64_t physicalSize = 0;
    uint64_t totalBytes = 0;

    String toString();
  private:
    String _description;
};

class RuntimeStats {
  public:
    RuntimeStats(const char* caller);
    String toString();
};

time_t getEpoch();

void dateFormat(char* inBuff, size_t inBuffLen, bool isFolder);
