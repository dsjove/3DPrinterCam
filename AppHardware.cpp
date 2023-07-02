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
  _cacheString.reserve(256);
}

const String& AppHardware::toString() {
  if (_cacheString.isEmpty() == false) {
    return _cacheString;
  }
  _cacheString.concat(appName);
  _cacheString.concat(':');
  _cacheString.concat(appVersion);
  _cacheString.concat(' ');
  _cacheString.concat(cameraBoard);
  _cacheString.concat(':');
  _cacheString.concat(cameraModel);
  _cacheString.concat(' ');
  _cacheString.concat(storageType);
  _cacheString.concat(':');
  _cacheString.concat(physicalSize / (1024 * 1024));
  _cacheString.concat(':');
  _cacheString.concat(totalBytes / (1024 * 1024));
  return _cacheString;
}
