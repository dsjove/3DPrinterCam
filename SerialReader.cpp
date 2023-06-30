#include "SerialReader.h"
#include "ICommandControl.h"
#include "Hardware.h"
#include <HardwareSerial.h>

SerialReader::SerialReader(ICommandControl& commandControl)
: _commandControl(commandControl)
,  _cmds({
      Command("cam_begin"),
      Command("cam_frame"),
      Command("cam_end"),
      Command("cam_photo")
    }) {
}

void SerialReader::vTaskCode(void* pvParameters) {
  SerialReader* reader = (SerialReader*)pvParameters;
  while(true) {
    reader->loop();
  }
}

void SerialReader::start() {
  TaskHandle_t xHandle = NULL;
  BaseType_t result = xTaskCreate(vTaskCode, "SerialReader", 4 * 1024, this, 12, &xHandle);
}

void SerialReader::loop() {
  int count = Serial.available();
  for (int i = 0; i <count; i++) {
    int value = Serial.read();
    int cmd = injest(value);
    switch (cmd) {
      case 0:
        _commandControl.begin();
        break;
      case 1:
        _commandControl.frame();
        break;
      case 2:
        _commandControl.end();
        break;
      case 3:
        _commandControl.photo();
        break;
    }
  }
}

int SerialReader::injest(int c) {
  if (c <= 0) return -1;
  size_t cmd = -1;
  for (int o = 0; o < sizeof(_cmds) / sizeof(Command); o++) {
      bool matched = _cmds[o].match((char)c);
      if (matched) {
          for (int o = 0; o < 7; o++) {
              _cmds[o].consume();
          }
          return o;
      }
  }
  return -1;
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
