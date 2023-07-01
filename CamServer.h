#pragma once
#include "esp_http_server.h"
#include "esp_camera.h"

class ICommandControl;
class AppHardware;

/**
  Basic webserver that provides url-based invocation of commands.
  TODO: Will serve files.
**/

class CamServer {
  public:
    CamServer(AppHardware& hardware, ICommandControl& commandControl);
    void setup();
    void liveStream(camera_fb_t* fb);

  private:
    AppHardware& _hardware;
    ICommandControl& _commandControl;
    httpd_handle_t httpServer = NULL; // web server port 
    camera_fb_t* _lastfb = NULL;

    static esp_err_t indexHandler(httpd_req_t* req);
    static esp_err_t signalHandler(httpd_req_t* req);
    static esp_err_t beginHandler(httpd_req_t* req);
    static esp_err_t frameHandler(httpd_req_t* req);
    static esp_err_t endHandler(httpd_req_t* req);
    static esp_err_t savePhotoHandler(httpd_req_t* req);
    static esp_err_t photoHandler(httpd_req_t* req);
};