#include "FFMPEGVideoWriter.h"
#ifdef __cplusplus
extern "C" {
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
}
#endif

FFMPEGVideoWriter::FFMPEGVideoWriter()
    : codec(NULL)
    , m_formatContext(NULL)
    , m_initialized(false)
    , m_frame_count(1)
{}

void FFMPEGVideoWriter::setup(const char* filename, int width, int height, int frameCountPerSec) {
    printf("Video encoding: %s\n",filename);
    /* register all the formats and codecs */
    av_register_all();

    /* allocate the output media context */
    avformat_alloc_output_context2(&m_formatContext, NULL, NULL, filename);
    if (!m_formatContext) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&m_formatContext, NULL, "mp4", filename);
    }
    if (!m_formatContext) {
        fprintf(stderr, "could not create AVFormat context\n");
        exit(1);
    }
    fmt = m_formatContext->oformat;

    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */
    m_video_st = NULL;
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        /* find the video encoder */
        AVCodecID avcid = fmt->video_codec;
        codec = avcodec_find_encoder(avcid);
        if (!codec) {
            fprintf(stderr, "codec not found: %s\n", avcodec_get_name(avcid));
            exit(1);
        } else {
            const AVPixelFormat* p = codec->pix_fmts;
            while (*p != AV_PIX_FMT_NONE) {
                printf("supported pix fmt: %s\n",av_get_pix_fmt_name(*p));
                ++p;
            }
        }

        m_video_st = avformat_new_stream(m_formatContext, codec);
        if (!m_video_st) {
            fprintf(stderr, "Could not allocate stream\n");
            exit(1);
        }
        m_video_st->id = m_formatContext->nb_streams-1;
        m_codecContext = m_video_st->codec;
    }

    /* Now that all the parameters are set, we can open the audio and
     * video codecs and allocate the necessary encode buffers. */
    {
        m_picture = av_frame_alloc();
        m_picture->width = width;
        m_picture->height = height;
        m_picture->pts = 0;

        /* put sample parameters */
        m_codecContext->bit_rate = 400000;
        /* resolution must be a multiple of two */
        m_codecContext->width = width;
        m_codecContext->height = height;

        /* frames per second */
        m_codecContext->time_base.num = 1;
        m_codecContext->time_base.den = frameCountPerSec;
        m_codecContext->gop_size = 10; /* emit one intra frame every ten frames */
        //m_codecContext->max_b_frames=1;
        m_codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

        /* open it */
        AVDictionary* options = NULL;
        if (avcodec_open2(m_codecContext, codec, &options) < 0) {
            fprintf(stderr, "could not open codec\n");
            exit(1);
        }
        else {
            printf("opened %s\n", avcodec_get_name(fmt->video_codec));
        }

        /* alloc image and output buffer */
        m_picture->data[0] = NULL;
        m_picture->linesize[0] = -1;
        m_picture->format = m_codecContext->pix_fmt;

        int ret = -1;
        ret = av_image_alloc(m_picture->data, m_picture->linesize, m_codecContext->width, m_codecContext->height, (AVPixelFormat)m_picture->format, 32);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate raw picture buffer\n");
            exit(1);
        } else {
            printf("allocated picture of size %d (ptr %x), linesize %d %d %d %d\n",ret,m_picture->data[0],m_picture->linesize[0],m_picture->linesize[1],m_picture->linesize[2],m_picture->linesize[3]);
        }

        m_picture_rgb24 = av_frame_alloc();
        m_picture_rgb24->format = AV_PIX_FMT_RGB24;
        if((ret = av_image_alloc(m_picture_rgb24->data, m_picture_rgb24->linesize, m_codecContext->width, m_codecContext->height, (AVPixelFormat)m_picture_rgb24->format, 24)) < 0) {
            fprintf(stderr,"cannot allocate RGB temp image\n");
            exit(1);
        } else
            printf("allocated picture of size %d (ptr %x), linesize %d %d %d %d\n",ret,m_picture_rgb24->data[0],m_picture_rgb24->linesize[0],m_picture_rgb24->linesize[1],m_picture_rgb24->linesize[2],m_picture_rgb24->linesize[3]);
        m_size = ret;
    }

    av_dump_format(m_formatContext, 0, filename, 1);
    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        int ret;
        if ((ret = avio_open(&m_formatContext->pb, filename, AVIO_FLAG_WRITE)) < 0) {
            //fprintf(stderr, "Could not open '%s': %s\n", filename, av_err2str(ret));
            std::cerr << "Could not open: " << filename << std::endl;
            exit(1);
        }
    }
    /* Write the stream header, if any. */
    int ret = avformat_write_header(m_formatContext, NULL);
    if (ret < 0) {
        //fprintf(stderr, "Error occurred when opening output file: %s\n", av_err2str(ret));
        std::cerr << "CError occurred when opening output file:: " << ret << std::endl;
        exit(1);
    }

    /* get sws context for RGB24 -> YUV420 conversion */
    sws_ctx = sws_getContext(m_codecContext->width, m_codecContext->height, (AVPixelFormat)m_picture_rgb24->format,
                             m_codecContext->width, m_codecContext->height, (AVPixelFormat)m_picture->format,
                             SWS_BICUBIC, NULL, NULL, NULL);
    if (!sws_ctx) {
        fprintf(stderr,
                "Could not initialize the conversion context\n");
        exit(1);
    }
    m_initialized = true;
}

/* add a frame to the video file, RGB 24bpp format */
void FFMPEGVideoWriter::addFrame(const uint8_t* pixels) {
    /* copy the buffer */
    memcpy(m_picture_rgb24->data[0], pixels, m_size);

    /* convert RGB24 to YUV420 */
    sws_scale(sws_ctx, m_picture_rgb24->data, m_picture_rgb24->linesize, 0, m_codecContext->height, m_picture->data, m_picture->linesize);
    AVPacket pkt = { 0 };
    int got_packet;
    av_init_packet(&pkt);

    /* encode the image */
    int ret = avcodec_encode_video2(m_codecContext, &pkt, m_picture, &got_packet);
    if (ret < 0) {
        std::cerr << "Error encoding video frame: " << ret << std::endl;
        exit(1);
    }

    /* If size is zero, it means the image was buffered. */
    if (!ret && got_packet && pkt.size) {
        pkt.stream_index = m_video_st->index;
        /* Write the compressed frame to the media file. */
        ret = av_interleaved_write_frame(m_formatContext, &pkt);
    } else {
        ret = 0;
    }
    m_picture->pts += av_rescale_q(1, m_video_st->codec->time_base, m_video_st->time_base);
    m_frame_count++;
}


void FFMPEGVideoWriter::close() {
    /* Write the trailer, if any. The trailer must be written before you
     * close the CodecContexts open when you wrote the header; otherwise
     * av_write_trailer() may try to use memory that was freed on
     * av_codec_close(). */

    AVPacket pkt = { 0 };
    int got_output = 1;
    av_init_packet(&pkt);
    while(got_output) {
        int ret = avcodec_encode_video2(m_codecContext, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            break;
        }
        if (got_output) {
            ret = av_interleaved_write_frame(m_formatContext, &pkt);
            av_free_packet(&pkt);
        }
    }

    av_write_trailer(m_formatContext);

    /* Close each codec. */

    avcodec_close(m_video_st->codec);
    av_freep(&(m_picture->data[0]));
    av_free(m_picture);

    if (!(fmt->flags & AVFMT_NOFILE))
    /* Close the output file. */
        avio_close(m_formatContext->pb);

    /* free the stream */
    avformat_free_context(m_formatContext);

    printf("closed video file\n");

    m_initialized = false;
    m_frame_count = 0;
}

