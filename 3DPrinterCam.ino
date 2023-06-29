#include "AppHardware.h"
#include "Storage.h"
#include "SerialReader.h"
#include "Camera.h"
#include "AVI.h"
#include "Photo.h"
#include "NetworkConfig.h"
#include "WIFIConnection.h"
#include "CamServer.h"
#include "ESPTime.h"
#include "ICommandControl.h"
#include "Hardware.h"

/**
  CommandControl wires together all the component classes and exposes setup/loop for the Arduino.
**/

class CommandControl: IWifiDelegate, ICommandControl {
  public:
    CommandControl() 
    : serialReader(*this)
    , wifi(network, *this)
    , camServer(*this) {
      _lastfb.buf = NULL;
    }

    void setup() {
      storage.setup(hardware);
      serialReader.setup();
      camera.setup(hardware);
      log_i("%s", hardware.toJson().c_str());
      wifi.setup();
      avi.setup(camera.frameSize());
      camServer.setup();
      // TODO: wait until time set before get frame
      getFrame();
      signal();
    }

    void loop() {
      serialReader.loop();
    }

  private:
    AppHardware hardware;
    Storage storage;
    SerialReader serialReader;
    Camera camera;
    NetworkConfig network;
    WifiConnection wifi;
    AVI avi;
    Photo _photo;
    CamServer camServer;
    
    camera_fb_t _lastfb;

    void getFrame() {
      camera_fb_t* buff = camera.processFrame();
      copy(_lastfb, *buff);
      esp_camera_fb_return(buff);
      camServer.liveStream(&_lastfb);
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
      log_i("Command Begin");
      avi.open();
    }

    virtual void frame() {
      log_i("Command Frame");
      getFrame();
      avi.record(&_lastfb);
      _photo.save(&_lastfb);
    }

    virtual void end() {
      log_i("Command End");
      avi.close();
    }

    virtual void photo() {
      log_i("Command Photo");
      getFrame();
      _photo.save(&_lastfb);
    }
};

CommandControl commandControl;

void setup() {
  commandControl.setup();
}

void loop() {
  commandControl.loop();
}
