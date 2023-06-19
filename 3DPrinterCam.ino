#include "Camera.h"
#include "Storage.h"
#include "AVI.h"
#include <HardwareSerial.h>

Camera camera;
Storage storage;
AVI avi;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  if (camera.setup() != 0)
  {
    Serial.println("Camera Setup Fail");
    return;
  }
  if (storage.setup() != 0)
  {
    Serial.println("Storage Setup Fail");
    return;
  }
  if (avi.setup(camera.frameSize()) != 0)
  {
    Serial.println("AVI Setup Fail");
    return;
  }
  Serial.println("Running");
}

void loop() {
  if (Serial.available())
  {
    String command = Serial.readStringUntil('\n');
    if (command.indexOf("snapbegin") != -1)
    {
      Serial.println("snapbegin");
      avi.open();
    }
    else if (command.indexOf("snaplayer") != -1)
    {
      Serial.println("snaplayer");
      avi.record(camera.processFrame());
    }
    else if (command.indexOf("snapend") != -1)
    {
      Serial.println("snapend");
      avi.close();
    }
  }
}
