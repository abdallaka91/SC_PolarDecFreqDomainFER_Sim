
#pragma once

#include "simul_parameters.hpp"
#include <fftw3.h>
#include <cmath>
#include <complex>
#include <algorithm>

template <int L, typename SeqType = decltype(CCSKSequences::BASE_SEQ_128)>
class CCSK_LLR_float
{
    float noise_sigma;
    float total_scale;

    std::complex<float> pn_ffts[L];

    fftwf_plan fwd_plan;
    fftwf_plan rev_plan;

    fftwf_complex* in; 
    fftwf_complex* out;

public:

    CCSK_LLR_float(const float noise_sigma = 1.0)
        : noise_sigma(noise_sigma)
    {
        total_scale = 2.0 / (noise_sigma * noise_sigma);

        in       = fftwf_alloc_complex(L);
        out      = fftwf_alloc_complex(L);

        fwd_plan = fftwf_plan_dft_1d(L, in, out, FFTW_FORWARD,  FFTW_MEASURE);
        rev_plan = fftwf_plan_dft_1d(L, out, in, FFTW_BACKWARD, FFTW_MEASURE);

        compute_scaled_pn_fft();
    }

    ~CCSK_LLR_float()
    {
        fftwf_destroy_plan(fwd_plan);
        fftwf_destroy_plan(rev_plan);

        fftwf_free(in);
        fftwf_free(out);
    }

    void calculate(const double *y, double *llr)
    {
        for (int i = 0; i < L; i++)
        {
            in[i][0] = y[i];
            in[i][1] = 0.f;
        }

        fftwf_execute_dft(fwd_plan, in, out);

        for (int i = 0; i < L; i++)
        {
            const float a = out[i][0];
            const float b = out[i][1];
            const float c = pn_ffts[i].real();
            const float d = pn_ffts[i].imag();
            in[i][0] = a * c - b * d;
            in[i][1] = a * d + b * c;
        }

        fftwf_execute_dft(rev_plan, in, out);

        float min_val = 3.40282347e+37f;
        for (int i = 0; i < L; i++)
        {
            float val = out[i][0] / L; // FFT scaling
            llr[i] = val;
            if (val < min_val)
                min_val = val;
        }

        for (int i = 0; i < L; i++)
        {
            llr[i] -= min_val;
        }
    }
/*
    template <int BatchSize = 4>
    void calculate_batch(const double **y_batch, double **llr_batch)
    {
        for (int b = 0; b < BatchSize; b++)
        {
            calculate(y_batch[b], llr_batch[b]);
        }
    }
*/
private:

    ////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //
    //
    void compute_scaled_pn_fft( )
    {
        const auto &seq = get_sequence<L>();

        for (int i = 0; i < L; i++)
        {
            float value = (i == 0) ? seq[0] : seq[L - i];
            in[i][0] = value;
            in[i][1] = 0.0;
        }

        fftwf_execute_dft(fwd_plan, in, out);

        for (int i = 0; i < L; i++)
        {
            float real_p = out[i][0] * total_scale;
            float imag_p = out[i][1] * total_scale;
            std::complex c = {real_p, imag_p};
            pn_ffts[i] = c;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //
    //
    template <int Size>
    const auto &get_sequence()
    {
		// WRONG CODE BLG 23/05/2026
        if constexpr (Size == 8)
            return CCSKSequences::BASE_SEQ_8;
        else if constexpr (Size == 16)
            return CCSKSequences::BASE_SEQ_16;
        else if constexpr (Size == 32)
            return CCSKSequences::BASE_SEQ_32;
        else if constexpr (Size == 64)
            return CCSKSequences::BASE_SEQ_64;
        else if constexpr (Size == 128)
            return CCSKSequences::BASE_SEQ_128;
        else if constexpr (Size == 256)
            return CCSKSequences::BASE_SEQ_256;
        else if constexpr (Size == 512)
            return CCSKSequences::BASE_SEQ_512;
        else if constexpr (Size == 1024)
            return CCSKSequences::BASE_SEQ_1024;
        else
            static_assert(Size == 8 || Size == 16 || Size == 32 || Size == 64 || Size == 128 || Size == 256 || Size == 512 || Size == 1024, "Unsupported sequence length");
    }

    void complex_multiply_simd(double *a_real, double *a_imag)
    {
//#ifdef __AVX2__
//        for (int i = 0; i < L; i += 4)
//        {
//            __m256d ar = _mm256_load_pd(&a_real[i]);
//            __m256d ai = _mm256_load_pd(&a_imag[i]);
//            __m256d cr = _mm256_load_pd(&pn_fft.real[i]);
//            __m256d ci = _mm256_load_pd(&pn_fft.imag[i]);
//
//            __m256d real = _mm256_fmsub_pd(ar, cr, _mm256_mul_pd(ai, ci));
//            __m256d imag = _mm256_fmadd_pd(ar, ci, _mm256_mul_pd(ai, cr));
//
//            _mm256_store_pd(&a_real[i], real);
//            _mm256_store_pd(&a_imag[i], imag);
//        }
//#else
        for (int i = 0; i < L; i++)
        {
            const float ar = a_real[i];
            const float ai = a_imag[i];
            const float cr = pn_ffts[i].real();
            const float ci = pn_ffts[i].imag();
            a_real[i] = ar * cr - ai * ci;
            a_imag[i] = ar * ci + ai * cr;
        }
//#endif
    }
};

//template <int L, typename SeqType>
//thread_local typename CCSK_LLR<L, SeqType>::ThreadBuffers CCSK_LLR<L, SeqType>::tls_buffers;
