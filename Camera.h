#include "esp_camera.h"

class Camera 
{
  public:
    Camera();
    int setup();
    framesize_t frameSize();
    camera_fb_t* processFrame();
};