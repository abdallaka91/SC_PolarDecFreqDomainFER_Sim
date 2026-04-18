#pragma once
#include "aff3ct_randn_gen/fast_noise_simple.hpp"
#include "ccsk_llr.hpp"
#include "ccsk_llr_float.hpp" 
#include "simul_parameters.hpp"
#include <array>
#include <cmath>
#include <memory>
#include <random>
#include <stdexcept>
#include <vector>


class CCSK_Channel
{
public:
	CCSK_Channel(){};
	virtual ~CCSK_Channel(){};
	virtual void    llr_to_probability     (double* llr_values, int num_symbols) = 0;
	virtual float*  llr_to_probability_fast(double* llr_values, int num_symbols, int thread_id = 0) = 0;
	virtual double* simulate_frame         (const uint16_t *tx_symbol,        int thread_id = 0) = 0;
	virtual void    generate_random_symbols(      uint16_t *tx_symbol, int K, int thread_id = 0)  = 0;
};


// Configuration as template parameters
template <int Tgf, int N = 1024, int MAX_LLR_1000 = SimulationParams::MAX_LLR_1000, int LLR_QUANT_BITS = SimulationParams::LLR_QUANT_BITS>
class CCSK_Simulator : public CCSK_Channel
{
	// Compile-time validation
	static_assert(N > 0, "N must be positive");
	static_assert((Tgf & (Tgf - 1)) == 0, "__GF__ must be power of 2");
	static_assert(LLR_QUANT_BITS > 0 && LLR_QUANT_BITS <= 16,
				  "LLR_QUANT_BITS must be 1-16");

	static constexpr double MAX_LLR_VALUE = static_cast<double>(MAX_LLR_1000) / 1000;

	static constexpr int CHIPS_PER_SYMBOL = Tgf;
	static constexpr int LUT_SIZE = 1 << LLR_QUANT_BITS; // 4096 for 12 bits
	static constexpr double LUT_fcat = (LUT_SIZE - 1) / MAX_LLR_VALUE;

	// COMPILE-TIME LOOKUP TABLE
	void create_exp_lut(std::array<float, LUT_SIZE>::iterator start, std::array<float, LUT_SIZE>::iterator stop)
	{
		constexpr double step = MAX_LLR_VALUE / (LUT_SIZE - 1);

		for (auto it = start; it < stop; it++)
		{
			double llr = std::distance(start, it) * step;
			if (llr > 150.0)
			{
				*it = 0.0f;
			}
			else
			{
				*it = static_cast<float>(std::exp(-llr));
			}
		}
	}

	std::array<float, LUT_SIZE> exp_neg_lut;

	const std::array<double, 2 * Tgf> base_seq;

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

		ThreadResources(double noise_sigma, int seed)
			: rng(seed),
			  noise_gen(noise_sigma, seed),
			  sym_dist(0, Tgf - 1),
			  llr_buffer(N * Tgf),
			  noise_buffer(N * CHIPS_PER_SYMBOL),
			  y_buffer(CHIPS_PER_SYMBOL),
			  prob_buffer(N * Tgf)
		{
		}
	};

//#define _DOUBLE_

	std::vector<std::unique_ptr<ThreadResources>> thread_resources;
#ifdef _DOUBLE_
	std::unique_ptr<CCSK_LLR<CHIPS_PER_SYMBOL>> llr_calc;
#else
	std::vector< CCSK_LLR_float<CHIPS_PER_SYMBOL>* > llr_calc;
//	std::unique_ptr< CCSK_LLR_float<CHIPS_PER_SYMBOL> > llr_calc;
#endif
  public:
	CCSK_Simulator(double noise_sigma, double llr_sigma, int max_threads = 1)
#ifdef _DOUBLE_
		: base_seq(get_base_seq_float<Tgf>()),
		  llr_calc(std::make_unique<CCSK_LLR<CHIPS_PER_SYMBOL>>(llr_sigma))
#else
		: base_seq(get_base_seq_float<Tgf>())
//		  llr_calc(std::make_unique<CCSK_LLR_float<CHIPS_PER_SYMBOL>>(llr_sigma))
#endif
	{
#ifndef _DOUBLE_
		for(int t = 0; t < max_threads; t += 1)
		  llr_calc.push_back( new CCSK_LLR_float<CHIPS_PER_SYMBOL>(llr_sigma) );
#endif

		create_exp_lut(exp_neg_lut.begin(), exp_neg_lut.end());
		thread_resources.reserve(max_threads);
		for (int i = 0; i < max_threads; i++)
		{
			thread_resources.push_back( std::make_unique<ThreadResources>(noise_sigma, 42 + i) );
		}
	}

  private:
	// Quantize LLR value to LUT index (compile-time friendly)
	int quantize_llr(double llr)
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
	float fast_exp_neg(double llr)
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
			const double* rotated_seq  = &base_seq [Tgf - tx_symbol[sym_idx]];
			const float*  symbol_noise = &noise_buf[sym_idx * CHIPS_PER_SYMBOL];

			for (int i = 0; i < CHIPS_PER_SYMBOL; i++)
			{
				y[i] = rotated_seq[i] + static_cast<double>(symbol_noise[i]);
			}

#ifdef _DOUBLE_
			llr_calc->calculate(y, &llr_output[sym_idx * Tgf]);
#else
			llr_calc[thread_id]->calculate(y, &llr_output[sym_idx * Tgf]);
#endif
		}

		return llr_output;
	}
#if 0
	void llr_to_probability(double *llr_values, int num_symbols)
	{
		for (int sym_idx = 0; sym_idx < num_symbols; sym_idx++)
		{
			double *symbol_llr = &llr_values[sym_idx * Tgf];
			double sum_exp = 0.0;

			for (int j = 0; j < Tgf; j++)
			{
				symbol_llr[j] = exp(-symbol_llr[j]);
				sum_exp += symbol_llr[j];
			}

			for (int j = 0; j < Tgf; j++)
			{
				symbol_llr[j] /= sum_exp;
			}
		}
	}
#else
	void llr_to_probability(double* llr_values, int num_symbols)
	{
		float buffer[Tgf];
		for (int sym_idx = 0; sym_idx < num_symbols; sym_idx++)
		{
			//
			//////////////////////////////////////////////////
			//
			for (int j = 0; j < Tgf; j++)
			{
				buffer[j] = llr_values[sym_idx * Tgf + j];
			}
			//
			//////////////////////////////////////////////////
			//
			for (int j = 0; j < Tgf; j++)
			{
				buffer[j] = exp(-buffer[j]);
			}
			//
			//////////////////////////////////////////////////
			//
			float sum_exp = 0.f;
			for (int j = 0; j < Tgf; j++)
			{
				sum_exp += buffer[j];
			}
			//
			//////////////////////////////////////////////////
			//
			for (int j = 0; j < Tgf; j++)
			{
				llr_values[sym_idx * Tgf + j] = (buffer[j] / sum_exp);
			}
			//
			//////////////////////////////////////////////////
			//
		}
	}
#endif

	// FAST VERSION using lookup table (output in thread's prob_buffer)
	float *llr_to_probability_fast(double *llr_values, int num_symbols, int thread_id = 0)
	{
		auto &thread_res = get_thread_res(thread_id);
		float *prob_output = thread_res.prob_buffer.data();

		for (int sym_idx = 0; sym_idx < num_symbols; sym_idx++)
		{
			double *symbol_llr = &llr_values[sym_idx * Tgf];
			float *symbol_prob = &prob_output[sym_idx * Tgf];
			float sum_exp = 0.0f;

			// Convert LLR to probability using LUT
			for (int j = 0; j < Tgf; j++)
			{
				symbol_prob[j] = fast_exp_neg(symbol_llr[j]);
				sum_exp += symbol_prob[j];
			}

			// Normalize to sum = 1
			float inv_sum = 1.0f / sum_exp;
			for (int j = 0; j < Tgf; j++)
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
