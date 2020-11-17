#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libswscale/swscale.h>
#include <stdio.h>

void SafeFrame(AVFrame* pFrame, int width, int height) {
    FILE* pFile;
    char szFilename[32];
    int y;
    
    sprintf(szFilename, "frame.ppm");
    pFile = fopen(szFilename, "wb");

    if(pFile == NULL) {
        return;
    }

    //写入头信息
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    //写入数据
    for (y = 0; y < height; y++) {
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width*3, pFile);
    }

    fclose(pFile);
}

int main(int argc, char const *argv[]) {
    av_register_all();
    AVFormatContext* pFormatCtx = NULL;

    if (avformat_open_input(&pFormat, argv[1], NULL, NULL) != 0) {
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        return -2;
    }

    av_dump_format(pFormatCtx, 0, argv[1], 0);
    AVCodecParameters *codecPar = NULL;
    int videoStream = -1;

    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codepar->codec_type == AVMEDIA_TYPE_VIDEO) {
            codecPar = pFormatCtx->stream[i]->codecpar;
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1) {
        return -1;
    }

    AVCodec *pCodec = NULL;
    pCodec = avcodec_find_decoder(codecPar->codec_id);
    if (pCodec == NULL) {
        fprintf(stderr, "Unsupported codec!");
        return -1;
    }

    AVCodecContext *pCodecCtx = NULL;
    pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, codecPar);

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        return -1;
    }

    AVFrame *pFrame = NULL;
    // Allocate video frame
    pFrame = av_frame_alloc();

    AVFrame *pFrameRGB = av_frame_alloc();
    if (pFrameRGB == NULL) {
        return -1;
    }

    unsigned char *buffer = NULL;
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);

    buffer = (unsigned char*)av_malloc(numBytes * sizeof(unsigned char));

    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
    
    struct SwsContext *sws_ctx = NULL;
    AVPacket packet;

    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);    

    int ms = 10*1000;
    int timeStamp = ((double)ms / (double)1000) * pFormatCtx->streams[videoStream]->time_base.den / pFormatCtx->streams[videoStream]->time_base.num;

    printf("timestamp: %d \n", timeStamp);
    printf("num: %d \n", pFormatCtx->streams[videoStream]->time_base.num);
    printf("den: %d \n", pFormatCtx->streams[videoStream]->time_base.den);

    int code = av_seek_frame(pFormatCtx, videoStream, timeStamp, AVSEEK_FLAG_BACKWARD);

    if (code < 0) {
        fprintf(stderr, "av_seek_frame failed");
        return -1;
    }

    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        if (packet.stream_index == videoStream) {
            if (avcodec_send_packet(pCodecCtx, &packet) != 0) {
                fprintf(stderr, "there is something wrong with avcodec_send_packet\n");
                av_packet_unref(&packet);
                continue;
            }

            if (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                sws_scale(sws_ctx, (uint8_t const *const *)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                SavaFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height);

                av_packet_unref(&packet);
                break;
            }
        }

        av_packet_unref(&packet);
    }

    av_free(buffer);
    av_frame_unref(pFrameRGB);
    av_frame_unref(pFrame);
    av_free(pFrame);
    avcodec_free_context(&pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}