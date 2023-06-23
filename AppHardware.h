#pragma once
#include <WString.h>

class AppHardware {
  public:
    AppHardware();
    
    const String appName;
    const String appVersion;
    const String cameraBoard;

    String cameraModel;
    String storageType;
    uint64_t physicalSize;
    uint64_t totalBytes;

    String toString();

  private:
    String _description;
};
