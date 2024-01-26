

#include <iostream>
#include "library.h"
#include "Zfft.h"

extern "C" {
#include "libavutil/cpu.h"
#include "encode_audio.h"
#include "decode_audio.h"
}
using namespace std;

#define  FRAME_LEN  2048

float f_cache[FRAME_LEN * 2];
int size_c = 0; //当前 缓存的下标..

Zfft zfft(FRAME_LEN);
float fft_out[FRAME_LEN];

const char *mp3_path = R"(../res/Maria Arredondo - Burning.mp3)";
const char *pcm_path = R"(C:\Users\lionel\Desktop\bbb.txt)";
const char *fft_out_path = R"(C:\Users\lionel\Desktop\spetruct.txt)";

std::string fmt("%.4f");

FILE *fft_out_file = nullptr;
char content[20];

void analyze_fft(float *in) {
    zfft.fft_1d(in, fft_out);
    float sum = 0;
    for (int i = 0; i < 200; ++i) {
        fprintf(fft_out_file, "%.4f,", fft_out[i]);
        sum += fft_out[i];
    }
    fprintf(fft_out_file, "%.4f", sum / 200);
    fprintf(fft_out_file, "\n");
}

// 单声道数据... 根据 采样率. 计算 20ms 的窗移, 为下一步计算fft 提供数据...
void receivePcmU16Data(const int16_t data, ZAudioFormat *format) {
    f_cache[size_c++] = data;
    int windowTransfer = format->sample_rate / 50; // 窗移..
    while (size_c >= FRAME_LEN) {
        analyze_fft(f_cache);
        size_c -= windowTransfer;
        memmove(f_cache, f_cache + windowTransfer, size_c * 4);
    }
}

int main() {
    int ret;
    zfft.init();
    fft_out_file = fopen(fft_out_path, "wt");
//    int ret = encode_test(mp3_path);
    print2("ret: %d\n", ret);
    ret = decode_test(mp3_path, pcm_path, receivePcmU16Data);
    print2("ret: %d\n", ret);
    fclose(fft_out_file);
    return 0;
}

