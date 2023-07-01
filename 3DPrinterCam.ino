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
    , _lastfb()
    , _commandQueue(xQueueCreate(5, sizeof(ICommandControl::Command))) {
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
      Serial.println("Setup Complete");
      signal();
      start();
      _serialReader.start();
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

    const xQueueHandle _commandQueue;
    const bool _photoOnFrame = true;
    camera_fb_t _lastfb;

    void getFrame() {
      camera_fb_t* buff = _camera.processFrame();
      copy(_lastfb, *buff);
      esp_camera_fb_return(buff);
      _camServer.liveStream(&_lastfb);
    }

    virtual void onCommand(Command command) {
      xQueueSend(_commandQueue, &command, 0);
    }

    void start() {
      TaskHandle_t xHandle = NULL;
      BaseType_t result = xTaskCreate(
        vTaskCode, "CommandControl", 4 * 1024, this, tskIDLE_PRIORITY+1, &xHandle);
    }

    static void vTaskCode(void* pvParameters) {
      CommandControl* reader = (CommandControl*)pvParameters;
      while(true) {
        reader->loop();
      }
    }

    void loop() {
      ICommandControl::Command command;
      if (xQueueReceive(_commandQueue, &command, portMAX_DELAY) == pdPASS) {
        switch (command.code) {
          case ICommandControl::Signal: signal(); break;
          case ICommandControl::Begin: begin(); break;
          case ICommandControl::Frame: frame(); break;
          case ICommandControl::End: end(); break;
          case ICommandControl::SavePhoto: savePhoto(); break;
          case ICommandControl::Flash: flash(); break;
          default: break;
        }
      }
    }

    void signal() {
      _camera.led(0.1);
      delay(125);
      _camera.led(0.0);
      delay(125);
      _camera.led(0.1);
      delay(125);
      _camera.led(0.0);
    }

    void begin() {
      log_d("Command Begin");
      _avi.open();
    }

    void frame() {
      log_d("Command Frame");
      getFrame();
      _avi.record(&_lastfb);
      if (_photoOnFrame) {
        _photo.save(&_lastfb);
      }
    }

    void end() {
      log_d("Command End");
      _avi.close();
    }

    void savePhoto() {
      log_d("Command Dave Photo");
      getFrame();
      _photo.save(&_lastfb);
    }

    void flash() {
    }
};

CommandControl commandControl;
void setup() { commandControl.setup(); }
void loop() {}
