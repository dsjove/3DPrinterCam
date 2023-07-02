#include "SerialReader.h"
#include "Hardware.h"
#include <HardwareSerial.h>

SerialReader::SerialReader(ICommandControl& commandControl)
: _commandControl(commandControl)
,  _cmds({
      Command("cam_signal"),
      Command("cam_begin"),
      Command("cam_frame"),
      Command("cam_end"),
      Command("cam_photo"),
      Command("cam_flash")
    }) {
}

void SerialReader::vTaskCode(void* pvParameters) {
  SerialReader* reader = (SerialReader*)pvParameters;
  while(true) {
    reader->loop();
    taskYIELD();
  }
}

void SerialReader::start() {
  TaskHandle_t xHandle = NULL;
  BaseType_t result = xTaskCreate(
    vTaskCode, "SerialReader", 4 * 1024, this, tskIDLE_PRIORITY+2, &xHandle);
}

void SerialReader::loop() {
  int count = Serial.available();
  for (int i = 0; i <count; i++) {
    int value = Serial.read();
    ICommandControl::Command cmd;
    cmd.code = injest(value);
    if (cmd.code != ICommandControl::None) {
      _commandControl.onCommand(cmd);
    }
  }
}

ICommandControl::Code SerialReader::injest(int c) {
  if (c > 0) {
    for (size_t o = ICommandControl::Code::_First; o < ICommandControl::Code::_Size; o++) {
        bool matched = _cmds[o].match((char)c);
        if (matched) {
            for (size_t o = ICommandControl::Code::_First; o < ICommandControl::Code::_Size; o++) {
                _cmds[o].consume();
            }
            return (ICommandControl::Code)o;
        }
    }
  }
  return ICommandControl::None;
}

SerialReader::Command::Command(const String& word)
: _word(word)
, _lastMatch(word.length())
{
}

bool SerialReader::Command::match(char c) {
  size_t testPosition = _lastMatch >= _word.length() ? 0 : _lastMatch + 1;
  if (_word[testPosition] == c) {
      _lastMatch = testPosition;
      if (testPosition == _word.length() - 1) {
          return true;
      }
  }
  return false;
}

void SerialReader::Command::consume() {
  _lastMatch = _word.length();
}
