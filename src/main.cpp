#include "esp_camera.h"
#include <WiFi.h>
#include <AsyncCameraServices.h>
#include <camera_pins.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

// Replace with your network credentials
const char* ssid = "ivorya 2";
const char* password = "wildflower17";

// Camera configuration
camera_config_t config;
AsyncWebServer server(80); // Create an AsyncWebServer on port 80

void startHttpServer();

void initWifi(const char * ssid, const char * password){
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void initCamera() {
  // Populate camera config structure with hardware and other defaults
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = XCLK_FREQ * 1000000;
  config.pixel_format = PIXFORMAT_JPEG;
  // Low(ish) default framesize and quality
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.fb_count = 2;
  config.grab_mode = CAMERA_GRAB_LATEST;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
      delay(100);  // need a delay here or the next serial o/p gets missed
      Serial.printf("\r\n\r\nCRITICAL FAILURE: Camera sensor failed to initialise.\r\n\r\n");
      Serial.printf("A full (hard, power off/on) reboot will probably be needed to recover from this.\r\n");
      Serial.printf("Meanwhile; this unit will reboot in 1 minute since these errors sometime clear automatically\r\n");
      // Reset the I2C bus.. may help when rebooting.
      periph_module_disable(PERIPH_I2C0_MODULE); // try to shut I2C down properly in case that is the problem
      periph_module_disable(PERIPH_I2C1_MODULE);
      periph_module_reset(PERIPH_I2C0_MODULE);
      periph_module_reset(PERIPH_I2C1_MODULE);

      // Start a 60 second watchdog timer
      esp_task_wdt_init(60,true);
      esp_task_wdt_add(NULL);
  } else {
      Serial.println("Camera init succeeded");

      // Get a reference to the sensor
      sensor_t * s = esp_camera_sensor_get();
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize the camera
  initCamera();

  // Connect to Wi-Fi
  initWifi(ssid, password);
  
  // Start the camera server
  startHttpServer();
}

void startHttpServer() {
    server.on("/bmp", HTTP_GET, sendBMP);
    server.on("/capture", HTTP_GET, sendJpg);
    server.on("/stream", HTTP_GET, streamJpg);
    server.on("/control", HTTP_GET, setCameraVar);
    server.on("/status", HTTP_GET, getCameraStatus);
    server.begin();
}

void loop() {

}
