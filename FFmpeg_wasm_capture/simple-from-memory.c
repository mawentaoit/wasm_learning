#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "process.h"

int read_packet(void *opaque, uint8_t *buf, int buf_size);

typedef struct {
    uint8_t *ptr;
    size_t size;
} buffer_data;

buffer_data bufferData;

ImageData *setFile(uint8_t *buff, const int buffLength) {
    unsigned char *avio_ctx_buffer = NULL;
    size_t avio_ctx_buffer_size = buffLength;
    
    bufferData.ptr = buff;
    bufferData.size = buffLength;

    AVFormatContext *pFormatCtx = avformat_alloc_context();

    uint8_t *avioCtxBuffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
    
    /* 读内存数据 */
    AVIOContext *avioCtx = avio_alloc_context(avioCtxBuffer, avio_ctx_buffer_size, 0, NULL, read_packet, NULL, NULL);

    pFormatCtx->pb = avioCtx;
    pFormatCtx->flags = AVFMT_FLAG_CUSTOM_IO;

    /* 打开内存缓存文件, and allocate format context */
    if (avformat_open_input(&pFormatCtx, "", NULL, NULL) < 0) {
        fprintf(stderr, "Could not open input\n");
        return NULL;
    }
    av_dump_format(pFormatCtx, 0, "", 0);
    ImageData *result = process(pFormatCtx);
    // 清掉内存
    avformat_close_input(&pFormatCtx);
    av_free(avioCtx->buffer);
    av_free(avioCtx);
    av_free(avioCtxBuffer);
    return result;
}

int read_packet(void *opaque, uint8_t *buf, int buf_size) {
    buf_size = FFMIN(buf_size, bufferData.size);

    if (!buf_size)
        return AVERROR_EOF;
    // printf("ptr:%p size:%zu bz%zu\n", bufferData.ptr, bufferData.size, buf_size);

    /* copy internal buffer data to buf */
    memcpy(buf, bufferData.ptr, buf_size);
    bufferData.ptr += buf_size;
    bufferData.size -= buf_size;

    return buf_size;
}

typedef struct {
    uint8_t *buffer;
    size_t length;
} DataBuffer;

DataBuffer readFile(char* name) {
    FILE *file;
    uint8_t *buffer;
    unsigned long fileLen;
    
    file = fopen(name, "rb");
    if (!file) {
        fprintf(stderr, "unable to open file %s", name);
    }

    //Get file length
    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (uint8_t *)malloc(fileLen + 1);
    if (!buffer) {
        fprintf(stderr, "Memory error!");
        fclose(file);
    }

    fread(buffer, fileLen, 1, file);
    fclose(file);

    DataBuffer databuffer;
    databuffer.length = fileLen;
    databuffer.buffer = buffer;
    return databuffer;
}

int main(int argc, char *argv[]) {
    DataBuffer databuffer = readFile(argv[1]);
    av_register_all();
    ImageData* data = setFile(databuffer.buffer, databuffer.length);
    printf("data->width = %d\n", data->width);
    return 0;
}