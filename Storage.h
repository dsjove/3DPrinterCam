#pragma once
#include <stdint.h>

class AppHardware;

/**
  Setup access to file system.
  This will need to provide file listings, etc
**/

class Storage {
  public:
    Storage();
    void setup(AppHardware& hardware);
    uint64_t freeSpace();
};