#include "AsyncCameraResponses.h"

void sendBMP(AsyncWebServerRequest *request){
    camera_fb_t * fb = esp_camera_fb_get();
    if (fb == NULL) {
        log_e("Camera frame failed");
        request->send(501);
        return;
    }

    uint8_t * buf = NULL;
    size_t buf_len = 0;
    unsigned long st = millis();
    bool converted = frame2bmp(fb, &buf, &buf_len);
    log_i("BMP: %lums, %uB", millis() - st, buf_len);
    esp_camera_fb_return(fb);
    if(!converted){
        request->send(501);
        return;
    }

    AsyncBufferResponse * response = new AsyncBufferResponse(buf, buf_len, BMP_CONTENT_TYPE);
    if (response == NULL) {
        log_e("Response alloc failed");
        request->send(501);
        return;
    }
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

void sendJpg(AsyncWebServerRequest *request){
    camera_fb_t * fb = esp_camera_fb_get();
    if (fb == NULL) {
        log_e("Camera frame failed");
        request->send(501);
        return;
    }

    if(fb->format == PIXFORMAT_JPEG){
        AsyncFrameResponse * response = new AsyncFrameResponse(fb, JPG_CONTENT_TYPE);
        if (response == NULL) {
            log_e("Response alloc failed");
            request->send(501);
            return;
        }
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
        return;
    }

    size_t jpg_buf_len = 0;
    uint8_t * jpg_buf = NULL;
    unsigned long st = millis();
    bool jpeg_converted = frame2jpg(fb, 80, &jpg_buf, &jpg_buf_len);
    esp_camera_fb_return(fb);
    if(!jpeg_converted){
        log_e("JPEG compression failed: %lu", millis());
        request->send(501);
        return;
    }
    log_i("JPEG: %lums, %uB", millis() - st, jpg_buf_len);

    AsyncBufferResponse * response = new AsyncBufferResponse(jpg_buf, jpg_buf_len, JPG_CONTENT_TYPE);
    if (response == NULL) {
        log_e("Response alloc failed");
        request->send(501);
        return;
    }
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}


void streamJpg(AsyncWebServerRequest *request){
    AsyncJpegStreamResponse *response = new AsyncJpegStreamResponse();
    if(!response){
        request->send(501);
        return;
    }
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);

    // Set the desired frame rate
    const int frameRate = 3; // 3 frames per second
    const int frameDelay = 1000 / frameRate; // Delay in milliseconds
}

void getCameraStatus(AsyncWebServerRequest *request){
    static char json_response[1024];

    sensor_t * s = esp_camera_sensor_get();
    if(s == NULL){
        request->send(501);
        return;
    }
    char * p = json_response;
    *p++ = '{';

    p+=sprintf(p, "\"framesize\":%u,", s->status.framesize);
    p+=sprintf(p, "\"quality\":%u,", s->status.quality);
    p+=sprintf(p, "\"brightness\":%d,", s->status.brightness);
    p+=sprintf(p, "\"contrast\":%d,", s->status.contrast);
    p+=sprintf(p, "\"saturation\":%d,", s->status.saturation);
    p+=sprintf(p, "\"sharpness\":%d,", s->status.sharpness);
    p+=sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
    p+=sprintf(p, "\"wb_mode\":%u,", s->status.wb_mode);
    p+=sprintf(p, "\"awb\":%u,", s->status.awb);
    p+=sprintf(p, "\"awb_gain\":%u,", s->status.awb_gain);
    p+=sprintf(p, "\"aec\":%u,", s->status.aec);
    p+=sprintf(p, "\"aec2\":%u,", s->status.aec2);
    p+=sprintf(p, "\"denoise\":%u,", s->status.denoise);
    p+=sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
    p+=sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
    p+=sprintf(p, "\"agc\":%u,", s->status.agc);
    p+=sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
    p+=sprintf(p, "\"gainceiling\":%u,", s->status.gainceiling);
    p+=sprintf(p, "\"bpc\":%u,", s->status.bpc);
    p+=sprintf(p, "\"wpc\":%u,", s->status.wpc);
    p+=sprintf(p, "\"raw_gma\":%u,", s->status.raw_gma);
    p+=sprintf(p, "\"lenc\":%u,", s->status.lenc);
    p+=sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
    p+=sprintf(p, "\"vflip\":%u,", s->status.vflip);
    p+=sprintf(p, "\"dcw\":%u,", s->status.dcw);
    p+=sprintf(p, "\"colorbar\":%u", s->status.colorbar);
    *p++ = '}';
    *p++ = 0;

    AsyncWebServerResponse * response = request->beginResponse(200, "application/json", json_response);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

void setCameraVar(AsyncWebServerRequest *request){
    if(!request->hasArg("var") || !request->hasArg("val")){
        request->send(404);
        return;
    }
    String var = request->arg("var");
    const char * variable = var.c_str();
    int val = atoi(request->arg("val").c_str());

    sensor_t * s = esp_camera_sensor_get();
    if(s == NULL){
        request->send(501);
        return;
    }


    int res = 0;
    if(!strcmp(variable, "framesize")) res = s->set_framesize(s, (framesize_t)val);
    else if(!strcmp(variable, "quality")) res = s->set_quality(s, val);
    else if(!strcmp(variable, "contrast")) res = s->set_contrast(s, val);
    else if(!strcmp(variable, "brightness")) res = s->set_brightness(s, val);
    else if(!strcmp(variable, "saturation")) res = s->set_saturation(s, val);
    else if(!strcmp(variable, "sharpness")) res = s->set_sharpness(s, val);
    else if(!strcmp(variable, "gainceiling")) res = s->set_gainceiling(s, (gainceiling_t)val);
    else if(!strcmp(variable, "colorbar")) res = s->set_colorbar(s, val);
    else if(!strcmp(variable, "awb")) res = s->set_whitebal(s, val);
    else if(!strcmp(variable, "agc")) res = s->set_gain_ctrl(s, val);
    else if(!strcmp(variable, "aec")) res = s->set_exposure_ctrl(s, val);
    else if(!strcmp(variable, "hmirror")) res = s->set_hmirror(s, val);
    else if(!strcmp(variable, "vflip")) res = s->set_vflip(s, val);
    else if(!strcmp(variable, "awb_gain")) res = s->set_awb_gain(s, val);
    else if(!strcmp(variable, "agc_gain")) res = s->set_agc_gain(s, val);
    else if(!strcmp(variable, "aec_value")) res = s->set_aec_value(s, val);
    else if(!strcmp(variable, "aec2")) res = s->set_aec2(s, val);
    else if(!strcmp(variable, "denoise")) res = s->set_denoise(s, val);
    else if(!strcmp(variable, "dcw")) res = s->set_dcw(s, val);
    else if(!strcmp(variable, "bpc")) res = s->set_bpc(s, val);
    else if(!strcmp(variable, "wpc")) res = s->set_wpc(s, val);
    else if(!strcmp(variable, "raw_gma")) res = s->set_raw_gma(s, val);
    else if(!strcmp(variable, "lenc")) res = s->set_lenc(s, val);
    else if(!strcmp(variable, "special_effect")) res = s->set_special_effect(s, val);
    else if(!strcmp(variable, "wb_mode")) res = s->set_wb_mode(s, val);
    else if(!strcmp(variable, "ae_level")) res = s->set_ae_level(s, val);

    else {
        log_e("unknown setting %s", var.c_str());
        request->send(404);
        return;
    }
    log_d("Got setting %s with value %d. Res: %d", var.c_str(), val, res);

    AsyncWebServerResponse * response = request->beginResponse(200);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}
