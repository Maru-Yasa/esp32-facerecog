#ifndef ASYNC_CAMERA_RESPONSES_H
#define ASYNC_CAMERA_RESPONSES_H

#include <ESPAsyncWebServer.h>
#include <esp_camera.h>

// Type definition for camera frame
typedef struct {
    camera_fb_t * fb;
    size_t index;
} camera_frame_t;

// Define constants for streaming
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* STREAM_PART = "Content-Type: %s\r\nContent-Length: %u\r\n\r\n";

static const char * JPG_CONTENT_TYPE = "image/jpeg";
static const char * BMP_CONTENT_TYPE = "image/x-windows-bmp";

// Class declaration for AsyncBufferResponse
class AsyncBufferResponse : public AsyncAbstractResponse {
private:
    uint8_t * _buf;
    size_t _len;
    size_t _index;

public:
    AsyncBufferResponse(uint8_t * buf, size_t len, const char * contentType);
    ~AsyncBufferResponse();
    bool _sourceValid() const;
    virtual size_t _fillBuffer(uint8_t *buf, size_t maxLen) override;
    size_t _content(uint8_t *buffer, size_t maxLen, size_t index);
};

// Class declaration for AsyncFrameResponse
class AsyncFrameResponse : public AsyncAbstractResponse {
private:
    camera_fb_t * fb;
    size_t _index;

public:
    AsyncFrameResponse(camera_fb_t * frame, const char * contentType);
    ~AsyncFrameResponse();
    bool _sourceValid() const;
    virtual size_t _fillBuffer(uint8_t *buf, size_t maxLen) override;
    size_t _content(uint8_t *buffer, size_t maxLen, size_t index);
};

// Class declaration for AsyncJpegStreamResponse
class AsyncJpegStreamResponse : public AsyncAbstractResponse {
private:
    camera_frame_t _frame;
    size_t _index;
    size_t _jpg_buf_len;
    uint8_t * _jpg_buf;
    uint64_t lastAsyncRequest;

public:
    AsyncJpegStreamResponse();
    ~AsyncJpegStreamResponse();
    bool _sourceValid() const;
    virtual size_t _fillBuffer(uint8_t *buf, size_t maxLen) override;
    size_t _content(uint8_t *buffer, size_t maxLen, size_t index);
};

#endif // ASYNC_CAMERA_RESPONSES_H
