#pragma once
#include "esp_http_server.h"
#include "esp_camera.h"

class ICommandControl;

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
    static esp_err_t signalHandler(httpd_req_t* req);
    static esp_err_t beginHandler(httpd_req_t* req);
    static esp_err_t frameHandler(httpd_req_t* req);
    static esp_err_t endHandler(httpd_req_t* req);
    static esp_err_t makePhotoHandler(httpd_req_t* req);
    static esp_err_t photoHandler(httpd_req_t* req);
};