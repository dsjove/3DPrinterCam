#include "AppHardware.h"
#include "Storage.h"
#include "Camera.h"
#include "AVI.h"
#include "NetworkConfig.h"
#include "WIFIConnection.h"
#include "CamServer.h"
#include "Globals.h"
#include <HardwareSerial.h>

/**
  CommandControl wires togher all the component classes and exposes setup/loop for the Arduino.
**/

/*
TODO: RuntimeState
- SD Free Space
- Free memory
- LED state
- timeelapse state (frame count, begin/end date, file name)
- last snap time-stamp
- wifi state and SSID, ap on
*/

class CommandControl: ICommandControl {
  public:
    CommandControl() 
    : wifi(network)
    , camServer(*this) {
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
      ping();
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

    virtual void begin() {
      avi.open();
    }

    virtual void snapLayer() {
      camera_fb_t* fb = camera.processFrame();
      avi.record(fb);
      esp_camera_fb_return(fb);
    }

    virtual void end() {
      avi.close();
    }

    virtual void snap() {
      camera_fb_t* fb = camera.processFrame();
      avi.snap(fb);
      esp_camera_fb_return(fb);
    }

    virtual void ping() {
      camera.led(0.25);
      delay(500);
      camera.led(0.0);
      delay(500);
      camera.led(0.25);
      delay(500);
      camera.led(0.0);
    }

    virtual void light(bool on) {
    }

  private:
    AppHardware hardware;
    Storage storage;
    Camera camera;
    NetworkConfig network;
    WifiConnection wifi;
    AVI avi;
    CamServer camServer;
};

CommandControl commandControl;

void setup() {
  commandControl.setup();
}

void loop() {
  commandControl.loop();
}
