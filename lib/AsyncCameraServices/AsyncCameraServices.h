#ifndef ASYNC_CAMERA_SERVICES_H
#define ASYNC_CAMERA_SERVICES_H

#include <ESPAsyncWebServer.h>
#include <esp_camera.h>

// Function declarations
void sendBMP(AsyncWebServerRequest *request);
void sendJpg(AsyncWebServerRequest *request);
void streamJpg(AsyncWebServerRequest *request);
void getCameraStatus(AsyncWebServerRequest *request);
void setCameraVar(AsyncWebServerRequest *request);

#endif // CAMERA_SERVER_H
