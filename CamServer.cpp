#include "CamServer.h"
#include "AppHardware.h"
#include "ICommandControl.h"
#include "Hardware.h"
#include <HardwareSerial.h>

CamServer::CamServer(AppHardware& hardware, ICommandControl& commandControl)
: _hardware(hardware)
, _commandControl(commandControl) {
}

esp_err_t CamServer::indexHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
  httpd_resp_send(req, ths._hardware.toString().c_str(), HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t CamServer::signalHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  ICommandControl::Command cmd;
  cmd.code = ICommandControl::Signal;
  ths._commandControl.onCommand(cmd);
  httpd_resp_send(req, "", 0);
  return ESP_OK;
}

esp_err_t CamServer::beginHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  ICommandControl::Command cmd;
  cmd.code = ICommandControl::Begin;
  ths._commandControl.onCommand(cmd);
  httpd_resp_send(req, "", 0);
  return ESP_OK;
}

esp_err_t CamServer::frameHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  ICommandControl::Command cmd;
  cmd.code = ICommandControl::Frame;
  ths._commandControl.onCommand(cmd);
  httpd_resp_send(req, "", 0);
  return ESP_OK;
}

esp_err_t CamServer::endHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  ICommandControl::Command cmd;
  cmd.code = ICommandControl::End;
  ths._commandControl.onCommand(cmd);
  httpd_resp_send(req, "", 0);
  return ESP_OK;
}

esp_err_t CamServer::savePhotoHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  ICommandControl::Command cmd;
  cmd.code = ICommandControl::SavePhoto;
  ths._commandControl.onCommand(cmd);
  return photoHandler(req);
}

esp_err_t CamServer::photoHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  if (!ths._lastfb)
  {
      httpd_resp_send_500(req);
      return ESP_FAIL;
  }
  httpd_resp_set_type(req, "image/jpeg");
  //TODO: lower image quality for this preview
  esp_err_t res = httpd_resp_send(req, (const char *)ths._lastfb->buf, ths._lastfb->len);
  return res;
}

void CamServer::setup() {
  if (WEB_PORT <= 0) return;
  Serial.println("CamServer Starting...");
   httpd_config_t config = HTTPD_DEFAULT_CONFIG();
   config.task_priority = tskIDLE_PRIORITY+5;
#if CONFIG_IDF_TARGET_ESP32S3
  config.stack_size = 1024 * 8;
#endif  
  config.server_port = WEB_PORT;
  config.ctrl_port = WEB_PORT; 
  config.lru_purge_enable = true;
  httpd_uri_t indexUri = {.uri = "/", .method = HTTP_GET, .handler = indexHandler, .user_ctx = this};
  httpd_uri_t beginUri = {.uri = "/begin", .method = HTTP_GET, .handler = beginHandler, .user_ctx = this};
  httpd_uri_t frameUri = {.uri = "/frame", .method = HTTP_GET, .handler = frameHandler, .user_ctx = this};
  httpd_uri_t endUri = {.uri = "/end", .method = HTTP_GET, .handler = endHandler, .user_ctx = this};
  httpd_uri_t signalUri = {.uri = "/signal", .method = HTTP_GET, .handler = signalHandler, .user_ctx = this};
  httpd_uri_t savePhotoUri = {.uri = "/photo", .method = HTTP_PUT, .handler = savePhotoHandler, .user_ctx = this};
  httpd_uri_t photoUri = {.uri = "/photo", .method = HTTP_GET, .handler = photoHandler, .user_ctx = this};
  //TODO: live stream frames as they come in
  //TODO: Serve file listing and files
 
  // allowing too many concurrent web clients can cause errors
  config.max_open_sockets = 2; 
  
  if (httpd_start(&httpServer, &config) == ESP_OK) {
    httpd_register_uri_handler(httpServer, &indexUri);
    httpd_register_uri_handler(httpServer, &signalUri);
    httpd_register_uri_handler(httpServer, &beginUri);
    httpd_register_uri_handler(httpServer, &frameUri);
    httpd_register_uri_handler(httpServer, &endUri);
    httpd_register_uri_handler(httpServer, &photoUri);
    httpd_register_uri_handler(httpServer, &savePhotoUri);
  }
}

void CamServer::liveStream(camera_fb_t* fb) {
  if (fb == NULL) return;
  _lastfb = fb;
}