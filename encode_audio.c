/*
 * Copyright (c) 2001 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file libavcodec encoding audio API usage examples
 * @example encode_audio.c
 *
 * Generate a synthetic audio signal and encode it to an output MP2 file.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>

#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>

/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(const AVCodec *codec, enum AVSampleFormat sample_fmt) {
    const enum AVSampleFormat *p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE) {
        printf("format: %d\n", *p);
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}

/* just pick the highest supported samplerate */
static int select_sample_rate(const AVCodec *codec) {
    const int *p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while (*p) {
        if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
            best_samplerate = *p;
        p++;
    }
    return best_samplerate;
}

/* select layout with the highest channel count */
static int select_channel_layout(const AVCodec *codec, AVChannelLayout *dst) {
    const AVChannelLayout *p, *best_ch_layout;
    int best_nb_channels = 0;

    if (!codec->ch_layouts)
        return av_channel_layout_copy(dst, codec->ch_layouts);

    p = codec->ch_layouts;
    while (p->nb_channels) {
        int nb_channels = p->nb_channels;

        if (nb_channels > best_nb_channels) {
            best_ch_layout = p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return av_channel_layout_copy(dst, best_ch_layout);
}

static void encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt,
                   FILE *output) {
    int ret;

    /* send the frame for encoding */
    ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending the frame to the encoder\n");
        exit(1);
    }

    /* read all the available output packets (in general there may be any
     * number of them */
    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error encoding audio frame\n");
            exit(1);
        }

        fwrite(pkt->data, 1, pkt->size, output);
        av_packet_unref(pkt);
    }
}

int encode_test(const char *filename) {
    const AVCodec *codec;
    AVCodecContext *ctx = NULL;
    AVFrame *frame;
    AVPacket *pkt;
    int i, j, k, ret;
    FILE *f;
    uint16_t *samples;
    float t, tincr;

    /* find the MP2 encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }

    /* put sample parameters */
    ctx->bit_rate = 64000;

    /* check that the encoder supports s16 pcm input */
    // 格式决定了, 数据在 frame->data 中 储存方式.

//    ctx->sample_fmt = AV_SAMPLE_FMT_S16;
    ctx->sample_fmt = *codec->sample_fmts;

    if (!check_sample_fmt(codec, ctx->sample_fmt)) {
        fprintf(stderr, "Encoder does not support sample format %s",
                av_get_sample_fmt_name(ctx->sample_fmt));
        exit(1);
    }

    /* select other audio parameters supported by the encoder */
    ctx->sample_rate = select_sample_rate(codec);
    ret = select_channel_layout(codec, &ctx->ch_layout);
    if (ret < 0)
        exit(1);

    /* open it */
    if (avcodec_open2(ctx, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    /* packet for holding encoded output */
    pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "could not allocate the packet\n");
        exit(1);
    }

    /* frame containing input raw audio */
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate audio frame\n");
        exit(1);
    }

    frame->nb_samples = ctx->frame_size;
    printf("nb_samples:%d\n", frame->nb_samples);
    frame->format = ctx->sample_fmt;
    ret = av_channel_layout_copy(&frame->ch_layout, &ctx->ch_layout);
    if (ret < 0)
        exit(1);

    /* allocate the data buffers */
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate audio data buffers\n");
        exit(1);
    }

    /* encode a single tone sound */
    t = 0;
    tincr = 2 * M_PI * 100.0 / ctx->sample_rate;
    for (i = 0; i < 2000; i++) {
        ret = av_frame_make_writable(frame);
        if (ret < 0)
            exit(1);

        if (ctx->sample_fmt <= AV_SAMPLE_FMT_DBL) {
            samples = (uint16_t *) frame->data[0];
            for (j = 0; j < ctx->frame_size; j++) {
                int value = (int) (sin(t) * 10000);
                for (k = 0; k < ctx->ch_layout.nb_channels; k++) {
                    samples[2 * j + k] = value;
                }
                t += tincr;
            }
        } else {
            // planar 二维数据...
            for (j = 0; j < frame->ch_layout.nb_channels; j++) {
                samples = (uint16_t *) frame->data[j];
                int value = (int) (sin(t) * 10000);
                for (k = 0; k < frame->nb_samples; k++) {
                    samples[k] = value;
                }
            }
        }
        encode(ctx, frame, pkt, f);
    }

    /* flush the encoder */
    encode(ctx, NULL, pkt, f);

    fclose(f);

    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);

    return 0;
}


