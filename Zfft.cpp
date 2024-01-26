//
// Created by lionel on 2024/1/25.
//

#include "Zfft.h"
#include <cmath>

#define DB_MIN (-30)
#define DB_MAX (80)

float interpolate(float f, float src_min, float src_max, float des_min, float des_max) {
    return (f - src_min) * (des_max - des_min) / (src_max - src_min) + des_min;
}

int Zfft::fft_1d(const float *const f_in, float *const f_out) {
    // 输入数据...
    for (int i = 0; i < frame_len; ++i) {
        this->in[i] = f_in[i];
    }
    fftw_execute(p);
    for (int i = 0; i < frame_len; ++i) {
        double hypot_ = hypot(out[i][0], out[i][1]);
        f_out[i] = hypot_ <= 0.0 ? DB_MIN : (float) (10 * log10(hypot_));
        f_out[i] = interpolate(f_out[i], DB_MIN, DB_MAX, -160, 200);
    }
    return 0;
}
