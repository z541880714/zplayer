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

int Zfft::fft_1d(const float *const f_in) {
    // 输入数据...
    float aver, sum;
    for (int i = 0; i < fft_N; ++i) sum += f_in[i];
    aver = sum / fft_N;
    for (int i = 0; i < fft_N; ++i) this->in[i] -= aver;
    fftw_execute(p);
    float fac = 1.0f / fft_N;
    for (int i = 0; i < fft_N; ++i) {
        double hypot_ = hypot(out[i][0], out[i][1]) * (i == 0 ? fac : fac * 0.5f);
        fft_out[i] = hypot_ <= 0.0 ? DB_MIN : (float) (10 * log10(hypot_));
        fft_out[i] = fft_out[i] == DB_MIN ? -160 : interpolate(fft_out[i], DB_MIN, DB_MAX, -160, 200) + 130;
    }
    return 0;
}

void Zfft::updateSampleRate(int rate) {
    if (this->sample_rate == rate) return;
    this->sample_rate = rate;
    this->fft_N = 1;
    // 实际需要的 fft 点的数量  fftN,
    // 需要进一步优化, 把 fft 的数量 变成 2 的N 次方. 取最靠近[fftN] 的数
    int realFFtN = sample_rate / FREQ_INTERVAL;
    while (fft_N < realFFtN) fft_N <<= 1;
    fft_N = abs(fft_N - realFFtN) < abs((fft_N << 1) - realFFtN) ? fft_N : (fft_N << 1);

    delete (fft_out);
    fft_out = new float[fft_N / 2]{0.0f};
    if (in) fftw_free(in);
    in = (double *) fftw_malloc(sizeof(double) * fft_N);
    if (out) fftw_free(out);
    out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * fft_N);
    std::cout << "fft_N:" << fft_N << " delta freq:" << sample_rate * 1.0 / fft_N << std::endl;

    fftw_destroy_plan(p);
    p = fftw_plan_dft_r2c_1d(fft_N, in, out, FFTW_ESTIMATE);
}
