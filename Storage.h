#include <string>

class Storage {
  public:
    struct Stats {
        std::string type;
        uint64_t physicalSize;
        uint64_t totalBytes;
        uint64_t usedBytes;
    };
    Storage();
    int setup();
    Stats stats();
};