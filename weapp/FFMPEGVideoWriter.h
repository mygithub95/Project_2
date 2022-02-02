#ifndef FFMPEGVIDEOWRITER_H
#define FFMPEGVIDEOWRITER_H

#include <stdio.h>
#include <iostream>
#ifdef __cplusplus
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#endif

class FFMPEGVideoWriter
{
    //instance variables
    AVCodec *codec;
    AVFrame *m_picture;
    AVOutputFormat *fmt;
    AVFormatContext *m_formatContext;
    AVStream *m_video_st;
    AVCodecContext* m_codecContext;

    struct SwsContext *sws_ctx;
    bool m_initialized;
    int m_frame_count;

public:
    FFMPEGVideoWriter();

    /**
     * setup the video writer
     * @param output filename, the codec and format will be determined by it. (e.g. "xxx.mpg" will create an MPEG1 file
     * @param width of the frame
     * @param height of the frame
     **/
    void setup(const char* filename, int width, int height, int frameCountPerSec);
    /**
     * add a frame to the video file
     * @param the pixels packed in RGB (24-bit RGBRGBRGB...)
     **/
    void addFrame(const uint8_t* pixels);
    /**
     * close the video file and release all datastructs
     **/
    void close();
    /**
     * is the videowriter initialized?
     **/
    bool isInitialized() const { return m_initialized; }

public:
    int m_size;
    AVFrame *m_picture_rgb24;
};

#endif // FFMPEGVIDEOWRITER_H
