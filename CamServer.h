#pragma once
#include "esp_http_server.h"
#include "esp_camera.h"

class ICommandControl {
  public:
    virtual void begin() = 0;
    virtual void snapLayer() = 0;
    virtual void end() = 0;
    virtual void snap() = 0;
    virtual void signal() = 0;
    virtual void light(bool on) = 0;
};

/**
  Basic webserver that provides url-based invocation of commands.
  TODO: Will serve files.
**/

class CamServer {
  public:
    CamServer(ICommandControl& commandControl);
    void setup();
    void liveStream(camera_fb_t* fb);

  private:
    ICommandControl& _commandControl;
    char* jsonBuff;
    httpd_handle_t httpServer = NULL; // web server port 
    camera_fb_t* _lastfb = NULL;

    static esp_err_t indexHandler(httpd_req_t* req);
    static esp_err_t getSnapHandler(httpd_req_t* req);
    static esp_err_t makeSnapHandler(httpd_req_t* req);
    static esp_err_t beginHandler(httpd_req_t* req);
    static esp_err_t endHandler(httpd_req_t* req);
    static esp_err_t signalHandler(httpd_req_t* req);
};