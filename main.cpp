

#include <iostream>
#include <sstream>
#include "library.h"
#include "Zfft.h"
#include "stringutil.h"
#include <io.h>

extern "C" {
#include "libavutil/cpu.h"
#include "encode_audio.h"
#include "decode_audio.h"
}
using namespace std;

#define  SPECTRAL_FRAME_LEN 200

float f_cache[10000];
int size_c = 0; //当前 缓存的下标..

Zfft zfft;


string audio_path = R"(..\res\Maria Arredondo - Burning.mp3)";
string out_dir = R"(..\data_output)";
string song_name;

string fft_out_spectral_path;
string fft_out_power_path;

FILE *fft_out_spectral_file = nullptr;
FILE *fft_out_power_file = nullptr;

float envelope[SPECTRAL_FRAME_LEN];

void analyze_fft(float *in) {
    zfft.fft_1d(in);
    float sum = 0;
    for (int i = 0; i < SPECTRAL_FRAME_LEN; ++i) {
        envelope[i] = zfft.fft_out[i];
        fprintf(fft_out_spectral_file, "%.4f", envelope[i]);
        if (i < SPECTRAL_FRAME_LEN - 1) {
            fprintf(fft_out_spectral_file, ",");
        }
        sum += zfft.fft_out[i];
    }
    fprintf(fft_out_spectral_file, "\n");
//    fprintf(fft_out_power_file, "%.4f", sum / SPECTRAL_FRAME_LEN);
//    fprintf(fft_out_power_file, "\n");
}

// 单声道数据... 根据 采样率. 计算 20ms 的窗移, 为下一步计算fft 提供数据...
void receivePcmU16Data(const INT_16 data, ZAudioFormat *format) {
    zfft.updateSampleRate(format->sample_rate);
    f_cache[size_c++] = data;
//    float f = (1.0f - fabsf(data - 32768) / 32768.0f) * (data > 32768 ? -1.0f : 1.0f);
//    f_cache[size_c++] = f * 32768;
    int windowTransfer = format->sample_rate / 50; // 窗移..
    while (size_c >= zfft.fft_N) {
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

    fft_out_spectral_path = str_fmt(R"(%s\spetruct_%s.txt)", out_dir.c_str(), song_name.c_str());
    fft_out_power_path = str_fmt(R"(%s\power_%s.txt)", out_dir.c_str(), song_name.c_str());
    printf("audio_path:%s, out_dir:%s, song name:%s, out_spectral:%s\n", audio_path.data(), out_dir.c_str(),
           song_name.c_str(), fft_out_spectral_path.c_str());
    cout << out_dir << endl;
    if (access(out_dir.c_str(), 0777) != 0) {
        mkdir(out_dir.c_str());
    }
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

