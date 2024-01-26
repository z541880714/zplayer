//
// Created by lionel on 2023/11/7.
//

#ifndef UNTITLED1_DECODE_AUDIO_H
#define UNTITLED1_DECODE_AUDIO_H

#include <stdio.h>
#include <string.h>
#include "libavutil/frame.h"
#include "libavutil/mem.h"
#include "libavcodec/avcodec.h"


typedef struct ZAudioFormat {
    int sample_rate;
    int nb_channel;
    int bit_size; // 一般 16位
} ZAudioFormat;


typedef void (*Callback)(const int16_t, ZAudioFormat *);

int decode_test(const char *infile_path, Callback callback);

#endif //UNTITLED1_DECODE_AUDIO_H
