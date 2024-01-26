//
// Created by lionel on 2024/1/25.
//

#ifndef ZPLAYER_ZFFT_H
#define ZPLAYER_ZFFT_H

#include "api/fftw3.h"

class Zfft {
public:
    Zfft(int frame_len) : frame_len(frame_len) {}

    ~Zfft() {
        fftw_free(in);
        fftw_free(out);
        fftw_destroy_plan(p);
    }

    void init() {
        in = (double *) fftw_malloc(sizeof(double) * frame_len);
        out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * frame_len);
        p = fftw_plan_dft_r2c_1d(frame_len, in, out, FFTW_ESTIMATE);
    }

    int fft_1d(const float *in, float *out);

private:
    int frame_len;
    double *in;
    fftw_complex *out;
    fftw_plan p;
};


#endif //ZPLAYER_ZFFT_H
