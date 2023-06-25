#pragma once
#include "esp_http_server.h"

class ICommandControl {
  public:
    virtual void begin() = 0;
    virtual void snapLayer() = 0;
    virtual void end() = 0;
    virtual void snap() = 0;
    virtual void ping() = 0;
    virtual void light(bool on) = 0;
};

/**
  Basic webserver that provides url-based invocation of commands.
  TODO Will serve files.
**/

class CamServer {
  public:
    CamServer(ICommandControl& commandControl);
    void setup();

  private:
    ICommandControl& _commandControl;
    char* jsonBuff;
    httpd_handle_t httpServer = NULL; // web server port 

    static esp_err_t indexHandler(httpd_req_t* req);
    static esp_err_t getSnapHandler(httpd_req_t* req);
    static esp_err_t makeSnapHandler(httpd_req_t* req);
    static esp_err_t beginHandler(httpd_req_t* req);
    static esp_err_t endHandler(httpd_req_t* req);
    static esp_err_t pingHandler(httpd_req_t* req);
};