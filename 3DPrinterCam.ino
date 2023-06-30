#include "AppHardware.h"
#include "SerialReader.h"
#include "Storage.h"
#include "Camera.h"
#include "AVI.h"
#include "Photo.h"
#include "NetworkConfig.h"
#include "WIFIConnection.h"
#include "CamServer.h"
#include "ICommandControl.h"
#include "Hardware.h"

/**
  CommandControl wires together all the component classes and exposes setup/loop for the Arduino.
**/

class CommandControl: ICommandControl {
  public:
    CommandControl() 
    : _serialReader(*this)
    , _wifi(_network)
    , _camServer(*this)
    , _lastfb() {
      _lastfb.buf = NULL;
    }

    void setup() {
      Serial.begin(115200);
      Serial.setDebugOutput(SERIALDEBUG);
      Serial.println("Setup Begin");
      _storage.setup(_hardware);
      _camera.setup(_hardware);
      Serial.println(_hardware.toString());
      _avi.setup(_camera.frameSize());
      _wifi.setup();
      _camServer.setup();
      getFrame();
      signal();
      _serialReader.start();
      Serial.println("Setup Complete");
    }

    void loop() {
    }

  private:
    AppHardware _hardware;
    Storage _storage;
    SerialReader _serialReader;
    Camera _camera;
    AVI _avi;
    NetworkConfig _network;
    WifiConnection _wifi;
    Photo _photo;
    CamServer _camServer;
    
    const bool _photoOnFrame = true;
    camera_fb_t _lastfb;

    void getFrame() {
      camera_fb_t* buff = _camera.processFrame();
      copy(_lastfb, *buff);
      esp_camera_fb_return(buff);
      _camServer.liveStream(&_lastfb);
    }
    
    virtual void ping(bool success) {
      _avi.detectIdle();
    }

    virtual void signal() {
      _camera.led(0.1);
      delay(125);
      _camera.led(0.0);
      delay(125);
      _camera.led(0.1);
      delay(125);
      _camera.led(0.0);
    }

    virtual void begin() {
      log_d("Command Begin");
      _avi.open();
    }

    virtual void frame() {
      log_d("Command Frame");
      getFrame();
      _avi.record(&_lastfb);
      if (_photoOnFrame) {
        _photo.save(&_lastfb);
      }
    }

    virtual void end() {
      log_d("Command End");
      _avi.close();
    }

    virtual void photo() {
      log_d("Command Photo");
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
