// #define find_llr_rang // disable it

#include "./include/code.hpp"
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
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <omp.h>

#include "./ccsk_simulator/ccsk_simulator.hpp"
#include "./ccsk_simulator/simul_parameters.hpp"
#include <chrono>
#include <iostream>

#include "./include/loader_so.hpp"
#include "./include/encoder_1.hpp"
#include "./include/custom_types.hpp"

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

void append_results_to_file1(const std::string &dec, int GFx, int Nx, int Kx, double SNR, unsigned long nb_err, uint64_t nb_gen_frame, float fake_sigma)
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
	if ((dec == "dec1_integer") || (dec == "dec4_integer"))
	{
		fprintf(fou, "    %5d", I_type::NBITS);
		fprintf(fou, "    %5.3f", fake_sigma);
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
    
#ifdef __APPLE__
    bool ok = loader_so::open("libNbScFFTdec.dylib");
#else
    bool ok = loader_so::open("libNbScFFTdec.so");
#endif
    
    if( ok )
        printf("#(II) + Decoder library was loaded successfully...\n");
    else
        printf("#(EE) + Error during the library loading...\n");

    int num_threads = omp_get_max_threads();

    ////////////////////////////

    softdata_t offset;

    ////////////////////////////

    std::string dec_type  = "";
    uint64_t NbMonteCarlo = 10000000000;
    float EbN0            = -1000.f;
    uint16_t q            = 0;
    uint16_t p            = 0;
    uint16_t N            = 0;
    uint16_t n            = 0;
    uint16_t K            = 0;
    int FER_STOP          = 100;
    uint16_t frozen_val   = 0;

    /////////////////////////////////////////////////////////////////

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-snr") {
            EbN0 = stod( std::string(argv[i+1]));
            i += 1;
        } else if (std::string(argv[i]) == "-q") {
            q = stoi(std::string(argv[i+1]));
            p = log2(q);
            i += 1;
        } else if (std::string(argv[i]) == "-N") {
            N = stoi(std::string(argv[i+1]));
            n = log2(N);
            i += 1;
        } else if (std::string(argv[i]) == "-K") {
            K = stoi(std::string(argv[i+1]));
            i += 1;
        } else if (std::string(argv[i]) == "-dec") {
            dec_type = std::string(argv[i+1]);
            i += 1;
        } else if (std::string(argv[i]) == "-cw") {
            NbMonteCarlo = std::stoull(argv[i+1]);
            i += 1;
        } else if (std::string(argv[i]) == "-thread") {
            num_threads = std::atoi(argv[i + 1]);
            omp_set_num_threads( num_threads );
            i += 1;
        } else if (std::string(argv[i]) == "-threads") {
            num_threads = std::atoi(argv[i + 1]);
            omp_set_num_threads( num_threads );
            i += 1;
        } else if (std::string(argv[i]) == "-cores") {
            num_threads = std::atoi(argv[i + 1]);
            omp_set_num_threads( num_threads );
            i += 1;
        } else if (std::string(argv[i]) == "-errors") {
            FER_STOP = std::atoi(argv[i + 1]);
            i += 1;
        }else{
            printf("(EE) Error during CLI parsing\n");
            printf("(EE) argument = [%s]\n", argv[i]);
            exit( EXIT_FAILURE );
        }
    }

    /////////////////////////////////////////////////////////////////

    if( EbN0 == -1000.f){
        printf("(EE) Error during CLI parsing\n");
        printf("(EE) missing [-snr] option\n");
        exit( EXIT_FAILURE );
    }
    if( q == 0){
        printf("(EE) Error during CLI parsing\n");
        printf("(EE) missing [-q] option\n");
        exit( EXIT_FAILURE );
    }
    if( N == 0){
        printf("(EE) Error during CLI parsing\n");
        printf("(EE) missing [-q] option\n");
        exit( EXIT_FAILURE );
    }
    if( K == 0){
        printf("(EE) Error during CLI parsing\n");
        printf("(EE) missing [-q] option\n");
        exit( EXIT_FAILURE );
    }
    if( dec_type == ""){
        printf("(EE) Error during CLI parsing\n");
        printf("(EE) missing [-dec] option\n");
        exit( EXIT_FAILURE );
    }
    if( N != _N_){
        printf("(EE) Error during CLI parsing\n");
        printf("(EE) N and _N_ values missmatched\n");
        exit( EXIT_FAILURE );
    }
    if( _GF_ != q){
        printf("(EE) Error during CLI parsing\n");
        printf("(EE) q and _GF_ values missmatched\n");
        exit( EXIT_FAILURE );
    }

    /////////////////////////////////////////////////////////////////

    std::cout << "(DD) NbMonteCarlo : " << NbMonteCarlo << std::endl;
    std::cout << "(DD) EbN0         : " << EbN0         << std::endl;
    std::cout << "(DD) q            : " << q            << std::endl;
    std::cout << "(DD) p            : " << p            << std::endl;
    std::cout << "(DD) N            : " << N            << std::endl;
    std::cout << "(DD) K            : " << K            << std::endl;
    std::cout << "(DD) dec_type     : " << dec_type     << std::endl;
    std::cout << "(DD) FER_STOP     : " << FER_STOP     << std::endl;
    std::cout << "(DD) num_threads  : " << num_threads  << std::endl;

    /////////////////////////////////////////////////////////////////

    std::transform(dec_type.begin(), dec_type.end(), dec_type.begin(), ::tolower);
    
	base_code_t code_param(N, K, n, q, p, frozen_val);
	code_param.sig_mod = "CCSK_BIN";

	int   gf_rand_SEED = 0;
	float nse_rand_SEED = 1.2544;
	bool  repeatable_randgen = 0;

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

	const auto s_start = std::chrono::system_clock::now();

	float sigma = sqrt(1.0 / (pow(10, EbN0 / 10.0)));
	float fake_sigma = sigma;

	CCSK_Simulator<_GF_, _N_> simulator(sigma, fake_sigma, num_threads);

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
		std::vector<symbols_s<_GF_>> llrs_n(N);
		std::vector<uint16_t> decoded_n(N);

		// Initialize decoder
		decoder *dec = nullptr;


        dec = loader_so::allocate_dec(dec_type, N, _GF_, frozen_symbols);
#if 0
        if (dec_type == "dec1")
        {
            //dec = new decoder_naive<_GF_>(N, frozen_symbols);
            std::cout << "loader_so::allocate_dec(dec1)" << std::endl;
            dec = loader_so::allocate_dec("dec1", N, _GF_, frozen_symbols);
        }
		else if (dec_type == "dec1_fixed")
		{
			dec = new decoder_naive_fixed<_GF_>(N, frozen_symbols);
            printf("(EE) The code here was not updated [%s:%d]\n", __FILE__, __LINE__);
            exit( EXIT_FAILURE );
		}
		else if (dec_type == "dec1_cfloat")
		{
			dec = new decoder_naive_cfloat<_GF_>(N, frozen_symbols);
            printf("(EE) The code here was not updated [%s:%d]\n", __FILE__, __LINE__);
            exit( EXIT_FAILURE );
		}
        else if (dec_type == "dec0")
        {
            //dec = new decoder_basic<_GF_>(N, frozen_symbols);
            std::cout << "loader_so::allocate_dec(dec0)" << std::endl;
            dec = loader_so::allocate_dec("dec0", N, _GF_, frozen_symbols);
        }
		else if (dec_type == "dec3")
		{
			//dec = new decoder_specialized<_GF_>(N, frozen_symbols);
            std::cout << "loader_so::allocate_dec(dec3)" << std::endl;
            dec = loader_so::allocate_dec("dec3", N, _GF_, frozen_symbols);
		}
        else if (dec_type == "dec4")
        {
            //dec = new decoder_specialized_pruning<_GF_>(N, frozen_symbols);
            std::cout << "loader_so::allocate_dec(dec4)" << std::endl;
            dec = loader_so::allocate_dec("dec4", N, _GF_, frozen_symbols);
        }
        else if (dec_type == "scl2-genius")
        {
            std::cout << "loader_so::allocate_dec(scl2-genius)" << std::endl;
            dec = loader_so::allocate_dec("scl2-genius", N, _GF_, frozen_symbols);
        }
        else if (dec_type == "scl4-genius")
        {
            std::cout << "loader_so::allocate_dec(scl4-genius)" << std::endl;
            dec = loader_so::allocate_dec("scl4-genius", N, _GF_, frozen_symbols);
        }
        else if (dec_type == "scf1-genius")
        {
            std::cout << "loader_so::allocate_dec(scf1-genius)" << std::endl;
            dec = loader_so::allocate_dec("scf1-genius", N, _GF_, frozen_symbols);
        }
        else if (dec_type == "scf2-genius")
        {
            std::cout << "loader_so::allocate_dec(scf2-genius)" << std::endl;
            dec = loader_so::allocate_dec("scf2-genius", N, _GF_, frozen_symbols);
        }
        else if (dec_type == "scf3-genius")
        {
            std::cout << "loader_so::allocate_dec(scf3-genius)" << std::endl;
            dec = loader_so::allocate_dec("scf3-genius", N, _GF_, frozen_symbols);
        }
        else if (dec_type == "scf4-genius")
        {
            std::cout << "loader_so::allocate_dec(scf4-genius)" << std::endl;
            dec = loader_so::allocate_dec("scf4-genius", N, _GF_, frozen_symbols);
        }
		else if (dec_type == "dec1_integer")
		{
			dec = new decoder_naive_integer<_GF_>(N, frozen_symbols);
            printf("(EE) The code here was not updated [%s:%d]\n", __FILE__, __LINE__);
            exit( EXIT_FAILURE );
		}
		else if (dec_type == "dec4_integer")
		{
			dec = new decoder_specialized_pruning_integer<_GF_>(N, frozen_symbols);
            printf("(EE) The code here was not updated [%s:%d]\n", __FILE__, __LINE__);
            exit( EXIT_FAILURE );
		}
		else
		{
			{
				std::cerr << "Error: Unknown decoder type: " << dec_type << std::endl;
			}
			exit(1);
		}
#endif
        
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
                r_symb[u] = u_symb[u]; // le mode génie dans la simulation

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
					for (int j = 0; j < _GF_; j++)
					{
						llrs_n[i].value[j] = static_cast<float>(llr_values[i * _GF_ + j]);
					}
				}
			}
			else if constexpr (SimulationParams::method == SimulationParams::LLRMethod::FAST_LUT)
			{
				// Fast method: lookup table
				float *probabilities = simulator.llr_to_probability_fast<_GF_>(llr_values, N, thread_id);
				for (int i = 0; i < N; i++)
				{
					for (int j = 0; j < _GF_; j++)
					{
						llrs_n[i].value[j] = probabilities[i * _GF_ + j];
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
            if ( thread_id == 0 )
			{
                auto curr = std::chrono::high_resolution_clock::now();
                auto sec  = std::chrono::duration<double>(curr - watchdod).count();
                if( sec >= 1 )
				{
					uint64_t local_success = global_counter.load() - FER.load();
					if ((global_counter.load() >= NbMonteCarlo) ||
						(FER.load() >= FER_STOP))
						stop.store(true);
					FER_out = FER.load();
					gen_frames_out = global_counter.load();

					double FER_ratio = (double)FER_out / gen_frames_out;
					std::ostringstream oss;
					FER_ratio < 0.0001 ? oss << std::scientific << std::setprecision(3) << std::setw(10) << FER_ratio : oss << std::fixed << std::setprecision(6) << std::setw(10) << FER_ratio;
					std::string FER_str = oss.str();

                    auto   end = std::chrono::high_resolution_clock::now();
                    double sec = std::chrono::duration<double>(end - start).count();

					std::cout << "\rSNR: " << std::fixed << std::setprecision(1) << EbN0
							  << " dB, FER = " << std::setw(8) << FER_out
							  << "/" << std::setw(8) << gen_frames_out
                    << " = " << FER_str;

                    printf(" | %6d sec. | ", (int)sec);

                    if( FER_out == 0 )
                        printf("------");
                    else{
                        double tps_p_err = (double)FER_out / sec;
                        double tps_rest  = (double)(FER_STOP - FER_out) * tps_p_err;
                        printf("%6d sec. | ", (int)tps_rest);
                    }
                    std::cout << std::flush << "\r";
                    watchdod = std::chrono::high_resolution_clock::now();
				}
			}
			if (stop.load())
				break;
		}
        delete dec;
	}

	auto end = std::chrono::high_resolution_clock::now();
	double sec = std::chrono::duration<double>(end - start).count();

	double FER_ratio = (double)FER_out / gen_frames_out;
	std::ostringstream oss;
	FER_ratio < 0.0001 ? oss << std::scientific << std::setprecision(3) << std::setw(10) << FER_ratio : oss << std::fixed << std::setprecision(6) << std::setw(10) << FER_ratio;
	std::string FER_str = oss.str();

	std::cout << "\rSNR: " << std::fixed << std::setprecision(1) << EbN0
			  << " dB, FER = " << std::setw(8) << FER_out
			  << "/" << std::setw(8) << gen_frames_out
			  << " = " << FER_str
			  << std::flush;

	append_results_to_file1(dec_type, q, N, K, EbN0, FER_out, gen_frames_out, fake_sigma);

	// Final results
	std::cout << "\nPolar Code: N=" << N << ", K=" << K << ", GF=" << _GF_ << std::endl;
	std::cout << "Decoder: " << dec_type << std::endl;
	std::cout << "Eb/N0: " << EbN0 << " dB, Sigma: " << sigma << std::endl;
	std::cout << "Actual frames: " << gen_frames_out << std::endl;
	std::cout << "Time: " << sec << " seconds" << std::endl;
	std::cout << "Throughput: " << gen_frames_out / sec << " fps" << std::endl;
	std::cout << "Throughput info: " << (gen_frames_out * K * _logGF_) / sec / 1e6 << " Mbps" << std::endl;
#ifdef find_llr_rang
	std::cout << "global_llr_max: " << global_llr_max << std::endl;
	std::cout << "global_llr_min: " << global_llr_min << std::endl;
#endif
}
