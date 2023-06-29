#pragma once

class ICommandControl {
  public:
    virtual void signal() = 0;
    virtual void begin() = 0;
    virtual void frame() = 0;
    virtual void end() = 0;
    virtual void photo() = 0;
    //TODO: Light
    //TODO: Stats
};
