#pragma once
#include <stdint.h>

struct AppHardware;

class Storage {
  public:
    Storage();
    void setup(AppHardware& hardware);
    uint64_t freeSpace();
};