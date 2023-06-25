#pragma once
#include "esp_camera.h"

class AppHardware;

/**
  Initializes the camera and LED. Provides simple interface of the hardware.
**/

class Camera {
  public:
    Camera();
    void setup(AppHardware& hardware);
    void led(double intensity);
    framesize_t frameSize();
    camera_fb_t* processFrame();
    
  private:
    void assignPins(camera_config_t& config);
    void initCam(camera_config_t& config, AppHardware& hardware);
};