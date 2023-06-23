#pragma once
#include <stdint.h>

class AppHardware;

class Storage {
  public:
    Storage();
    void setup(AppHardware& hardware);
    uint64_t freeSpace();
};