#pragma once
#include "ICommandControl.h"
#include <WString.h>

class SerialReader {
  public:
    SerialReader(ICommandControl& commandControl);
    void start();

  private:
    static void vTaskCode(void* pvParameters);
    void loop();

    struct Command {
      public:
        Command(const String& word);
        bool match(char c);
        void consume();
    
      private:
        const String _word;
        size_t _lastMatch;
    };

    ICommandControl& _commandControl;
    Command _cmds[ICommandControl::Code::_Size];

    ICommandControl::Code injest(int c);
};
