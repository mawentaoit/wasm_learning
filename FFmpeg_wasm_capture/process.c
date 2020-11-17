#include "process.h"
#include <stdio.h>
#include <string.h>

ImageData *imageData = NULL;

ImageData *process (AVFormatContext *pFormatCtx, int timeStamp) {
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        return NULL;
    }

    //找到视频流
int videoStream = findVideoStream(pFormatCtx);
   AVCodecContext *pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // 打开相应的解码器
    AVCodecContext *pNewCodecCtx = openCodec(pCodecCtx);
    if (!pNewCodecCtx) {
        fprintf(stderr, "openCodec failed, pNewCodecCtx is NULL\n");
        return NULL;
    }

    uint8_t *frameBuffer;
    AVFrame *pFrameRGB = initAVFrame(pNewCodecCtx, &frameBuffer);

}

AVFrame *readAVFrame (AVCodecContext *pCodecCtx, AVFormatContext *pFormatCtx,
    AVFrame *pFrameRGB, int videoStream, int timeStamp){
                struct SwsContext *sws_ctx = NULL;
                int frameFinished;
                AVPacket packet;
                AVFrame *pFrame = NULL;
                pFrame = av_frame_alloc();

                // initialize SWS context for software scaling
                sws_ctx = sws_getContext(pCodecCtx->width,
                    pCodecCtx->height,
                    pCodecCtx->pix_fmt,
                    pCodecCtx->width,
                    pCodecCtx->height,
                    AV_PIX_FMT_RGB24,
                    SWS_BILINEAR,
                    NULL,
                    NULL,
                    NULL
                    );
                int i = 0;
                int ret = av_seek_frame(pFormatCtx, videoStream, ((double)timeStamp/(double)33492.5)*AV_TIME_BASE
                            + (double)pFormatCtx->start_time, AVSEEK_FLAG_BACKWARD);

                if (ret < 0) {
                    fprintf(stderr, "av_seek_frame failed");
                    return NULL;
                }

                while (av_read_frame(pFormatCtx, &packet) >= 0) {
                    if (packet.stream_index == videoStream) {
                        ret = avcodec_send_packet(pCodecCtx, &packet);
                        av_free_packet(&packet);
                        if(ret < 0) {
                            fprintf(stderr, "avcodec_send_packet");
                            return NULL;
                        }
                        while(ret >= 0) {
                            ret = avcodec_receive_frame(pCodecCtx, pFrame);
                            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                                break;
                            else if (ret < 0) {
                                fprintf(stderr, "Error during decoding\n");
                                return NULL;
                            }
                        }

                        sws_scale(sws_ctx, (uint8_t const* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                            pFrameRGB->data, pFrameRGB->linesize);
                        
                        return pFrameRGB;
                    }
                }
                return NULL;
            }

AVFrame *initAVFrame (AVCodecContext *pCodecCtx, uint8_t **frameBuffer) {
    AVFrame *pFrameRGB = av_frame_alloc();
    if (pFrameRGB == NULL) {
        return NULL;
    }

    int numBytes;
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
                                pCodecCtx->height);
    *frameBuffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));

    avpicture_fill((AVPicture *)pFrameRGB, *frameBuffer, AV_PIX_FMT_RGB24,
                pCodecCtx->width, pCodecCtx->height);
    
    return pFrameRGB;
}

int findVideoStream (AVFormatContext *pFormatCtx) {
    // Find the first video stream
    int videoStream = -1;
    for (int i = 0; i < pFormatCtx -> nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    return videoStream;    
}

AVCodecContext* openCodec (AVCodecContext* pCodecCtx) {
    AVCodec *pCodec = NULL;
    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        fprintf(stderr, "%s", "Unsupported codec!\n");
        return NULL; // Codec not found
    }
    // Copy context
    AVCodecContext *pNewCodecCtx = avcodec_alloc_context3(pCodec);
    if (avcodec_copy_context(pNewCodecCtx, pCodecCtx) != 0) {
        fprintf(stderr, "Couldn't copy codec context");
        return NULL; // Error copying codec context
    }
    // Open codec
    if (avcodec_open2(pNewCodecCtx, pCodec, NULL) < 0) {
        return NULL; // Could not open codec
    }
    return pNewCodecCtx;
}