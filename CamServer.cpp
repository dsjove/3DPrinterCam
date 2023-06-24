#include "CamServer.h"
#include "Globals.h"

#include <HardwareSerial.h>
#define LOG_INF Serial.printf
#define LOG_ERR LOG_INF
#define LOG_WRN LOG_INF

CamServer::CamServer() {

}

static esp_err_t indexHandler(httpd_req_t* req) {
}

static esp_err_t getSnapHandler(httpd_req_t* req) {
}

static esp_err_t makeSnapHandler(httpd_req_t* req) {
}

static esp_err_t beginHandler(httpd_req_t* req) {
}

static esp_err_t endHandler(httpd_req_t* req) {
}

static esp_err_t pingHandler(httpd_req_t* req) {
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
  httpd_uri_t makeSnapUri = {.uri = "/snap", .method = HTTP_PUT, .handler = makeSnapHandler, .user_ctx = this};
  httpd_uri_t beginUri = {.uri = "/begin", .method = HTTP_GET, .handler = beginHandler, .user_ctx = this};
  httpd_uri_t endUri = {.uri = "/end", .method = HTTP_GET, .handler = endHandler, .user_ctx = this};
  httpd_uri_t pingUri = {.uri = "/ping", .method = HTTP_GET, .handler = pingHandler, .user_ctx = this};

  config.max_open_sockets = MAX_CLIENTS; 
  if (httpd_start(&httpServer, &config) == ESP_OK) {
    httpd_register_uri_handler(httpServer, &indexUri);
    httpd_register_uri_handler(httpServer, &getSnapUri);
    httpd_register_uri_handler(httpServer, &makeSnapUri);
    httpd_register_uri_handler(httpServer, &beginUri);
    httpd_register_uri_handler(httpServer, &endUri);
    httpd_register_uri_handler(httpServer, &pingUri);
    LOG_INF("Starting web server on port: %u", config.server_port);
  } 
  else {
    LOG_ERR("Failed to start web server");
  }
}