#include "AppHardware.h"
#include "Globals.h"
#include <string>

AppHardware::AppHardware()
: appName(APP_NAME)
, appVersion(APP_VER)
, cameraBoard(CAM_BOARD)
, cameraModel()
, storageType()
, physicalSize(0)
, totalBytes(0) {
}

String AppHardware::toJson() {
  static char buffer[256] = {0};
  //TODO: real JSON
  if (_json.length() == 0) {
    sprintf(buffer, "%s:%s %s:%s %s:%d:%d", 
      appName.c_str(), appVersion.c_str(),
      cameraBoard.c_str(), cameraModel.c_str(), 
      storageType.c_str(), physicalSize, totalBytes);
    _json = buffer;
  }
  return _json;
}
