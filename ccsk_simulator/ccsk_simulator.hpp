#pragma once
#include "ccsk_llr.hpp"
#include "aff3ct_randn_gen/fast_noise_simple.hpp"
#include <random>
#include <memory>
#include <vector>

template <int __GF__, int N = 1024>
class CCSK_Simulator
{
    static_assert(N > 0, "N must be positive");
    static_assert((__GF__ & (__GF__ - 1)) == 0, "__GF__ must be power of 2");

    static constexpr int CHIPS_PER_SYMBOL = __GF__;

    // Precomputed base sequence (BPSK ±1)
    const std::array<double, 2 * __GF__> base_seq;

    // Thread-local resources
    struct ThreadResources
    {
        std::mt19937 rng;
        FastGaussianNoise<float> noise_gen;
        std::uniform_int_distribution<int> sym_dist;

        ThreadResources(double real_sigma, int seed)
            : rng(seed), noise_gen(real_sigma, seed), sym_dist(0, __GF__ - 1) {}
    };

    // Store per-thread resources
    std::vector<std::unique_ptr<ThreadResources>> thread_resources;

    // LLR calculator (uses fake_sigma)
    std::unique_ptr<CCSK_LLR<CHIPS_PER_SYMBOL>> llr_calc;

public:
    // Constructor: precomputes base sequence
    CCSK_Simulator(double real_sigma, double fake_sigma, int max_threads = 1)
        : base_seq(get_base_seq_float<__GF__>()), llr_calc(std::make_unique<CCSK_LLR<CHIPS_PER_SYMBOL>>(fake_sigma))
    {
        // Initialize thread resources
        thread_resources.reserve(max_threads);
        for (int i = 0; i < max_threads; i++)
        {
            thread_resources.push_back(
                std::make_unique<ThreadResources>(real_sigma, 42 + i));
        }
    }

    // Main simulation function
    double *simulate_frame(const uint16_t tx_symbol[N], int thread_id = 0)
    {
        auto &thread_res = get_thread_res(thread_id);

        // Allocate output: LLRs for all symbols [N × __GF__]
        double *llr_output = new double[N * __GF__];

        // Temporary buffers
        double y[CHIPS_PER_SYMBOL];
        float *noise_buf = new float[N * CHIPS_PER_SYMBOL];

        // Batch generate all noise for this frame
        thread_res.noise_gen.generate(noise_buf, N * CHIPS_PER_SYMBOL);

        // Process each symbol
        for (int sym_idx = 0; sym_idx < N; sym_idx++)
        {
            // Get rotated sequence for this symbol
            const double *rotated_seq = &base_seq[__GF__ - tx_symbol[sym_idx]];

            // Add signal to noise
            const float *symbol_noise = &noise_buf[sym_idx * CHIPS_PER_SYMBOL];
            for (int i = 0; i < CHIPS_PER_SYMBOL; i++)
            {
                y[i] = rotated_seq[i] + (double)symbol_noise[i];
            }

            // Calculate LLRs for this symbol
            llr_calc->calculate(y, &llr_output[sym_idx * __GF__]);
        }

        delete[] noise_buf;
        return llr_output; // Caller must delete[]
    }

    template <int GF>
    void llr_to_probability(double *llr_values, int num_symbols)
    {
        for (int sym_idx = 0; sym_idx < num_symbols; sym_idx++)
        {
            double *symbol_llr = &llr_values[sym_idx * GF];
            double sum_exp = 0.0;

            for (int j = 0; j < GF; j++)
            {
                symbol_llr[j] = exp(-symbol_llr[j]);
                sum_exp += symbol_llr[j];
            }

            for (int j = 0; j < GF; j++)
            {
                symbol_llr[j] /= sum_exp;
            }
        }
    }

    // Generate random symbols
    void generate_random_symbols(uint16_t tx_symbol[N], int thread_id = 0)
    {
        auto &thread_res = get_thread_res(thread_id);
        for (int i = 0; i < N; i++)
        {
            tx_symbol[i] = thread_res.sym_dist(thread_res.rng);
        }
    }

private:
    // Helper to get thread resources (with bounds checking)
    ThreadResources &get_thread_res(int thread_id)
    {
        if (thread_id >= (int)thread_resources.size())
        {
            throw std::runtime_error("Thread ID exceeds allocated resources");
        }
        return *thread_resources[thread_id];
    }

    // Function to precompute base sequence (same as before)
    template <int GF>
    static constexpr auto get_base_seq_float()
    {
        constexpr auto get_seq = []() -> const auto &
        {
            if constexpr (GF == 64)
                return CCSKSequences::BASE_SEQ_64;
            else if constexpr (GF == 128)
                return CCSKSequences::BASE_SEQ_128;
            else if constexpr (GF == 256)
                return CCSKSequences::BASE_SEQ_256;
            else if constexpr (GF == 512)
                return CCSKSequences::BASE_SEQ_512;
            else if constexpr (GF == 1024)
                return CCSKSequences::BASE_SEQ_1024;
        };

        constexpr auto &int_seq = get_seq();
        std::array<double, 2 * GF> float_seq{};

        for (int i = 0; i < GF; i++)
        {
            float_seq[i] = -static_cast<double>(int_seq[i]) * 2 + 1;
            float_seq[i + GF] = float_seq[i];
        }
        return float_seq;
    }
};