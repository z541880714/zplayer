//
// Created by lionel on 2024/1/25.
//

#ifndef ZPLAYER_ZFFT_H
#define ZPLAYER_ZFFT_H

#include <cstdlib>
#include "api/fftw3.h"

class Zfft {
public:
    Zfft() {}

    ~Zfft() {
        fftw_free(in);
        fftw_free(out);
        fftw_destroy_plan(p);
        delete (fft_out);
    }

    void init() {

    }

    int fft_1d(const float *in, float *out);

    void updateSampleRate(int rate);

private:
    int sample_rate = 0;
    double *in = nullptr;
    fftw_complex *out;
    fftw_plan p;
public:
    float *fft_out = nullptr;
    int frame_len;
};


#endif //ZPLAYER_ZFFT_H
