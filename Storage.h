#pragma once
#include <stdint.h>

class AppHardware;

/**
  Setup access to file system.
  This will need to provide file listings, etc
**/

struct StorageStatus {
  uint64_t freeStorage = 0;
  uint64_t freeHeap = 0;
  uint64_t largestBlock = 0;
  uint64_t freePSRAM = 0;
};

class Storage {
  public:
    Storage();
    void setup(AppHardware& hardware);
    StorageStatus status() const;
};