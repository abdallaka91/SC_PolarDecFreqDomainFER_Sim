
#pragma once

#include "simul_parameters.hpp"
#include <fftw3.h>
#include <cmath>
#include <algorithm>

template <int L, typename SeqType = decltype(CCSKSequences::BASE_SEQ_128)>
class CCSK_LLR
{
    static_assert((L & (L - 1)) == 0, "L must be power of 2");

    double sigma;
    double total_scale;

    struct FFTSOA
    {
        alignas(32) double real[L];
        alignas(32) double imag[L];
    };

    FFTSOA pn_fft;

    fftw_plan fwd_plan, rev_plan;

public:
    CCSK_LLR(double sigma = 1.0) : sigma(sigma)
    {
        total_scale = 2.0 / (sigma * sigma);

        fftw_complex *in = fftw_alloc_complex(L);
        fftw_complex *out = fftw_alloc_complex(L);

        fwd_plan = fftw_plan_dft_1d(L, in, out, FFTW_FORWARD, FFTW_MEASURE);
        rev_plan = fftw_plan_dft_1d(L, out, in, FFTW_BACKWARD, FFTW_MEASURE);

        compute_scaled_pn_fft(in, out);

        fftw_free(in);
        fftw_free(out);
    }

    ~CCSK_LLR()
    {
        fftw_destroy_plan(fwd_plan);
        fftw_destroy_plan(rev_plan);
    }

    struct ThreadBuffers
    {
        fftw_complex *in, *out;
        ThreadBuffers()
        {
            in = fftw_alloc_complex(L);
            out = fftw_alloc_complex(L);
        }
        ~ThreadBuffers()
        {
            fftw_free(in);
            fftw_free(out);
        }
    };

    static thread_local ThreadBuffers tls_buffers;

    void calculate(const double *y, double *llr)
    {
        fftw_complex *in = tls_buffers.in;
        fftw_complex *out = tls_buffers.out;

        for (int i = 0; i < L; i++)
        {
            in[i][0] = y[i];
            in[i][1] = 0.0;
        }

        fftw_execute_dft(fwd_plan, in, out);

        for (int i = 0; i < L; i++)
        {
            double a = out[i][0], b = out[i][1];
            double c = pn_fft.real[i], d = pn_fft.imag[i];
            in[i][0] = a * c - b * d;
            in[i][1] = a * d + b * c;
        }

        fftw_execute_dft(rev_plan, in, out);

        double min_val = 1e100;
        for (int i = 0; i < L; i++)
        {
            double val = out[i][0] / L; // FFT scaling
            llr[i] = val;
            if (val < min_val)
                min_val = val;
        }

        for (int i = 0; i < L; i++)
        {
            llr[i] -= min_val;
        }
    }

    template <int BatchSize = 4>
    void calculate_batch(const double **y_batch, double **llr_batch)
    {
        for (int b = 0; b < BatchSize; b++)
        {
            calculate(y_batch[b], llr_batch[b]);
        }
    }

private:
    void compute_scaled_pn_fft(fftw_complex *in, fftw_complex *out)
    {
        const auto &seq = get_sequence<L>();

        for (int i = 0; i < L; i++)
        {
            double value = (i == 0) ? seq[0] : seq[L - i];
            in[i][0] = value;
            in[i][1] = 0.0;
        }

        fftw_execute_dft(fwd_plan, in, out);

        for (int i = 0; i < L; i++)
        {
            pn_fft.real[i] = out[i][0] * total_scale;
            pn_fft.imag[i] = out[i][1] * total_scale;
        }
    }

    template <int Size>
    const auto &get_sequence()
    {
        if constexpr (Size == 64)
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
            static_assert(Size == 64 || Size == 128 || Size == 256 || Size == 512 || Size == 1024, "Unsupported sequence length");
    }

    void complex_multiply_simd(double *a_real, double *a_imag)
    {
#ifdef __AVX2__
        for (int i = 0; i < L; i += 4)
        {
            __m256d ar = _mm256_load_pd(&a_real[i]);
            __m256d ai = _mm256_load_pd(&a_imag[i]);
            __m256d cr = _mm256_load_pd(&pn_fft.real[i]);
            __m256d ci = _mm256_load_pd(&pn_fft.imag[i]);

            __m256d real = _mm256_fmsub_pd(ar, cr, _mm256_mul_pd(ai, ci));
            __m256d imag = _mm256_fmadd_pd(ar, ci, _mm256_mul_pd(ai, cr));

            _mm256_store_pd(&a_real[i], real);
            _mm256_store_pd(&a_imag[i], imag);
        }
#else
        for (int i = 0; i < L; i++)
        {
            double ar = a_real[i], ai = a_imag[i];
            double cr = pn_fft.real[i], ci = pn_fft.imag[i];
            a_real[i] = ar * cr - ai * ci;
            a_imag[i] = ar * ci + ai * cr;
        }
#endif
    }
};

template <int L, typename SeqType>
thread_local typename CCSK_LLR<L, SeqType>::ThreadBuffers CCSK_LLR<L, SeqType>::tls_buffers;
