#include "CamServer.h"
#include "Globals.h"

#include <HardwareSerial.h>
#define LOG_INF Serial.printf
#define LOG_ERR LOG_INF
#define LOG_WRN LOG_INF

#define JSON_BUFF_LEN (32 * 1024) // set big enough to hold all file names in a folder

CamServer::CamServer(ICommandControl& commandControl)
: _commandControl(commandControl)
, jsonBuff(psramFound() ? (char*)ps_malloc(JSON_BUFF_LEN) : (char*)malloc(JSON_BUFF_LEN))
{
  ::memset(jsonBuff, 0, JSON_BUFF_LEN);
}

esp_err_t CamServer::indexHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  ths.jsonBuff[0] = '{';
  //TODO: App, Network, Runtime Stats
  ths.jsonBuff[1] = '}';
  ths.jsonBuff[2] = 0;
  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, ths.jsonBuff, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t CamServer::getSnapHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  if (!ths._lastfb)
  {
      log_e("Camera capture failed");
      httpd_resp_send_500(req);
      return ESP_FAIL;
  }
  uint8_t * buf = NULL;
  size_t buf_len = 0;
  bool converted = frame2bmp(ths._lastfb, &buf, &buf_len);
  if(!converted){
      httpd_resp_send_500(req);
      return ESP_FAIL;
  }
  httpd_resp_set_type(req, "image/x-windows-bmp");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.bmp");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  esp_err_t res = httpd_resp_send(req, (const char *)buf, buf_len);
  free(buf);
  return res;
}

esp_err_t CamServer::makeSnapHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  ths._commandControl.snap();
  httpd_resp_set_type(req, "application/json");
  ths.jsonBuff[0] = 0;
  httpd_resp_send(req, ths.jsonBuff, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t CamServer::beginHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  ths._commandControl.begin();
  httpd_resp_set_type(req, "application/json");
  ths.jsonBuff[0] = 0;
  httpd_resp_send(req, ths.jsonBuff, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t CamServer::endHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  ths._commandControl.end();
  httpd_resp_set_type(req, "application/json");
  ths.jsonBuff[0] = 0;
  httpd_resp_send(req, ths.jsonBuff, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t CamServer::signalHandler(httpd_req_t* req) {
  CamServer& ths = *(CamServer*)req->user_ctx;
  ths._commandControl.signal();
  httpd_resp_set_type(req, "application/json");
  ths.jsonBuff[0] = 0;
  httpd_resp_send(req, ths.jsonBuff, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

#define MAX_CLIENTS 2 // allowing too many concurrent web clients can cause errors

void CamServer::setup() {
   httpd_config_t config = HTTPD_DEFAULT_CONFIG();
#if CONFIG_IDF_TARGET_ESP32S3
  config.stack_size = 1024 * 8;
#endif  
  config.server_port = WEB_PORT;
  config.ctrl_port = WEB_PORT; 
  config.lru_purge_enable = true;
  httpd_uri_t indexUri = {.uri = "/", .method = HTTP_GET, .handler = indexHandler, .user_ctx = this};
  httpd_uri_t getSnapUri = {.uri = "/snap", .method = HTTP_GET, .handler = getSnapHandler, .user_ctx = this};
  httpd_uri_t makeSnapUri = {.uri = "/snapmake", .method = HTTP_PUT, .handler = makeSnapHandler, .user_ctx = this};
  httpd_uri_t beginUri = {.uri = "/begin", .method = HTTP_GET, .handler = beginHandler, .user_ctx = this};
  httpd_uri_t endUri = {.uri = "/end", .method = HTTP_GET, .handler = endHandler, .user_ctx = this};
  httpd_uri_t signalUri = {.uri = "/signal", .method = HTTP_GET, .handler = signalHandler, .user_ctx = this};

  config.max_open_sockets = MAX_CLIENTS; 
  if (httpd_start(&httpServer, &config) == ESP_OK) {
    httpd_register_uri_handler(httpServer, &indexUri);
    httpd_register_uri_handler(httpServer, &getSnapUri);
    httpd_register_uri_handler(httpServer, &makeSnapUri);
    httpd_register_uri_handler(httpServer, &beginUri);
    httpd_register_uri_handler(httpServer, &endUri);
    httpd_register_uri_handler(httpServer, &signalUri);
    LOG_INF("Starting web server on port: %u", config.server_port);
  } 
  else {
    LOG_ERR("Failed to start web server");
  }
}

void CamServer::liveStream(camera_fb_t* fb) {
  if (fb == NULL) return;
  _lastfb = fb;
}