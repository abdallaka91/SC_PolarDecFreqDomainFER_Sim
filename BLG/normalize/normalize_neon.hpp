//
// Created by legal on 03/07/2025.
//
#pragma once
#include <arm_neon.h>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <int gf_size> inline void normalize(float * tab) {
    float sum = 1e-32f;
    for (int i = 0; i < gf_size; i += 1) {
        sum += tab[i];
    }
    const float factor = 1.f / sum;
    for (int i = 0; i < gf_size; i++) {
        tab[i] *= factor;
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<8>(float * data) {
    float32x4_t v0 = vld1q_f32(data);
    float32x4_t v1 = vld1q_f32(data + 4);

    // Somme avec 1e-32f pour éviter division par zéro
    float32x4_t sum = vaddq_f32(vdupq_n_f32(1e-32f), v0);
    sum = vaddq_f32(sum, v1);

    // Réduction horizontale
    float32x2_t sum_low = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
    float total_sum = vget_lane_f32(sum_low, 0) + vget_lane_f32(sum_low, 1);

    // Calcul de l'inverse pour normalisation
    float inv_sum = 1.0f / total_sum;
    float32x4_t inv_sum_vec = vdupq_n_f32(inv_sum);

    // Normalisation in-place
    v0 = vmulq_f32(v0, inv_sum_vec);
    v1 = vmulq_f32(v1, inv_sum_vec);

    vst1q_f32(data, v0);
    vst1q_f32(data + 4, v1);
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<16>(float * data) {
    float32x4_t v0 = vld1q_f32(data);
    float32x4_t v1 = vld1q_f32(data + 4);
    float32x4_t v2 = vld1q_f32(data + 8);
    float32x4_t v3 = vld1q_f32(data + 12);

    // Somme avec 1e-32f
    float32x4_t sum = vaddq_f32(vdupq_n_f32(1e-32f), v0);
    sum = vaddq_f32(sum, v1);
    sum = vaddq_f32(sum, v2);
    sum = vaddq_f32(sum, v3);

    // Réduction horizontale
    float32x2_t sum_low = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
    float total_sum = vget_lane_f32(sum_low, 0) + vget_lane_f32(sum_low, 1);

    // Inverse pour normalisation
    float inv_sum = 1.0f / total_sum;
    float32x4_t inv_sum_vec = vdupq_n_f32(inv_sum);

    // Normalisation in-place
    v0 = vmulq_f32(v0, inv_sum_vec);
    v1 = vmulq_f32(v1, inv_sum_vec);
    v2 = vmulq_f32(v2, inv_sum_vec);
    v3 = vmulq_f32(v3, inv_sum_vec);

    vst1q_f32(data, v0);
    vst1q_f32(data + 4, v1);
    vst1q_f32(data + 8, v2);
    vst1q_f32(data + 12, v3);
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<32>(float * data) {
    float32x4_t v0 = vld1q_f32(data);
    float32x4_t v1 = vld1q_f32(data + 4);
    float32x4_t v2 = vld1q_f32(data + 8);
    float32x4_t v3 = vld1q_f32(data + 12);
    float32x4_t v4 = vld1q_f32(data + 16);
    float32x4_t v5 = vld1q_f32(data + 20);
    float32x4_t v6 = vld1q_f32(data + 24);
    float32x4_t v7 = vld1q_f32(data + 28);

    // Somme initiale avec 1e-32f
    float32x4_t sum = vaddq_f32(vdupq_n_f32(1e-32f), v0);
    sum = vaddq_f32(sum, v1);
    sum = vaddq_f32(sum, v2);
    sum = vaddq_f32(sum, v3);
    sum = vaddq_f32(sum, v4);
    sum = vaddq_f32(sum, v5);
    sum = vaddq_f32(sum, v6);
    sum = vaddq_f32(sum, v7);

    // Réduction horizontale
    float32x2_t sum_low = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
    float total_sum = vget_lane_f32(sum_low, 0) + vget_lane_f32(sum_low, 1);

    float inv_sum = 1.0f / total_sum;
    float32x4_t inv_sum_vec = vdupq_n_f32(inv_sum);

    // Normalisation in-place
    v0 = vmulq_f32(v0, inv_sum_vec);
    v1 = vmulq_f32(v1, inv_sum_vec);
    v2 = vmulq_f32(v2, inv_sum_vec);
    v3 = vmulq_f32(v3, inv_sum_vec);
    v4 = vmulq_f32(v4, inv_sum_vec);
    v5 = vmulq_f32(v5, inv_sum_vec);
    v6 = vmulq_f32(v6, inv_sum_vec);
    v7 = vmulq_f32(v7, inv_sum_vec);

    vst1q_f32(data, v0);
    vst1q_f32(data + 4, v1);
    vst1q_f32(data + 8, v2);
    vst1q_f32(data + 12, v3);
    vst1q_f32(data + 16, v4);
    vst1q_f32(data + 20, v5);
    vst1q_f32(data + 24, v6);
    vst1q_f32(data + 28, v7);
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<64>(float * data) {
    float32x4_t v[16];
    for (int i = 0; i < 16; i++) {
        v[i] = vld1q_f32(data + i * 4);
    }

    float32x4_t sum = vdupq_n_f32(1e-32f);
    for (int i = 0; i < 16; i++) {
        sum = vaddq_f32(sum, v[i]);
    }

    // Réduction horizontale
    float32x2_t sum_low = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
    float total_sum = vget_lane_f32(sum_low, 0) + vget_lane_f32(sum_low, 1);

    float inv_sum = 1.0f / total_sum;
    float32x4_t inv_sum_vec = vdupq_n_f32(inv_sum);

    for (int i = 0; i < 16; i++) {
        v[i] = vmulq_f32(v[i], inv_sum_vec);
        vst1q_f32(data + i * 4, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<128>(float * data) {
    const int vec_count = 32; // 128 / 4
    float32x4_t v[vec_count];

    for (int i = 0; i < vec_count; i++) {
        v[i] = vld1q_f32(data + i * 4);
    }

    float32x4_t sum = vdupq_n_f32(1e-32f);
    for (int i = 0; i < vec_count; i++) {
        sum = vaddq_f32(sum, v[i]);
    }

    float32x2_t sum_low = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
    float total_sum = vget_lane_f32(sum_low, 0) + vget_lane_f32(sum_low, 1);

    float inv_sum = 1.0f / total_sum;
    float32x4_t inv_sum_vec = vdupq_n_f32(inv_sum);

    for (int i = 0; i < vec_count; i++) {
        v[i] = vmulq_f32(v[i], inv_sum_vec);
        vst1q_f32(data + i * 4, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<256>(float * data) {
    const int vec_count = 64; // 256 / 4
    float32x4_t v[vec_count];

    for (int i = 0; i < vec_count; i++) {
        v[i] = vld1q_f32(data + i * 4);
    }

    float32x4_t sum = vdupq_n_f32(1e-32f);
    for (int i = 0; i < vec_count; i++) {
        sum = vaddq_f32(sum, v[i]);
    }

    float32x2_t sum_low = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
    float total_sum = vget_lane_f32(sum_low, 0) + vget_lane_f32(sum_low, 1);

    float inv_sum = 1.0f / total_sum;
    float32x4_t inv_sum_vec = vdupq_n_f32(inv_sum);

    for (int i = 0; i < vec_count; i++) {
        v[i] = vmulq_f32(v[i], inv_sum_vec);
        vst1q_f32(data + i * 4, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<512>(float * data) {
    const int vec_count = 128; // 512 / 4
    float32x4_t v[vec_count];

    for (int i = 0; i < vec_count; i++) {
        v[i] = vld1q_f32(data + i * 4);
    }

    float32x4_t sum = vdupq_n_f32(1e-32f);
    for (int i = 0; i < vec_count; i++) {
        sum = vaddq_f32(sum, v[i]);
    }

    float32x2_t sum_low = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
    float total_sum = vget_lane_f32(sum_low, 0) + vget_lane_f32(sum_low, 1);

    float inv_sum = 1.0f / total_sum;
    float32x4_t inv_sum_vec = vdupq_n_f32(inv_sum);

    for (int i = 0; i < vec_count; i++) {
        v[i] = vmulq_f32(v[i], inv_sum_vec);
        vst1q_f32(data + i * 4, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<1024>(float * data) {
    const int vec_count = 256; // 1024 / 4
    float32x4_t v[vec_count];

    for (int i = 0; i < vec_count; i++) {
        v[i] = vld1q_f32(data + i * 4);
    }

    float32x4_t sum = vdupq_n_f32(1e-32f);
    for (int i = 0; i < vec_count; i++) {
        sum = vaddq_f32(sum, v[i]);
    }

    float32x2_t sum_low = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
    float total_sum = vget_lane_f32(sum_low, 0) + vget_lane_f32(sum_low, 1);

    float inv_sum = 1.0f / total_sum;
    float32x4_t inv_sum_vec = vdupq_n_f32(inv_sum);

    for (int i = 0; i < vec_count; i++) {
        v[i] = vmulq_f32(v[i], inv_sum_vec);
        vst1q_f32(data + i * 4, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
