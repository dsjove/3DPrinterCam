#include "AppHardware.h"
#include "Hardware.h"
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

String AppHardware::toJson() const {
  String result;
  result.reserve(256);
  result.concat(appName);
  result.concat(':');
  result.concat(appVersion);
  result.concat(' ');
  result.concat(cameraBoard);
  result.concat(':');
  result.concat(cameraModel);
  result.concat(' ');
  result.concat(storageType);
  result.concat(':');
  result.concat(physicalSize);
  result.concat(':');
  result.concat(totalBytes);
  return result;
}
