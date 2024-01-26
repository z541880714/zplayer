//
// Created by lionel on 2023/11/7.
//


#include "decode_audio.h"

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

Callback callback__ = NULL;
ZAudioFormat zAudioFormat;

int get_format_from_fmt(const char **fmt, enum AVSampleFormat sample_fmt) {
    int i = 0, ret = -1;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt;
        const char *fmt_be;
        const char *fmt_le;
    } sample_fmt_entries[] = {
            {AV_SAMPLE_FMT_U8,  "u8",    "u8"},
            {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
            {AV_SAMPLE_FMT_S32, "s32be", "s32le"},
            {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
            {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
    };

    *fmt = NULL;
    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry entry = sample_fmt_entries[i];
        if (entry.sample_fmt == sample_fmt) {
            *fmt = AV_NE(entry.fmt_be, entry.fmt_le);
            ret = 0;
            break;
        }
    }
    return ret;
}

static int decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame, FILE *outfile) {
    int i, ch;
    int ret, data_size;
    ret = avcodec_send_packet(dec_ctx, pkt); // pkt ->frame 解码
    if (ret < 0) {
        return -1;
    }
    // 主要的格式为 带p 和不带p 的. 带p 会把每个声道的数据存储在不同的数组frame->data[i]里;
    // 而不带p的 所有的声道数据,一起交错储存在一个数组里 frame->data[0] 里.
    // 解码 frame format 会根据 AVContext 当前的格式自动生成...
    // 读取 所有可以接受到的 frame 中的数据
    while (1) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret < 0)break;
        // 获取每个采样点的字节数, 16位即两个字节数.
        data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        if (data_size < 0) {
            return -3;
        }
        // 带P 结尾的 每个声道数据单独 保存.. 不带P 结尾的, c1c2c1c2 排列..
        if (frame->format <= AV_SAMPLE_FMT_DBL) {
            int16_t *frame_data = (int16_t *) frame->data[0];
            for (i = 0; i < frame->nb_samples; i++) { // 采样点遍历
                fwrite(frame_data + i * data_size, 1, data_size * 2, outfile);
                zAudioFormat.sample_rate = dec_ctx->sample_rate;
                zAudioFormat.nb_channel = dec_ctx->ch_layout.nb_channels;
                zAudioFormat.bit_size = data_size;
                if (callback__) callback__(frame_data[i * data_size], &zAudioFormat);
            }
        } else {
            for (i = 0; i < frame->nb_samples; i++) { // 采样点遍历
                // uint8_t *[8] frame->data 默认有8组通道数据.. 每个通道数据占据一个...
                for (ch = 0; ch < dec_ctx->ch_layout.nb_channels - 1; ch++) {
                    fwrite(frame->data[ch] + i * data_size, 1, data_size, outfile);
                }
            }
        }
    }
    return ret;
}


int decode_test(const char *infile_path, const char *outfile_path, Callback callback) {
    callback__ = callback;
    const AVCodec *codec;
    AVCodecContext *ctx = NULL;
    AVCodecParserContext *parser = NULL;

    int ret;
    FILE *infile, *outfile;

    uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *data;
    size_t data_size;
    AVPacket *pkt;
    AVFrame *decoded_frame;
    enum AVSampleFormat sfmt;
    int n_channel = 0;

    pkt = av_packet_alloc();
    if (!pkt) return -10;

    decoded_frame = av_frame_alloc();
    if (!decoded_frame) return -11;

    codec = avcodec_find_decoder(AV_CODEC_ID_MP2);
    if (!codec) return -12;
    parser = av_parser_init(codec->id);
    if (!parser) return -13;

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) return -14;

    ret = avcodec_open2(ctx, codec, NULL);
    ctx->sample_fmt = 1;
    if (ret < 0) return -15;
    const enum AVSampleFormat *pp = codec->sample_fmts;
    while (*pp != AV_SAMPLE_FMT_NONE) {
        printf("decode AVSampleFormat: %d\n", *pp);
        pp++;
    }

    printf("decode format: %d\n", ctx->sample_fmt);
    infile = fopen(infile_path, "rb");
    if (!infile) return -16;
    outfile = fopen(outfile_path, "wb");
    if (!outfile) return -17;

    data = inbuf;
    data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, infile);

    // 将数据装入 packet 中,  然后进行 解码, 从frame 中获取到解码后的原始数据.
    while (data_size > 0) {
        ret = av_parser_parse2(parser, ctx, &pkt->data, &pkt->size,
                               data, (int) data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
//        printf("ret:%d,  data_size:%zd \n", ret, data_size);
        if (ret < 0) {
            return -18;
        }
        data += ret;
        data_size -= ret;

        if (pkt->size > 0) {
            decode(ctx, pkt, decoded_frame, outfile);
        }
        // 重新从 file中读取,,然后 需要把原来 剩下的数据 移动到 数组开头.. 从文件中读取的数据将加入到其后.
        if (data_size < AUDIO_REFILL_THRESH) {
            memmove(inbuf, data, data_size);
            data = inbuf; // 从头开始读取数据.
            size_t len = fread(inbuf + data_size, 1, AUDIO_INBUF_SIZE - data_size, infile);
//            printf("read file len:%zd\n", len);
            if (len > 0) {
                data_size += len;
            }
        }
    }

    pkt->data = NULL;
    pkt->size = 0;
    // pkt->data = NULL; 表示结束
    decode(ctx, pkt, decoded_frame, outfile);
    sfmt = ctx->sample_fmt;
    if (av_sample_fmt_is_planar(sfmt)) {
        const char *packed = av_get_sample_fmt_name(sfmt);
        printf("Warning: the sample format the decoder produced is planar "
               "(%s). This example will output the first channel only.\n",
               packed ? packed : "?");
        sfmt = av_get_packed_sample_fmt(sfmt);
    }
    const char *fmt;
    get_format_from_fmt(&fmt, sfmt);
    int n_channels = ctx->ch_layout.nb_channels;
    int sample_rate = ctx->sample_rate;

    printf("Play the output audio file with the command:\n"
           "ffplay -f %s -ac %d -ar %d %s\n",
           fmt, n_channels, sample_rate,
           outfile_path);


    fclose(infile);
    fclose(outfile);
    avcodec_close(ctx);
    av_frame_free(&decoded_frame);
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);

    return 0;
}
