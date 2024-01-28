//
// Created by lionel on 2024/1/25.
//

#include "Zfft.h"
#include <cmath>
#include <iostream>

#define DB_MIN (-30)
#define DB_MAX (80)

float interpolate(float f, float src_min, float src_max, float des_min, float des_max) {
    return (f - src_min) * (des_max - des_min) / (src_max - src_min) + des_min;
}

int Zfft::fft_1d(const float *f_in) {
    // 输入数据...
    for (int i = 0; i < fft_N; ++i) {
        this->in[i] = f_in[i];
    }
    fftw_execute(p);
    float factor = 1.0f / (float) fft_N;
    float factor_half = factor * 0.5f;
    // 只用 前面一半的数据才有效的.
    for (int i = 0; i < fft_N / 2; ++i) {
        double hypot_ = hypot(out[i][0], out[i][1]) * (i == 0 ? factor : factor_half);
        fft_out[i] = hypot_ <= 0.0 ? DB_MIN : fmaxf((float) ((10 * log10(hypot_))), -30.0f) + 30;
        fft_out[i] = interpolate(fft_out[i], DB_MIN, DB_MAX, -160, 200);
    }
    return 0;
}

void Zfft::updateSampleRate(int rate) {
    if (this->sample_rate == rate) return;
    printf("rate: %d\n", rate);
    this->sample_rate = rate;
    fft_N = sample_rate / 10;
    /* fft_N = 1;
     while (fft_N < (sample_rate / 10 / 2)) {
         fft_N <<= 1;
     }*/
    delete (fft_out);
    fft_out = new float[fft_N / 2]{0.0f};
    if (in) fftw_free(in);
    in = (double *) fftw_malloc(sizeof(double) * fft_N);
    if (out) fftw_free(out);
    out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * fft_N);
    std::cout << "fft_N:" << fft_N << std::endl;
    fftw_destroy_plan(p);
    p = fftw_plan_dft_r2c_1d(fft_N, in, out, FFTW_ESTIMATE);
}
