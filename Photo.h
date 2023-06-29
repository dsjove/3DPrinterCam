#pragma once
#include "esp_camera.h"
#include <SD_MMC.h>

class Photo {
  public:
    Photo();
    bool save(camera_fb_t* fb);
  private:
    File last;
    time_t _lastPhoto = 0;
};