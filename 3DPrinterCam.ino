#include "AppHardware.h"
#include "Storage.h"
#include "Camera.h"
#include "AVI.h"
#include "NetworkConfig.h"
#include "WIFIConnection.h"
#include "CamServer.h"
#include "ESPTime.h"
#include "Globals.h"
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

/**
  CommandControl wires together all the component classes and exposes setup/loop for the Arduino.
**/

class CommandControl: IWifiDelegate, ICommandControl {
  public:
    CommandControl() 
    : wifi(network, *this)
    , camServer(*this) {
      _lastfb.buf = NULL;
    }

    void setup() {
      Serial.begin(115200);
      Serial.setDebugOutput(true);
      storage.setup(hardware);
      camera.setup(hardware);
      Serial.println(hardware.toJson());
      wifi.setup();
      avi.setup(camera.frameSize());
      camServer.setup();
      // TODO: wait until time set before get frame
      getFrame();
      signal();
    }

    void loop() {
      if (Serial.available()) {
        //TODO: This does not work with a real printer!
        //TODO: is this thread save with CamServer?
        String command = Serial.readStringUntil('\n');
        if (command.indexOf("cam_begin") != -1) {
          Serial.println("cam_begin");
          begin();
        }
        else if (command.indexOf("cam_frame") != -1) {
          Serial.println("cam_frame");
          frame();
        }
        else if (command.indexOf("cam_end") != -1) {
          Serial.println("cam_end");
          end();
        }
        else if (command.indexOf("cam_photo") != -1) {
          Serial.println("cam_photo");
          photo();
        }
      }
    }

    virtual void ping(bool success) {
      if (success) {
        ESPTime::getLocalNTP();
      }
      avi.detectIdle();
    }

    virtual void signal() {
      camera.led(0.1);
      delay(500);
      camera.led(0.0);
      delay(500);
      camera.led(0.1);
      delay(500);
      camera.led(0.0);
    }

    virtual void begin() {
      avi.open();
    }

    virtual void frame() {
      getFrame();
      avi.record(&_lastfb);
    }

    virtual void end() {
      avi.close();
    }

    virtual void photo() {
      getFrame();
      avi.photo(&_lastfb);
    }

  private:
    AppHardware hardware;
    Storage storage;
    Camera camera;
    NetworkConfig network;
    WifiConnection wifi;
    AVI avi;
    CamServer camServer;

    camera_fb_t _lastfb;

    void getFrame() {
      camera_fb_t* buff = camera.processFrame();
      copy(_lastfb, *buff);
      esp_camera_fb_return(buff);
      camServer.liveStream(&_lastfb);
    }
};

CommandControl commandControl;

void setup() {
  commandControl.setup();
}

void loop() {
  commandControl.loop();
}
