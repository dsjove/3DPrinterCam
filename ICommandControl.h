#pragma once

class ICommandControl {
  public:

  enum Code {
    None = -1,
    _First = 0,
    Signal = 0,
    Begin = 1,
    Frame = 2,
    End = 3, //TODO: filename
    SavePhoto = 4, //TODO: filename
    Flash = 5, //TODO: on/off
    _Size = 6,
  };

  struct Command {
    Code code = None;
    char info[128] = { 0 }; //TODO
  };

    virtual void onCommand(Command command);
};
