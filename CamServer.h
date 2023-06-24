#pragma once
#include "esp_http_server.h"

class CamServer {
  public:
    CamServer();
    void setup();

  private:
    httpd_handle_t httpServer = NULL; // web server port 
};