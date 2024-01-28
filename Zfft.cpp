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

int Zfft::fft_1d(const float *f_in, float *f_out) {
    // 输入数据...
    for (int i = 0; i < frame_len; ++i) {
        this->in[i] = f_in[i];
    }
    fftw_execute(p);
    // 只用 前面一半的数据才有效的.
    for (int i = 0; i < frame_len / 2; ++i) {
        int N = i == 0 ? frame_len : frame_len >> 1;
        double hypot_ = hypot(out[i][0], out[i][1]) / N;
        f_out[i] = hypot_ <= 0.0 ? DB_MIN : fmaxf((float) ((10 * log10(hypot_))), -30.0f);
        f_out[i] = interpolate(f_out[i], DB_MIN, DB_MAX, -160, 200);
    }
    return 0;
}

void Zfft::updateSampleRate(int rate) {
    if (this->sample_rate == rate) return;
    printf("rate: %d\n", rate);
    this->sample_rate = rate;
    frame_len = sample_rate / 10;
    /* frame_len = 1;
     while (frame_len < (sample_rate / 10 / 2)) {
         frame_len <<= 1;
     }*/
    delete (fft_out);
    fft_out = new float[frame_len]{0.0f};
    if (in) fftw_free(in);
    in = (double *) fftw_malloc(sizeof(double) * frame_len);
    if (out) fftw_free(out);
    out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * frame_len);
    std::cout << "frame_len:" << frame_len << std::endl;
    fftw_destroy_plan(p);
    p = fftw_plan_dft_r2c_1d(frame_len, in, out, FFTW_ESTIMATE);
}
