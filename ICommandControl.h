#pragma once

class ICommandControl {
  public:

  enum Code {
    Signal,
    Begin,
    Frame,
    End,
    SavePhoto,
    Flash,
  };

  struct Command {
    Code code = Signal;
    char info[128] = { 0 };
  };

    virtual void onCommand(Command command);
};
