#pragma once
#include <WString.h>

/**
  Compile time and Setup time constants.
**/

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

    String toJson();

  private:
    String _json;
};
