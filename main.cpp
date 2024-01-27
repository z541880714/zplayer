

#include <iostream>
#include "library.h"
#include "Zfft.h"
#include "stringutil.h"

extern "C" {
#include "libavutil/cpu.h"
#include "encode_audio.h"
#include "decode_audio.h"
}
using namespace std;

#define  FRAME_LEN  2048
#define  SPECTRAL_FRAME_LEN 200

float f_cache[FRAME_LEN * 2];
int size_c = 0; //当前 缓存的下标..

Zfft zfft(FRAME_LEN);
float fft_out[FRAME_LEN];


string audio_path = R"(..\res\Maria Arredondo - Burning.mp3)";
string out_dir = R"(C:\Users\lionel\Desktop)";
string song_name;

string fft_out_spectral_path;
string fft_out_power_path;

std::string fmt("%.4f");

FILE *fft_out_spectral_file = nullptr;
FILE *fft_out_power_file = nullptr;
char content[20];

void analyze_fft(float *in) {
    zfft.fft_1d(in, fft_out);
    float sum = 0;
    for (int i = 0; i < SPECTRAL_FRAME_LEN; ++i) {
        fprintf(fft_out_spectral_file, "%.4f", fft_out[i]);
        if (i < SPECTRAL_FRAME_LEN - 1) {
            fprintf(fft_out_spectral_file, ",");
        }
        sum += fft_out[i];
    }
    fprintf(fft_out_spectral_file, "\n");
    fprintf(fft_out_power_file, "%.4f", sum / SPECTRAL_FRAME_LEN);
    fprintf(fft_out_power_file, "\n");
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


int main(int len, char **args) {
    printf("len:%d\n", len);
    if (len == 3) {
        audio_path = string(args[1]);
        out_dir = string(args[2]);
    }
    int lastSeparatorIndex = audio_path.find_last_of('\\');
    int lastPointIndex = audio_path.find_last_of('.');
    printf("index: %d->%d len:%zu, path:%s\n", lastSeparatorIndex, lastPointIndex, audio_path.size(),
           audio_path.c_str());
    song_name = audio_path.substr(lastSeparatorIndex + 1, lastPointIndex - lastSeparatorIndex - 1);

    fft_out_spectral_path = str_fmt(R"(C:\Users\lionel\Desktop\spetruct_%s.txt)", song_name.c_str());
    fft_out_power_path = str_fmt(R"(C:\Users\lionel\Desktop\power_%s.txt)", song_name.c_str());
    printf("audio_path:%s, out_dir:%s, song name:%s, out_spectral:%s\n", audio_path.data(), out_dir.c_str(),
           song_name.c_str(), fft_out_spectral_path.c_str());
    int ret;
    zfft.init();
    fft_out_spectral_file = fopen(fft_out_spectral_path.c_str(), "wt");
    fft_out_power_file = fopen(fft_out_power_path.c_str(), "wt");
    ret = decode_test(audio_path.c_str(), receivePcmU16Data);
    print2("ret: %d\n", ret);
    fclose(fft_out_spectral_file);
    fclose(fft_out_power_file);
    return 0;
}

