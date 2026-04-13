// #define find_llr_rang // disable it

#include "init.h"
#include "struct.h"
#include "tools.h"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <omp.h>
#include <random>
#include <signal.h>
#include <sstream>
#include <string>
#include <vector>

#include "./ccsk_simulator/ccsk_simulator.hpp"
#include "./ccsk_simulator/simul_parameters.hpp"
#include <chrono>
#include <iostream>

#include "./include/custom_types.hpp"
#include "./include/encoder_1.hpp"
#include "./include/loader_so.hpp"

using namespace PoAwN::structures;
using namespace PoAwN::tools;
using namespace PoAwN::init;
using std::array;
using std::cout;
using std::endl;
using std::stod;
using std::stoi;
using std::string;
using std::vector;

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

std::string format_FER(double FER_value, int width = 10)
{
	std::ostringstream oss;
	if (FER_value < 0.0001)
		oss << std::scientific << std::setprecision(3) << FER_value;
	else
		oss << std::fixed << std::setprecision(10) << FER_value;
	std::string s = oss.str();
	if ((int)s.length() < width)
		s = std::string(width - s.length(), ' ') + s;

	if ((int)s.length() > width)
		s = s.substr(0, width);

	return s;
}

void append_results_to_file1(const std::string &dec, int GFx, int Nx, int Kx, double SNR, unsigned long nb_err, uint64_t nb_gen_frame, float llr_sigma, int seconds, int nbits = -1)
{
	fs::path dir = "results";

	std::error_code ec;
	if (!fs::exists(dir))
	{
		if (!fs::create_directories(dir, ec))
		{
			std::cerr << "Error creating directory " << dir << ": " << ec.message()
					  << "\n";
			return;
		}
	}

	fs::path filename =
		dir / ("GF" + std::to_string(GFx) + "_N" + std::to_string(Nx) + "_K" +
			   std::to_string(Kx) + "_" + dec.c_str() + ".txt");

	FILE *fou = fopen(filename.c_str(), "a");

	if (fou == nullptr)
	{
		std::cerr << "Error opening file " << filename << " for appending.\n";
		return;
	}

	double FER_value = (nb_gen_frame == 0) ? 0.0 : static_cast<double>(nb_err) / nb_gen_frame;
	std::string FER_str = format_FER(FER_value, 11); // 11 chars wide

	fprintf(fou, "%+7.3f   %s   %6d   %12" PRIu64, SNR, FER_str.c_str(), nb_err, nb_gen_frame);
	fprintf(fou, " %6d ", seconds);
	if ((nbits > 0))
	{
		fprintf(fou, "    %5d", nbits);
		fprintf(fou, "    %5.3f", llr_sigma);
	}

	if (dec.rfind("dec4", 0) == 0) // prefix check
	{
#ifdef REP_DISABLED
		fprintf(fou, "   REP_DISABLED");
#endif
#ifdef SPC_DISABLED
		fprintf(fou, "   SPC_DISABLED");
#endif
	}
	// ----------------------------------

	fprintf(fou, "\n");
	fclose(fou);
}

#define STR(S) #S

#define EVAL(x) STR(x)
//
//
//
//
static bool force_quit = false;
void intHandler(int dummy)
{
	printf("\n#(DD) CTRL+C was detected\n");
	force_quit = true;
}
//
//
//
//
int main(int argc, char *argv[])
{

#ifdef __AVX512BW__
	printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation "
		   "program (AVX512 version)\n");
#elif __AVX2__
	printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation "
		   "program (AVX2 version)\n");
#else
	printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation "
		   "program (ARM NEON version)\n");
#endif
	printf("#(II) + developped by Abdallah ABDALLAH in 2025...\n");
	printf("#(II) +        and by Camille MONIERE   in 2025...\n");
	printf("#(II) +        and by Bertrand LE GAL   in 2025...\n");
	printf("#(II)\n");
	printf("#(II) Binary generated : %s - %s\n", __DATE__, __TIME__);

	signal(SIGINT, intHandler);

#ifdef __APPLE__
	bool ok = loader_so::open("libNbScFFTdec.dylib");
#else
	bool ok = loader_so::open("libNbScFFTdec.so");
#endif

	if (ok)
		printf("#(II) + Decoder library was loaded successfully...\n");
	else
		printf("#(EE) + Error during the library loading...\n");

	int num_threads = omp_get_max_threads();

	////////////////////////////

	softdata_t offset;

	////////////////////////////

	std::string dec_type = "";
	uint64_t NbMonteCarlo = 10000000000;
	float EbN0 = -1000.f;
	uint16_t q = 0;
	uint16_t p = 0;
	uint16_t N = 0;
	uint16_t n = 0;
	uint16_t K = 0;
	int FER_STOP = 100;
	uint16_t frozen_val = 0;
	int nbits = -1;
	float llr_sigma = -1.f;
	/////////////////////////////////////////////////////////////////

	for (int i = 1; i < argc; i++)
	{
		if (std::string(argv[i]) == "-snr")
		{
			EbN0 = stod(std::string(argv[i + 1]));
			i += 1;
		}
		else if (std::string(argv[i]) == "-q")
		{
			q = stoi(std::string(argv[i + 1]));
			p = log2(q);
			i += 1;
		}
		else if (std::string(argv[i]) == "-N")
		{
			N = stoi(std::string(argv[i + 1]));
			n = log2(N);
			i += 1;
		}
		else if (std::string(argv[i]) == "-K")
		{
			K = stoi(std::string(argv[i + 1]));
			i += 1;
		}
		else if (std::string(argv[i]) == "-dec")
		{
			dec_type = std::string(argv[i + 1]);
			i += 1;
		}
		else if (std::string(argv[i]) == "-cw")
		{
			NbMonteCarlo = std::stoull(argv[i + 1]);
			i += 1;
		}
		else if (std::string(argv[i]) == "-thread")
		{
			num_threads = std::atoi(argv[i + 1]);
			omp_set_num_threads(num_threads);
			i += 1;
		}
		else if (std::string(argv[i]) == "-threads")
		{
			num_threads = std::atoi(argv[i + 1]);
			omp_set_num_threads(num_threads);
			i += 1;
		}
		else if (std::string(argv[i]) == "-cores")
		{
			num_threads = std::atoi(argv[i + 1]);
			omp_set_num_threads(num_threads);
			i += 1;
		}
		else if (std::string(argv[i]) == "-errors")
		{
			FER_STOP = std::atoi(argv[i + 1]);
			i += 1;
		}
		else if (std::string(argv[i]) == "-nbits")
		{
			nbits = std::atoi(argv[i + 1]);
			i += 1;
		}
		else if (std::string(argv[i]) == "-llr_sigma")
		{
			llr_sigma = std::stof(argv[i + 1]);
			i += 1;
		}
		else if (std::string(argv[i]) == "-nbits_dec")
		{
			nbits = std::atoi(argv[i + 1]);
			i += 1;
		}
		else
		{
			printf("(EE) Error during CLI parsing\n");
			printf("(EE) argument = [%s]\n", argv[i]);
			exit(EXIT_FAILURE);
		}
	}

	/////////////////////////////////////////////////////////////////

	if (EbN0 == -1000.f)
	{
		printf("(EE) Error during CLI parsing\n");
		printf("(EE) missing [-snr] option\n");
		exit(EXIT_FAILURE);
	}
	if (q == 0)
	{
		printf("(EE) Error during CLI parsing\n");
		printf("(EE) missing [-q] option\n");
		exit(EXIT_FAILURE);
	}
	if (N == 0)
	{
		printf("(EE) Error during CLI parsing\n");
		printf("(EE) missing [-q] option\n");
		exit(EXIT_FAILURE);
	}
	if (K == 0)
	{
		printf("(EE) Error during CLI parsing\n");
		printf("(EE) missing [-q] option\n");
		exit(EXIT_FAILURE);
	}
	if (dec_type == "")
	{
		printf("(EE) Error during CLI parsing\n");
		printf("(EE) missing [-dec] option\n");
		exit(EXIT_FAILURE);
	}

	/////////////////////////////////////////////////////////////////

	std::cout << "(DD) NbMonteCarlo : " << NbMonteCarlo << std::endl;
	std::cout << "(DD) EbN0         : " << EbN0 << std::endl;
	std::cout << "(DD) q            : " << q << std::endl;
	std::cout << "(DD) p            : " << p << std::endl;
	std::cout << "(DD) N            : " << N << std::endl;
	std::cout << "(DD) K            : " << K << std::endl;
	std::cout << "(DD) dec_type     : " << dec_type << std::endl;
	std::cout << "(DD) FER_STOP     : " << FER_STOP << std::endl;
	std::cout << "(DD) num_threads  : " << num_threads << std::endl;

	/////////////////////////////////////////////////////////////////

	std::transform(dec_type.begin(), dec_type.end(), dec_type.begin(), ::tolower);

	if (dec_type == "dec1-int" || dec_type == "naive-int")
	{
		if (nbits < 1)
		{
			printf("(EE) missing [-nbits] option for dec1_integer\n");
			exit(EXIT_FAILURE);
		}
		dec_type = "naive-int{" + std::to_string(nbits) + "}";
	}
	else if (dec_type == "dec4-int" || dec_type == "pruned-int")
	{
		if (nbits < 1)
		{
			printf("(EE) missing [-nbits] option for dec4_integer\n");
			exit(EXIT_FAILURE);
		}
		dec_type = "pruned-int{" + std::to_string(nbits) + "}";
	}
	else if (dec_type == "dec4" || dec_type == "pruned")
	{
		dec_type = "pruned";
	}
	// else if ( dec_type == "dec4_integer" || dec_type == "pruned_integer") {
	//     if (nbits < 1) {
	//         printf("(EE) missing [-nbits] option for dec4_integer\n");
	//         exit(EXIT_FAILURE);
	//     }
	//     dec_type = "pruned-int{" + std::to_string(nbits) + "}";
	// }

	base_code_t code_param(N, K, n, q, p, frozen_val);
	code_param.sig_mod = "CCSK_BIN";

	int gf_rand_SEED = 0;
	float nse_rand_SEED = 1.2544;
	bool repeatable_randgen = 0;

	table_GF table;

	cout << "(II) Loading code_param [START]" << endl;
	LoadCode(code_param, EbN0, "./matrices/");
	cout << "(II) Loading code_param [END OK]" << endl;

	cout << EVAL(FWHT) " and " EVAL(FWHT_NORM) " are used for FWHT operations."
		 << endl;

	cout << "Simulation starts..." << endl;

	int frozen_symbols[N];
	for (int i = 0; i < N; i += 1)
		frozen_symbols[i] = true;
	for (int i = 0; i < K; i += 1)
		frozen_symbols[code_param.reliab_sequence[i]] = false;

	printf("(II) Reliability sequence:\n");
	printf("(II) ");
	for (int i = 0; i < K; i += 1)
	{
		printf("%2d ", code_param.reliab_sequence[i]);
	}
	printf("\n");

	const auto s_start = std::chrono::system_clock::now();

	float noise_sigma = sqrt(1.0 / (pow(10, EbN0 / 10.0)));
	if (llr_sigma < 0.f)
		llr_sigma = noise_sigma;

	CCSK_Simulator<_GF_, _N_> simulator(noise_sigma, llr_sigma, num_threads);

	std::atomic<uint64_t> frame_errors(0);
	std::atomic<uint64_t> frames_simulated(0);

	uint64_t FER_out = 0, gen_frames_out = 0;
	std::atomic<uint64_t> global_counter(0);
	std::atomic<uint64_t> FER(0);
	std::atomic<bool> stop(false);
	std::atomic<double> global_llr_min {std::numeric_limits<double>::max()};
	std::atomic<double> global_llr_max {std::numeric_limits<double>::lowest()};

	auto start = std::chrono::high_resolution_clock::now();
	auto watchdod = std::chrono::high_resolution_clock::now();

#ifndef NDEBUG
#pragma omp parallel num_threads(1)
	printf("Debug build - single thread mode\n");
#else
#pragma omp parallel
#endif
	{
		int thread_id = omp_get_thread_num();
		uint16_t K_symb[K];
		uint16_t u_symb[N];
		uint16_t r_symb[N];
		std::vector<float> llrs_n(N * q);
		std::vector<uint16_t> decoded_n(N);

		// Initialize decoder
		decoder *dec = nullptr;
		dec = loader_so::allocate_dec(dec_type, N, q, frozen_symbols);
		// #pragma omp single
		while (true)
		{
			bool succ_dec = true;

			// Generate symbols for THIS frame
			simulator.generate_random_symbols(K_symb, K, thread_id);
			for (int u = 0; u < K; u++)
				u_symb[code_param.reliab_sequence[u]] = K_symb[u];
			for (int u = K; u < N; u++)
				u_symb[code_param.reliab_sequence[u]] = 0;

			for (int u = 0; u < N; u++) // on conserve une copie des données afin d'utiliser
				r_symb[u] = u_symb[u];	// le mode génie dans la simulation

			polar_encode<_N_>(u_symb);

			// Simulate CCSK transmission
			double *llr_values = simulator.simulate_frame(u_symb, thread_id);

#ifdef find_llr_rang
			double local_min = llr_values[0];
			double local_max = llr_values[0];

			for (int i = 1; i < N * _GF_; i++)
			{
				if (llr_values[i] < local_min)
					local_min = llr_values[i];
				if (llr_values[i] > local_max)
					local_max = llr_values[i];
			}

			// update global min
			double current_min = global_llr_min.load();
			while (local_min < current_min &&
				   !global_llr_min.compare_exchange_weak(current_min, local_min))
			{
			}

			// update global max
			double current_max = global_llr_max.load();
			while (local_max > current_max &&
				   !global_llr_max.compare_exchange_weak(current_max, local_max))
			{
			}
#endif

			if constexpr (SimulationParams::method == SimulationParams::LLRMethod::EXP)
			{
				// Original method: exp() calls
				simulator.llr_to_probability<_GF_>(llr_values, N);
				for (int i = 0; i < N; i++)
				{
					for (int j = 0; j < q; j++)
					{
						llrs_n[i * q + j] = static_cast<float>(llr_values[i * q + j]);
					}
				}
			}

			// #ifdef find_llr_rang
			// 			for (int i = 0; i < N; i++)
			// 			{
			// 				for (int j = 0; j < _GF_; j++)
			// 				{
			// 					double val = llrs_n[i].value[j];
			// 					// llrs_n[i].value[j] = val;

			// 					// Update min
			// 					double current_min = global_llr_min.load();
			// 					while (val < current_min &&
			// 						   !global_llr_min.compare_exchange_weak(current_min, val))
			// 					{
			// 					}

			// 					// Update max
			// 					double current_max = global_llr_max.load();
			// 					while (val > current_max &&
			// 						   !global_llr_max.compare_exchange_weak(current_max, val))
			// 					{
			// 					}
			// 				}
			// 			}
			// #endif

			// Decode

			else if constexpr (SimulationParams::method == SimulationParams::LLRMethod::FAST_LUT)
			{
				// Fast method: lookup table
				float *probabilities = simulator.llr_to_probability_fast<_GF_>(llr_values, N, thread_id);
				for (int i = 0; i < N; i++)
				{
					for (int j = 0; j < q; j++)
					{
						llrs_n[i * q + j] = probabilities[i * q + j];
					}
				}
			}

			dec->setResult(r_symb);
			dec->execute(llrs_n.data(), decoded_n.data());

			// Check for errors
			for (uint16_t i = 0; i < code_param.K; i++)
			{
				if (K_symb[i] != decoded_n[code_param.reliab_sequence[i]])
				{
					succ_dec = false;
					break;
				}
			}

			global_counter.fetch_add(1);
			uint64_t succ_now = global_counter.load() - FER.load();
			if (!succ_dec)
			{
				FER.fetch_add(1);
			}
			succ_now = global_counter.load() - FER.load();

			//
			// Plus d'openmp critical car on filtre sur thread 1
			//
			if (thread_id == 0)
			{
				auto curr = std::chrono::high_resolution_clock::now();
				auto sec = std::chrono::duration<double>(curr - watchdod).count();
				if (sec >= 1)
				{
					uint64_t local_success = global_counter.load() - FER.load();
					if ((global_counter.load() >= NbMonteCarlo) ||
						(FER.load() >= FER_STOP))
						stop.store(true);
					FER_out = FER.load();
					gen_frames_out = global_counter.load();

					double FER_ratio = (double)FER_out / gen_frames_out;
					std::ostringstream oss;
					//					FER_ratio < 0.0001 ? oss << std::scientific << std::setprecision(3) << std::setw(10) << FER_ratio : oss << std::fixed << std::setprecision(6) << std::setw(10) << FER_ratio;
					oss << std::scientific << std::setprecision(3) << std::setw(10) << FER_ratio;
					std::string FER_str = oss.str();

					auto end = std::chrono::high_resolution_clock::now();
					double sec = std::chrono::duration<double>(end - start).count();

					std::cout << "\r" << std::fixed << std::setprecision(1) << EbN0
							  << " dB, FER = " << std::setw(8) << FER_out
							  << "/" << std::setw(8) << gen_frames_out
							  << " = " << FER_str;

					printf(" | %6d sec. | ", (int)sec);

					if (FER_out == 0)
						printf("------");
					else
					{
						double tps_p_err = (double)sec / (double)FER_out;
						double restant = (FER_STOP - FER_out) >= 0 ? (FER_STOP - FER_out) : 0;
						double tps_rest = (double)(restant)*tps_p_err;
						//
						//						printf("[%6d, %6d, %f]  ", FER_STOP, FER_out, tps_p_err);
						//
						printf("%6d sec. | ", (int)tps_rest);
					}
					std::cout << std::flush << "\r";
					watchdod = std::chrono::high_resolution_clock::now();
				}
			}
			if (stop.load())
				break;

			if (force_quit == true)
				break;
		}
		delete dec;
	}

	auto end = std::chrono::high_resolution_clock::now();
	double sec = std::chrono::duration<double>(end - start).count();

	double FER_ratio = (double)FER_out / gen_frames_out;
	std::ostringstream oss;
	//	FER_ratio < 0.0001 ? oss << std::scientific << std::setprecision(3) << std::setw(10) << FER_ratio : oss << std::fixed << std::setprecision(6) << std::setw(10) << FER_ratio;
	oss << std::scientific << std::setprecision(3) << std::setw(10) << FER_ratio;

	std::string FER_str = oss.str();

	std::cout << "\rSNR: " << std::fixed << std::setprecision(1) << EbN0
			  << " dB, FER = " << std::setw(8) << FER_out
			  << "/" << std::setw(8) << gen_frames_out
			  << " = " << FER_str
			  << std::flush;

	append_results_to_file1(dec_type, q, N, K, EbN0, FER_out, gen_frames_out, llr_sigma, (int)sec, nbits);

	// Final results
std::cout << "\nPolar Code: N=" << N << ", K=" << K << ", GF=" << q << std::endl;
	std::cout << "Decoder: " << dec_type << std::endl;
	std::cout << "Eb/N0: " << EbN0 << " dB, noise_sigma: " << noise_sigma << std::endl;
	std::cout << "Actual frames: " << gen_frames_out << std::endl;
	std::cout << "Time: " << sec << " seconds" << std::endl;
	std::cout << "Throughput: " << gen_frames_out / sec << " fps" << std::endl;
std::cout << "Throughput info: " << (gen_frames_out * K * p) / sec / 1e6 << " Mbps" << std::endl;
#ifdef find_llr_rang
	std::cout << "global_llr_max: " << global_llr_max << std::endl;
	std::cout << "global_llr_min: " << global_llr_min << std::endl;
#endif
}
