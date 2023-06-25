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

/**
  CommandControl wires together all the component classes and exposes setup/loop for the Arduino.
**/

class CommandControl: IWifiDelegate, ICommandControl {
  public:
    CommandControl() 
    : wifi(network, *this)
    , camServer(*this)
    , _lastfb(NULL) {
    }

    void setup() {
      Serial.begin(115200);
      Serial.setDebugOutput(true);
      storage.setup(hardware);
      //TODO: Config File
      camera.setup(hardware);
      Serial.println(hardware.toString());
      wifi.setup();
      Serial.println(network.toString());
      avi.setup(camera.frameSize());
      camServer.setup();
      // TODO: wait until time set
      getFrame();
      signal();
    }

    void loop() {
      if (Serial.available()) {
        //TODO: is this the best way to parse the commands?
        String command = Serial.readStringUntil('\n');
        if (command.indexOf("snapbegin") != -1) {
          Serial.println("snapbegin");
          begin();
        }
        else if (command.indexOf("snaplayer") != -1) {
          Serial.println("snaplayer");
          snapLayer();
        }
        else if (command.indexOf("snappic") != -1) {
          Serial.println("snappic");
          snap();
        }
        else if (command.indexOf("snapend") != -1) {
          Serial.println("snapend");
          end();
        }
      }
    }

    virtual void ping(bool success) {
      if (success) {
        ESPTime::getLocalNTP();
      }
      avi.detectIdle();
    }

    virtual void begin() {
      avi.open();
    }

    virtual void snapLayer() {
      getFrame();
      avi.record(getFrame());
    }

    virtual void snap() {
      avi.snap(getFrame());
    }

    virtual void end() {
      avi.close();
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

    virtual void light(bool on) {
      camera.led(on ? 1.0 : 0.0);
    }

  private:
    AppHardware hardware;
    Storage storage;
    Camera camera;
    NetworkConfig network;
    WifiConnection wifi;
    AVI avi;
    CamServer camServer;

    camera_fb_t* _lastfb;

    camera_fb_t* getFrame() {
      if (_lastfb) {
        esp_camera_fb_return(_lastfb);
        _lastfb = NULL;
      }
      _lastfb = camera.processFrame();
      camServer.liveStream(_lastfb);
      return _lastfb;
    }
};

CommandControl commandControl;

void setup() {
  commandControl.setup();
}

void loop() {
  commandControl.loop();
}
