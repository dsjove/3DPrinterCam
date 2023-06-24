#include "AppHardware.h"
#include "Storage.h"
#include "Camera.h"
#include "AVI.h"
#include "NetworkConfig.h"
#include "WIFIConnection.h"
#include "Globals.h"
#include <HardwareSerial.h>

AppHardware hardware;
Storage storage;
Camera camera;
NetworkConfig network;
WifiConnection wifi(network);
AVI avi;

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
  //TODO: CamServer
  Serial.println("Running");
}

void loop() {
  if (Serial.available()) {
    //TODO: is this the best way to parse the commands?
    String command = Serial.readStringUntil('\n');
    if (command.indexOf("snapbegin") != -1) {
      Serial.println("snapbegin");
      avi.open();
    }
    else if (command.indexOf("snaplayer") != -1) {
      Serial.println("snaplayer");
      camera_fb_t* fb = camera.processFrame();
      avi.record(fb);
      esp_camera_fb_return(fb);
    }
    else if (command.indexOf("snappic") != -1) {
      Serial.println("snappic");
      camera_fb_t* fb = camera.processFrame();
      avi.snap(fb);
      esp_camera_fb_return(fb);
    }
    else if (command.indexOf("snapend") != -1) {
      Serial.println("snapend");
      avi.close();
    }
  }
}
