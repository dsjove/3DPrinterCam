#include "Camera.h"
#include "AppHardware.h"
#include "Hardware.h"
#include <exception>
#include <HardwareSerial.h>

void copy(camera_fb_t& dest, const camera_fb_t& src) {
  if (dest.buf == NULL || dest.len < src.len) {
      if (dest.buf) {
        free(dest.buf);
      }
      dest.buf = (uint8_t*)malloc(src.len);
  }
  dest.len = src.len;
  memcpy(dest.buf, src.buf, src.len);
  dest.width = src.width;
  dest.height = src.height;
  dest.format = src.format;
  dest.timestamp = src.timestamp;
}

Camera::Camera() {
}

void Camera::setup(AppHardware& hardware) {
  Serial.println("Configuring Camera...");
  camera_config_t config;
  ::memset(&config, 0, sizeof(config));
  assignPins(config);
  initCam(config, hardware);
}

void Camera::assignPins(camera_config_t& config) {
  uint8_t xclkMhz = 20; // camera clock rate MHz
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = xclkMhz * 1000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  // init with high specs to pre-allocate larger buffers
  config.fb_location = CAMERA_FB_IN_PSRAM;
#if CONFIG_IDF_TARGET_ESP32S3
  config.frame_size = FRAMESIZE_QSXGA; // 8M
#else
  config.frame_size = FRAMESIZE_UXGA;  // 4M
#endif  
  config.jpeg_quality = 10;
  config.fb_count = 4;

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

#if defined(LED_GPIO_NUM)
#if CONFIG_LED_ILLUMINATOR_ENABLED
  ledcSetup(LED_LEDC_CHANNEL, 5000, 8);
  ledcAttachPin(LED_GPIO_NUM, LED_LEDC_CHANNEL);
#endif
#endif
}

void Camera::initCam(camera_config_t& config, AppHardware& hardware) {
  // camera init
  if (psramFound()) {
    esp_err_t err = ESP_FAIL;
    uint8_t retries = 2;
    while (retries && err != ESP_OK) {
      err = esp_camera_init(&config);
      if (err != ESP_OK) {
        // power cycle the camera, provided pin is connected
        digitalWrite(PWDN_GPIO_NUM, 1);
        delay(100);
        digitalWrite(PWDN_GPIO_NUM, 0); 
        delay(100);
        retries--;
      }
    } 
    if (err != ESP_OK) {
      return;
    }
    sensor_t * s = esp_camera_sensor_get();
    switch (s->id.PID) {
      case (OV2640_PID):
        hardware.cameraModel = "OV2640";
        break;
      case (OV3660_PID):
        hardware.cameraModel = "OV3660";
        break;
      case (OV5640_PID):
        hardware.cameraModel = "OV5640";
        break;
      default:
        hardware.cameraModel = "Other";
        break;
    }
    // model specific corrections
    if (s->id.PID == OV3660_PID) {
      // initial sensors are flipped vertically and colors are a bit saturated
      s->set_vflip(s, 1);//flip it back
      s->set_brightness(s, 1);//up the brightness just a bit
      s->set_saturation(s, -2);//lower the saturation
    }
    //drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_SVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif
  }
}

void Camera::led(double intensity) { 
#if defined(LED_GPIO_NUM)
#if CONFIG_LED_ILLUMINATOR_ENABLED
    //TODO:  always appears full brightness
    uint32_t duty = (intensity * 255.0);
    ledcWrite(LED_LEDC_CHANNEL, duty);
#endif
#endif
}

framesize_t Camera::frameSize() {
  sensor_t * s = esp_camera_sensor_get();
  return s->status.framesize; 
}

camera_fb_t* Camera::processFrame()  {
  return esp_camera_fb_get();
}
