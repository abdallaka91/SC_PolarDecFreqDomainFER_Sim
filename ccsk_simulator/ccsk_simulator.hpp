#pragma once
#include "aff3ct_randn_gen/fast_noise_simple.hpp"
#include "ccsk_llr.hpp"
#include "simul_parameters.hpp"
#include <array>
#include <cmath>
#include <memory>
#include <random>
#include <stdexcept>
#include <vector>

// Configuration as template parameters
template <int __GF__, int N = 1024, int MAX_LLR_1000 = SimulationParams::MAX_LLR_1000, int LLR_QUANT_BITS = SimulationParams::LLR_QUANT_BITS>
class CCSK_Simulator
{
	// Compile-time validation
	static_assert(N > 0, "N must be positive");
	static_assert((__GF__ & (__GF__ - 1)) == 0, "__GF__ must be power of 2");
	static_assert(LLR_QUANT_BITS > 0 && LLR_QUANT_BITS <= 16,
				  "LLR_QUANT_BITS must be 1-16");

	static constexpr double MAX_LLR_VALUE = static_cast<double>(MAX_LLR_1000) / 1000;

	static constexpr int CHIPS_PER_SYMBOL = __GF__;
	static constexpr int LUT_SIZE = 1 << LLR_QUANT_BITS; // 4096 for 12 bits
	static constexpr double LUT_fcat = (LUT_SIZE - 1) / MAX_LLR_VALUE;

	// COMPILE-TIME LOOKUP TABLE
	static constexpr std::array<float, LUT_SIZE> create_exp_lut()
	{
		std::array<float, LUT_SIZE> lut {};
		constexpr double step = MAX_LLR_VALUE / (LUT_SIZE - 1);

		for (int i = 0; i < LUT_SIZE; i++)
		{
			double llr = i * step;
			if (llr > 150.0)
			{
				lut[i] = 0.0f;
			}
			else
			{
				lut[i] = static_cast<float>(std::exp(-llr));
			}
		}
		return lut;
	}

	static constexpr std::array<float, LUT_SIZE> exp_neg_lut = create_exp_lut();

	const std::array<double, 2 * __GF__> base_seq;

	struct ThreadResources
	{
		std::mt19937 rng;
		FastGaussianNoise<float> noise_gen;
		std::uniform_int_distribution<int> sym_dist;

		// Reusable buffers
		std::vector<double> llr_buffer;
		std::vector<float> noise_buffer;
		std::vector<double> y_buffer;
		std::vector<float> prob_buffer;

		ThreadResources(double real_sigma, int seed)
			: rng(seed),
			  noise_gen(real_sigma, seed),
			  sym_dist(0, __GF__ - 1),
			  llr_buffer(N * __GF__),
			  noise_buffer(N * CHIPS_PER_SYMBOL),
			  y_buffer(CHIPS_PER_SYMBOL),
			  prob_buffer(N * __GF__)
		{
		}
	};

	std::vector<std::unique_ptr<ThreadResources>> thread_resources;
	std::unique_ptr<CCSK_LLR<CHIPS_PER_SYMBOL>> llr_calc;

  public:
	CCSK_Simulator(double real_sigma, double fake_sigma, int max_threads = 1)
		: base_seq(get_base_seq_float<__GF__>()),
		  llr_calc(std::make_unique<CCSK_LLR<CHIPS_PER_SYMBOL>>(fake_sigma))
	{
		thread_resources.reserve(max_threads);
		for (int i = 0; i < max_threads; i++)
		{
			thread_resources.push_back(
				std::make_unique<ThreadResources>(real_sigma, 42 + i));
		}
	}

  private:
	// Quantize LLR value to LUT index (compile-time friendly)
	static constexpr int quantize_llr(double llr)
	{
		if (llr <= 0.0)
			return 0;
		if (llr >= MAX_LLR_VALUE)
			return LUT_SIZE - 1;

		// Compile-time computable if llr is constexpr
		double scaled = llr * LUT_fcat;
		return static_cast<int>(scaled + 0.5);
	}

	// Fast exp(-llr) using compile-time LUT
	static float fast_exp_neg(double llr)
	{
		return exp_neg_lut[quantize_llr(llr)];
	}

  public:
	double *simulate_frame(const uint16_t tx_symbol[N], int thread_id = 0)
	{
		auto &thread_res = get_thread_res(thread_id);

		double *llr_output = thread_res.llr_buffer.data();
		float *noise_buf = thread_res.noise_buffer.data();
		double *y = thread_res.y_buffer.data();

		thread_res.noise_gen.generate(noise_buf, N * CHIPS_PER_SYMBOL);

		for (int sym_idx = 0; sym_idx < N; sym_idx++)
		{
			const double *rotated_seq = &base_seq[__GF__ - tx_symbol[sym_idx]];
			const float *symbol_noise = &noise_buf[sym_idx * CHIPS_PER_SYMBOL];

			for (int i = 0; i < CHIPS_PER_SYMBOL; i++)
			{
				y[i] = rotated_seq[i] + static_cast<double>(symbol_noise[i]);
			}

			llr_calc->calculate(y, &llr_output[sym_idx * __GF__]);
		}

		return llr_output;
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

	// FAST VERSION using lookup table (output in thread's prob_buffer)
	template <int GF>
	float *llr_to_probability_fast(double *llr_values, int num_symbols, int thread_id = 0)
	{
		auto &thread_res = get_thread_res(thread_id);
		float *prob_output = thread_res.prob_buffer.data();

		for (int sym_idx = 0; sym_idx < num_symbols; sym_idx++)
		{
			double *symbol_llr = &llr_values[sym_idx * GF];
			float *symbol_prob = &prob_output[sym_idx * GF];
			float sum_exp = 0.0f;

			// Convert LLR to probability using LUT
			for (int j = 0; j < GF; j++)
			{
				symbol_prob[j] = fast_exp_neg(symbol_llr[j]);
				sum_exp += symbol_prob[j];
			}

			// Normalize to sum = 1
			float inv_sum = 1.0f / sum_exp;
			for (int j = 0; j < GF; j++)
			{
				symbol_prob[j] *= inv_sum;
			}
		}

		return prob_output;
	}

	// Configuration getters
	static constexpr double get_max_llr()
	{
		return MAX_LLR_VALUE;
	}
	static constexpr int get_llr_quant_bits()
	{
		return LLR_QUANT_BITS;
	}
	static constexpr int get_lut_size()
	{
		return LUT_SIZE;
	}

	void generate_random_symbols(uint16_t *tx_symbol, int K, int thread_id = 0)
	{
		auto &thread_res = get_thread_res(thread_id);
		for (int i = 0; i < K; i++)
		{
			tx_symbol[i] = static_cast<uint16_t>(thread_res.sym_dist(thread_res.rng));
		}
	}

  private:
	ThreadResources &get_thread_res(int thread_id)
	{
		if (thread_id >= static_cast<int>(thread_resources.size()))
		{
			throw std::runtime_error("Thread ID exceeds allocated resources");
		}
		return *thread_resources[thread_id];
	}

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
			else
				static_assert(GF == 64 || GF == 128 || GF == 256 ||
								  GF == 512 || GF == 1024,
							  "Unsupported GF size - add sequence");
		};

		constexpr auto &int_seq = get_seq();
		std::array<double, 2 * GF> float_seq {};

		for (int i = 0; i < GF; i++)
		{
			float_seq[i] = -static_cast<double>(int_seq[i]) * 2 + 1;
			float_seq[i + GF] = float_seq[i];
		}
		return float_seq;
	}
};