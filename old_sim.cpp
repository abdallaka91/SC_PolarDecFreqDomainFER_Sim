#include "ccsk_llr.h"
#include <iostream>
#include <random>
#include <chrono>

#ifdef _OPENMP
#include <omp.h>
#else
inline int omp_get_thread_num() { return 0; }
inline int omp_get_max_threads() { return 1; }
#endif

template <int _GF_>
constexpr auto get_base_seq_float()
{
    constexpr auto get_seq = []() -> const auto &
    {
        if constexpr (_GF_ == 64)
            return CCSKSequences::BASE_SEQ_64;
        if constexpr (_GF_ == 128)
            return CCSKSequences::BASE_SEQ_128;
        if constexpr (_GF_ == 256)
            return CCSKSequences::BASE_SEQ_256;
        if constexpr (_GF_ == 512)
            return CCSKSequences::BASE_SEQ_512;
        if constexpr (_GF_ == 1024)
            return CCSKSequences::BASE_SEQ_1024;
    };

    constexpr auto &int_seq = get_seq();
    std::array<double, 2 * _GF_> float_seq{};

    for (int i = 0; i < _GF_; i++)
    {
        float_seq[i] = -static_cast<double>(int_seq[i]) * 2 + 1;
        float_seq[i + _GF_] = float_seq[i];
    }
    return float_seq;
}
int main()
{
    constexpr int _GF_ = 1024;
    constexpr int N = 1024;                // GF symbols per frame
    constexpr int CHIPS_PER_SYMBOL = _GF_; // chips per GF symbol
    constexpr int NbMonteCarlo = 2000;
    const double sigma = 1.2;

    CCSK_LLR<CHIPS_PER_SYMBOL> llr_calc(sigma);
    constexpr auto base_seq = get_base_seq_float<_GF_>();

    int frame_errors = 0;
    auto start = std::chrono::high_resolution_clock::now();

#ifdef _OPENMP
#pragma omp parallel reduction(+ : frame_errors)
#endif
    {
#ifdef _OPENMP
        std::mt19937 rng(42 + omp_get_thread_num());
#else
        std::mt19937 rng(44);
#endif
        std::uniform_int_distribution<int> sym_dist(0, _GF_ - 1);
        std::normal_distribution<double> noise(0, sigma);

        double y[CHIPS_PER_SYMBOL];

#ifdef _OPENMP
#pragma omp for nowait
#endif
        for (int frame = 0; frame < NbMonteCarlo; frame++)
        {
            int frame_error = 0;
            uint16_t tx_symbol[N];
            double *llr = new double[N * _GF_];

            for (int sym_idx = 0; sym_idx < N; sym_idx++)
            {
                tx_symbol[sym_idx] = sym_dist(rng);

                const double *rotated_seq = &base_seq[_GF_ - tx_symbol[sym_idx]];
                for (int i = 0; i < CHIPS_PER_SYMBOL; i++)
                {
                    y[i] = rotated_seq[i] + noise(rng);
                }
                llr_calc.calculate(y, &llr[sym_idx * _GF_]);
            }
            // for (int sym_idx = 0; sym_idx < N; sym_idx++)
            // {
            //     int detected = 0;
            //     for (int i = 1; i < _GF_; i++)
            //     {
            //         if (llr[sym_idx * _GF_ + i] < llr[sym_idx * _GF_ + detected])
            //             detected = i;
            //     }
            //     if (detected != tx_symbol[sym_idx])
            //     {
            //         frame_error = 1;
            //         break;
            //     }
            // }
            delete[] llr;

            frame_errors += frame_error;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double sec = std::chrono::duration<double>(end - start).count();

    std::cout << "CCSK FER Simulation Results:\n";
    std::cout << "GF(" << _GF_ << "), Symbols/Frame=" << N << ", Frames=" << NbMonteCarlo << "\n";
    std::cout << "Frame Errors: " << frame_errors << "/" << NbMonteCarlo << "\n";
    std::cout << "FER: " << (double)frame_errors / NbMonteCarlo << "\n";
    std::cout << "Time: " << sec << " sec\n";
    std::cout << "Throughput: " << NbMonteCarlo / sec << " fps\n";

    return 0;
}