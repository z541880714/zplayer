//
// Created by lionel on 2024/1/25.
//

#ifndef ZPLAYER_ZFFT_H
#define ZPLAYER_ZFFT_H

#include <cstdlib>
#include "api/fftw3.h"

// 目标 频域 相邻两个频点间的跨度..
#define FREQ_INTERVAL  10

class Zfft {
public:
    Zfft() {};

    ~Zfft() {
        fftw_free(in);
        fftw_free(out);
        fftw_destroy_plan(p);
        delete (fft_out);
    }

    void init() {
    }

    int fft_1d(const float *in);

    void updateSampleRate(int rate);

public:
    float *fft_out = nullptr;
    int fft_N;


private:
    double *in = nullptr;
    fftw_complex *out;
    fftw_plan p;
    int sample_rate = 0;

};


#endif //ZPLAYER_ZFFT_H
