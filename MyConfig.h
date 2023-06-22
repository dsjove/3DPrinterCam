#if defined(CONFIG_IDF_TARGET_ESP32)
// default pin configuration for ESP32 cam boards
#define CAMERA_MODEL_AI_THINKER // Has PSRAM  
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
// default pin configuration below for Freenove ESP32S3 cam boards
#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
#endif

/**************************************************************************
 Uncomment one only of the camera models below if not using a default above
 and comment out above define block
 Selecting wrong model may crash your device due to pin conflict
***************************************************************************/

//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
//#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD

#define INCLUDE_SD
#define STORAGE SD_MMC // one of: SPIFFS LittleFS SD_MMC 
