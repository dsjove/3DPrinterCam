#pragma once
#include <WString.h>

class ICommandControl;

class SerialReader {
  public:
    SerialReader(ICommandControl& commandControl);
    void setup();
    void loop();

  private:
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
    Command _cmds[4];

    int injest(int c);
};
