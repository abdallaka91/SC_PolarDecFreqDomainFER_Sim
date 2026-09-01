// #define find_llr_rang // disable it

#include "init.h"
#include "struct.h"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <omp.h>
#include <signal.h>
#include <sstream>
#include <string>
#include <vector>

#include "./ccsk_simulator/ccsk_simulator.hpp"
#include "./ccsk_simulator/simul_parameters.hpp"
#include <chrono>
#include <iostream>

#include "./include/encoder_1.hpp"
#include "./include/loader_so.hpp"

#if 0
#include "./include/frame_dump.hpp"
#include "./include/frame_reader.hpp"
#endif

using namespace PoAwN::structures;
using namespace PoAwN::init;

#include <cstdio>
#include <iostream>
#include <string>

#include "./source/crc/crc_16b.hpp"

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
//
//
//
//
void append_results_to_file1(const std::string &dec, int GFx, int Nx, int Kx, double SNR, unsigned long nb_err, uint64_t nb_gen_frame, const float forced_EbN0, const bool forced_mode, const float llr_sigma, const int seconds, const int nbits = -1)
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

	if (forced_mode == true)
	{
		filename = dir / ("GF" + std::to_string(GFx) + "_N" + std::to_string(Nx) +
						  "_K" + std::to_string(Kx) + "_" + dec.c_str() +
						  "_forced_" + std::to_string(forced_EbN0) + ".txt");
	}

	FILE *fou = fopen(filename.c_str(), "a");

	if (fou == nullptr)
	{
		std::cerr << "Error opening file " << filename << " for appending.\n";
		return;
	}

	double FER_value =
		(nb_gen_frame == 0) ? 0.0 : static_cast<double>(nb_err) / nb_gen_frame;
	std::string FER_str = format_FER(FER_value, 11); // 11 chars wide

	fprintf(fou, "%+7.3f   %s   %6lu   %12" PRIu64, SNR, FER_str.c_str(), nb_err, nb_gen_frame);
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
	if (force_quit == true)
	{
		printf("\n#(DD) CTRL+C was already called, forcing the program termination "
			   "!\n");
		exit(EXIT_FAILURE);
	}
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
	printf("#(II) Non-binary FFT Successive Cancellation wrong frame replay "
		   "program (AVX512 version)\n");
#elif __AVX2__
	printf("#(II) Non-binary FFT Successive Cancellation wrong frame replay "
		   "program (AVX2 version)\n");
#else
	printf("#(II) Non-binary FFT Successive Cancellation wrong frame replay "
		   "program (ARM NEON version)\n");
#endif

	printf("#(II) + developped by Abdallah ABDALLAH in 2025...\n");
	printf("#(II) +        and by Camille MONIERE   in 2025...\n");
	printf("#(II) +        and by Bertrand LE GAL   in 2025...\n");
	printf("#(II)\n");
	printf("#(II) Binary generated : %s - %s\n", __DATE__, __TIME__);
	printf("#(II)\n");
#if defined(__clang__)
	/* Clang/LLVM. ---------------------------------------------- */
	printf("#(II) + Clang/LLVM version %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__ICC) || defined(__INTEL_COMPILER)
	/* Intel ICC/ICPC. ------------------------------------------ */
	printf("# + Intel ICC/ICPC version %d.%d\n", __INTEL_COMPILER, __INTEL_COMPILER_BUILD_DATE);
#elif defined(__GNUC__) || defined(__GNUG__)
	/* GNU GCC/G++. --------------------------------------------- */
	printf("#(II) + GNU GCC/G++ version %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
	/* Microsoft Visual Studio. --------------------------------- */
	printf("#(II) + Microsoft Visual Studio\n");
#else
	#error "#(II) + Undetected compiler !"
#endif

#if (defined(__ICC) || defined(__INTEL_COMPILER)) == 0
	std::time_t t = std::time(nullptr);
	std::cout << "#(II) + Trace date and time : "
			  << std::put_time(std::localtime(&t), "%c %Z") << '\n';
	printf("#(II)\n");
#endif

    printf("# Run command:\n# ");
    for(uint32_t i = 0; i < argc; i += 1){
        printf("%s ", argv[i]);
    }printf("\n");
	printf("#(II)\n");
	
	signal(SIGINT, intHandler);

#ifdef __APPLE__
	bool ok = loader_so::open("libNbScFFTdec.dylib");
#else
	bool ok = loader_so::open("libNbScFFTdec.so");
#endif

	if (ok == false)
	{
		printf("#(EE) + Error during the library loading...\n");
		exit(EXIT_FAILURE);
	}
	
	/////////////////////////////////////////////////////////////////

	int num_threads = omp_get_max_threads();
	
	/////////////////////////////////////////////////////////////////

	std::string dec_type = "";
	uint64_t NbMonteCarlo = 10000000000;

	float forced_EbN0 = -1000.f;
	bool  forced_mode = false;
	bool  clean_mode = false;

	bool  dump_err_frames = false;

	bool time_limit_ena = false;
	int  time_limit_val =  0;

	float EbN0_mini = -1000.f;
	float EbN0_maxi = -1000.f;
	float EbN0_step = -1000.f;

	bool print_crc_errors = true;

	uint16_t q = 0;
	uint16_t p = 0;
	uint16_t N = 0;
	uint16_t NN = 0;
	uint16_t n = 0;
	uint16_t K = 0;

	int FER_STOP = 100;
	uint16_t frozen_val = 0;
	int nbits = -1;

	float llr_sigma_value  = -1.f;
	bool  llr_sigma_forced = false;

	int debug = 0;

	/////////////////////////////////////////////////////////////////

	for (int i = 1; i < argc; i++)
	{
		if (std::string(argv[i]) == "-snr-min" || std::string(argv[i]) == "-m")
		{
			EbN0_mini = stod(std::string(argv[i + 1]));
			i += 1;
		}
		else if (std::string(argv[i]) == "-snr-max" || std::string(argv[i]) == "-M")
		{
			EbN0_maxi = stod(std::string(argv[i + 1]));
			i += 1;
		}
		else if (std::string(argv[i]) == "-snr-step" || std::string(argv[i]) == "-s")
		{
			EbN0_step = stod(std::string(argv[i + 1]));
			i += 1;
		}
		else if (std::string(argv[i]) == "-target" || std::string(argv[i]) == "-t")
		{
			forced_EbN0 = stod(std::string(argv[i + 1]));
			forced_mode = true;
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
		else if (std::string(argv[i]) == "-NN" ||
				 std::string(argv[i]) == "-shortened-N" ||
				 std::string(argv[i]) == "-shortened-n")
		{
			NN = stoi(std::string(argv[i + 1]));
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
		else if (std::string(argv[i]) == "-clean")
		{
			clean_mode = true;
		}
		else if (std::string(argv[i]) == "-thread" || std::string(argv[i]) == "-threads")
		{
			num_threads = std::atoi(argv[i + 1]);
			omp_set_num_threads(num_threads);
			i += 1;
		}
		else if (std::string(argv[i]) == "-cores" || std::string(argv[i]) == "-c")
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
			llr_sigma_value  = std::stof(argv[i + 1]);
			llr_sigma_forced = true;
			i += 1;
		}
		else if (std::string(argv[i]) == "-nbits_dec")
		{
			nbits = std::atoi(argv[i + 1]);
			i += 1;
		}
		else if (std::string(argv[i]) == "-debug")
		{
			debug = true;
		}
		else if (std::string(argv[i]) == "-verbose")
		{
			debug += 1;
		}
		else if (std::string(argv[i]) == "-dump")
		{
			dump_err_frames = true;
		}
		else if (std::string(argv[i]) == "-time-limit")
		{
			time_limit_ena = true;
			time_limit_val = std::atoi(argv[i + 1]);
			i += 1;
		}
		else if( (std::string(argv[i]) == "-no-crc-errors") || (std::string(argv[i]) == "-no-crc-err") )
		{
			print_crc_errors  = false;
		}
		else
		{
			printf("(EE) Error during CLI parsing\n");
			printf("(EE) argument = [%s]\n", argv[i]);
			exit(EXIT_FAILURE);
		}
	}

	/////////////////////////////////////////////////////////////////
	if (NN == 0)
		NN = N;

	if (EbN0_mini == -1000.f)
	{
		printf("(EE) Error during CLI parsing\n");
		printf("(EE) missing [-snr-min] option\n");
		exit(EXIT_FAILURE);
	}
	if (EbN0_maxi == -1000.f)
	{
		printf("(EE) Error during CLI parsing\n");
		printf("(EE) missing [-snr-max] option\n");
		exit(EXIT_FAILURE);
	}
	if (EbN0_step == -1000.f)
	{
		printf("(EE) Error during CLI parsing\n");
		printf("(EE) missing [-snr-step] option\n");
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
	if (NN > N || NN < K)
	{
		printf("(EE) Invalid shortened length: K <= NN <= N is required "
			   "(K=%u, NN=%u, N=%u)\n", K, NN, N);
		exit(EXIT_FAILURE);
	}
	if (dec_type == "")
	{
		printf("(EE) Error during CLI parsing\n");
		printf("(EE) missing [-dec] option\n");
		exit(EXIT_FAILURE);
	}

/*	NOTE: I added a mutex to solve this issue ;-)

	if ((dump_err_frames == true) && (num_threads != 1))
	{
		printf("(EE) Error during CLI parsing\n");
		printf("(EE) It is impossible to enable both [-dump] and [-thread] options\n");
		printf("(EE) Currently, [-dump] is monothreaded only !\n");
		exit(EXIT_FAILURE);
	}
*/

	/////////////////////////////////////////////////////////////////

	std::cout << "#(DD) NbMonteCarlo : " << NbMonteCarlo << std::endl;
	std::cout << "#(DD) EbN0_min     : " << EbN0_mini << " dB" << std::endl;
	std::cout << "#(DD) EbN0_max     : " << EbN0_maxi << " dB" << std::endl;
	std::cout << "#(DD) EbN0_step    : " << EbN0_step << " dB" << std::endl;
	std::cout << "#(DD) N            : " << N << std::endl;
	std::cout << "#(DD) NN           : " << NN << std::endl;
	std::cout << "#(DD) K            : " << K << std::endl;
	std::cout << "#(DD) GF(q)        : " << q << std::endl;
	if (forced_mode == true)
		std::cout << "#(DD) Targeted SNR : " << forced_EbN0 << " dB" << std::endl;
	std::cout << "#(DD) dec_type     : " << dec_type << std::endl;
	std::cout << "#(DD) FER_STOP     : " << FER_STOP << std::endl;
	std::cout << "#(DD) num_threads  : " << num_threads << std::endl;

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
	printf("#------+--------+------------+-----------+--------+--------+--------------------------+--------------------------+-----------+\n");
	printf("#         Simulation parameters          | Simualtion time |   Performance evaluation on a single-core decoder   | Simulat°  |\n");
	printf("# SNR  | Simul. |  Simulated |  Obtained | Elaps. | Remain |    decoding throughput   |    decoding latency      | Simulat°  |\n");
	printf("#  dB  | errors |     frames |       FER |   sec. |   sec. |           in MBps        |         in u-sec         |     MBps  |\n");
	printf("#------+--------+------------+-----------+--------+--------+--------------------------+--------------------------+-----------+\n");
	printf("  SNR  | F.Errs |     frames |       FER | E.Time | R.Time |  T.avg |  T.min |  T.max |  L.avg |  L.min |  L.max | Througput |\n");
	//
	// Loop ici mais comment gere t'on le forced SNR ?
	//
	for (float cSNR = EbN0_mini; cSNR <= EbN0_maxi; cSNR += EbN0_step)
	{
		//
		//
		//
		base_code_t code_param(N, K, n, q, p, frozen_val);
		code_param.sig_mod = "CCSK_BIN";

		//		int   gf_rand_SEED = 0;
		//		float nse_rand_SEED = 1.2544;
		//		bool  repeatable_randgen = 0;

		table_GF table;

		const float sSNR = (forced_mode == true) ? forced_EbN0 : cSNR;
		if (debug >= 2)
		{
			printf("#(DD)\n");
			printf("#(DD) Frozen vector configured for EbN0 = %f\n", sSNR);
		}
		LoadCode(code_param, sSNR, "./matrices/", debug >= 2);

		//
		// On cree toujours le logger et le parametre "dump_err_frames" active
		// ou pas le code en interne.
		//
#if 0		
		std::mutex mtx;  // Declare a mutex
		frame_dumper frame_store(N, K, q, cSNR, sSNR, dump_err_frames);
#endif
		//
		//
		//
		std::vector<uint16_t> reliability_low_to_high(N);
		for (int i = 0; i < N; ++i)
			reliability_low_to_high[i] = code_param.reliab_sequence[N - i - 1];

		const uint16_t short_count = N - NN;
		std::vector<uint16_t> short_positions;
		std::vector<uint16_t> frozen_positions;
		if (short_count > 0)
		{
			const std::vector<uint16_t> shortening_order =
				shortened_sequence(reliability_low_to_high, N);
			short_positions.assign(shortening_order.begin(),
								   shortening_order.begin() + short_count);
			frozen_positions = froz_from_short(
				N, reliability_low_to_high, N - K, short_positions);
		}
		else
		{
			frozen_positions.assign(reliability_low_to_high.begin(),
									reliability_low_to_high.begin() + (N - K));
		}

		if (frozen_positions.size() != static_cast<size_t>(N - K))
		{
			printf("#(EE) Shortening produced %zu frozen positions; expected %u\n",
				   frozen_positions.size(), N - K);
			exit(EXIT_FAILURE);
		}

		std::vector<int> frozen_symbols(N, false);
		for (const uint16_t pos : frozen_positions)
			frozen_symbols.at(pos) = true;

		std::vector<uint16_t> information_positions;
		information_positions.reserve(K);
		for (int pos = 0; pos < N; ++pos)
			if (!frozen_symbols[pos])
				information_positions.push_back(pos);
		//
		//
		//

		if (debug >= 2)
		{
			printf("#(II) Reliability sequence:\n");
			printf("#(II) ");
			for (const uint16_t pos : information_positions)
			{
				printf("%2d ", pos);
			}
			printf("\n");
			if (short_count > 0)
			{
				printf("#(II) Shortened codeword positions:\n#(II) ");
				for (const uint16_t pos : short_positions)
					printf("%2d ", pos);
				printf("\n");
			}
		}

		const auto s_start = std::chrono::system_clock::now();

		//
		// On cree le canal de simulation
		//

		const float noise_sigma = sqrt(1.0 / (pow(10, cSNR / 10.0)));
		const float llr_sigma = (llr_sigma_forced == true) ? llr_sigma_value : noise_sigma;

		if (debug >= 2)
		{
			printf("#(II) Allocating CCSK channel\n");
		}

		CCSK_Channel *simulator = nullptr;
		//
		// Q = 64
		//
		     if (N ==   64 && q == 64) simulator = new CCSK_Simulator<64,   64>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  128 && q == 64) simulator = new CCSK_Simulator<64,  128>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  256 && q == 64) simulator = new CCSK_Simulator<64,  256>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  512 && q == 64) simulator = new CCSK_Simulator<64,  512>(noise_sigma, llr_sigma, num_threads);
		else if (N == 1024 && q == 64) simulator = new CCSK_Simulator<64, 1024>(noise_sigma, llr_sigma, num_threads);
		else if (N == 2048 && q == 64) simulator = new CCSK_Simulator<64, 2048>(noise_sigma, llr_sigma, num_threads);
		//
		// Q = 128
		//
		else if (N ==   64 && q == 128) simulator = new CCSK_Simulator<128,   64>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  128 && q == 128) simulator = new CCSK_Simulator<128,  128>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  256 && q == 128) simulator = new CCSK_Simulator<128,  256>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  512 && q == 128) simulator = new CCSK_Simulator<128,  512>(noise_sigma, llr_sigma, num_threads);
		else if (N == 1024 && q == 128) simulator = new CCSK_Simulator<128, 1024>(noise_sigma, llr_sigma, num_threads);
		else if (N == 2048 && q == 128) simulator = new CCSK_Simulator<128, 2048>(noise_sigma, llr_sigma, num_threads);
		//
		// Q = 256
		//
		else if (N ==   64 && q == 256) simulator = new CCSK_Simulator<256,   64>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  128 && q == 256) simulator = new CCSK_Simulator<256,  128>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  256 && q == 256) simulator = new CCSK_Simulator<256,  256>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  512 && q == 256) simulator = new CCSK_Simulator<256,  512>(noise_sigma, llr_sigma, num_threads);
		else if (N == 1024 && q == 256) simulator = new CCSK_Simulator<256, 1024>(noise_sigma, llr_sigma, num_threads);
		else if (N == 2048 && q == 256) simulator = new CCSK_Simulator<256, 2048>(noise_sigma, llr_sigma, num_threads);
		//
		// Q = 512
		//
		else if (N ==   64 && q == 512) simulator = new CCSK_Simulator<512,   64>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  128 && q == 512) simulator = new CCSK_Simulator<512,  128>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  256 && q == 512) simulator = new CCSK_Simulator<512,  256>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  512 && q == 512) simulator = new CCSK_Simulator<512,  512>(noise_sigma, llr_sigma, num_threads);
		else if (N == 1024 && q == 512) simulator = new CCSK_Simulator<512, 1024>(noise_sigma, llr_sigma, num_threads);
		else if (N == 2048 && q == 512) simulator = new CCSK_Simulator<512, 2048>(noise_sigma, llr_sigma, num_threads);
		//
		// Q = 1024
		//
		else if (N ==   64 && q == 1024) simulator = new CCSK_Simulator<1024 ,  64>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  128 && q == 1024) simulator = new CCSK_Simulator<1024,  128>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  256 && q == 1024) simulator = new CCSK_Simulator<1024,  256>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  512 && q == 1024) simulator = new CCSK_Simulator<1024,  512>(noise_sigma, llr_sigma, num_threads);
		else if (N == 1024 && q == 1024) simulator = new CCSK_Simulator<1024, 1024>(noise_sigma, llr_sigma, num_threads);
		else if (N == 2048 && q == 1024) simulator = new CCSK_Simulator<1024, 2048>(noise_sigma, llr_sigma, num_threads);

		//
		// Q = 1024
		//
		else if (N ==   64 && q == 8) simulator = new CCSK_Simulator<8 ,  64>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  128 && q == 8) simulator = new CCSK_Simulator<8,  128>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  256 && q == 8) simulator = new CCSK_Simulator<8,  256>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  512 && q == 8) simulator = new CCSK_Simulator<8,  512>(noise_sigma, llr_sigma, num_threads);
		else if (N == 1024 && q == 8) simulator = new CCSK_Simulator<8, 1024>(noise_sigma, llr_sigma, num_threads);
		else if (N == 2048 && q == 8) simulator = new CCSK_Simulator<8, 2048>(noise_sigma, llr_sigma, num_threads);

		//
		// Q = 1024
		//
		else if (N ==   64 && q == 16) simulator = new CCSK_Simulator<16 ,  64>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  128 && q == 16) simulator = new CCSK_Simulator<16,  128>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  256 && q == 16) simulator = new CCSK_Simulator<16,  256>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  512 && q == 16) simulator = new CCSK_Simulator<16,  512>(noise_sigma, llr_sigma, num_threads);
		else if (N == 1024 && q == 16) simulator = new CCSK_Simulator<16, 1024>(noise_sigma, llr_sigma, num_threads);
		else if (N == 2048 && q == 16) simulator = new CCSK_Simulator<16, 2048>(noise_sigma, llr_sigma, num_threads);

		//
		// Q = 1024
		//
		else if (N ==   64 && q == 32) simulator = new CCSK_Simulator<32 ,  64>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  128 && q == 32) simulator = new CCSK_Simulator<32,  128>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  256 && q == 32) simulator = new CCSK_Simulator<32,  256>(noise_sigma, llr_sigma, num_threads);
		else if (N ==  512 && q == 32) simulator = new CCSK_Simulator<32,  512>(noise_sigma, llr_sigma, num_threads);
		else if (N == 1024 && q == 32) simulator = new CCSK_Simulator<32, 1024>(noise_sigma, llr_sigma, num_threads);
		else if (N == 2048 && q == 32) simulator = new CCSK_Simulator<32, 2048>(noise_sigma, llr_sigma, num_threads);

		if (simulator == nullptr)
		{
			printf("#(EE) + Error during the simulation env. creation...\n");
			exit(EXIT_FAILURE);
		}
		// std::atomic<uint64_t> frame_errors(0);
		// std::atomic<uint64_t> frames_simulated(0);

		int64_t FER_out = 0;
		int64_t gen_frames_out = 0;
		std::atomic<uint64_t> global_counter(0);
		std::atomic<uint64_t> FER(0);
		std::atomic<bool> stop(false);

		auto start    = std::chrono::high_resolution_clock::now();
		auto watchdod = std::chrono::high_resolution_clock::now();

		//
		//
		//

		if (debug >= 2)
		{
			printf("#(II) Starting openmp section\n");
		}

#pragma omp parallel
		{
			int thread_id = omp_get_thread_num();
			std::vector<uint16_t> K_symb(K);
			std::vector<uint16_t> F_symb(K);
			std::vector<uint16_t> u_symb(N);
			std::vector<uint16_t> r_symb(N);
			std::vector<float> llrs_n(N * q);
			std::vector<uint16_t> decoded_n(N);

			// Initialize decoder
			decoder *dec = nullptr;
#if 0
			printf("dec->k  = %d\n", dec->k());
			printf("dec->n  = %d\n", dec->n());
			printf("dec->gf = %d\n", dec->gf());
#endif

			dec = allocate_dec(dec_type, N, q, frozen_symbols.data());

			//
			// On genere le mapping des K symbols dans le mot de N
			//
			std::vector<uint16_t> k_pos = information_positions;
			//
			//
#if 0
			printf("Reordered k_pos[x] : ");
			for (int x = 0; x < K; x += 1) {
				printf("%2d ", k_pos[x]);
			}
			printf("\n");
#endif
			//
			//
			while (true)
			{

				//
				// Generate symbols for THIS frame
				//
				simulator->generate_random_symbols(K_symb.data(), K, thread_id);
#ifdef _CRC_DEBUG_
				printf("GEN : "); for (int i = 0; i < K; i++) printf("%2d ", K_symb[i]); printf("\n");
#endif
				//
				// On insere le CRC 16b
				//
				
				crc16_insert(K_symb.data(), K, q);
				//
				// On verifie le CRC 16b
				//
#ifdef _CRC_DEBUG_
				const bool crc_ok = crc16_verify(K_symb.data(), K, q);
				if( crc_ok == false ){
					printf("CRC error !\n");
					exit(EXIT_FAILURE);
				}
#endif
				//
				//
				//

#ifdef _CRC_DEBUG_
				printf("CRC : "); for (int i = 0; i < K; i++) printf("%2d ", K_symb[i]); printf("\n");
#endif
				for (int u = 0; u < N; u++)
				{
					u_symb[u] = 0;
				}
				for (int u = 0; u < K; u++)
				{
					u_symb[ k_pos[u] ] = K_symb[u];
				}

#ifdef _CRC_DEBUG_
				printf("PRE : "); for (int i = 0; i < N; i++) printf("%2d ", u_symb[i]); printf("\n");
#endif

				for (int u = 0; u < N;
					 u++)
				{						   // on conserve une copie des données afin d'utiliser
					r_symb[u] = u_symb[u]; // le mode génie dans la simulation
				}

				     if (N ==   64) polar_encode<  64>(u_symb.data());
				else if (N ==  128) polar_encode< 128>(u_symb.data());
				else if (N ==  256) polar_encode< 256>(u_symb.data());
				else if (N ==  512) polar_encode< 512>(u_symb.data());
				else if (N == 1024) polar_encode<1024>(u_symb.data());
				else if (N == 2048) polar_encode<2048>(u_symb.data());
				else
					exit(EXIT_FAILURE);

#ifdef _CRC_DEBUG_
				printf("ENC : "); for (int i = 0; i < N; i++) printf("%2d ", u_symb[i]); printf("\n");
#endif

				//
				// Simulate CCSK transmission
				//
				double *llr_values = simulator->simulate_frame(u_symb.data(), thread_id);
				if constexpr (SimulationParams::method ==
							  SimulationParams::LLRMethod::EXP)
				{
					// Original method: exp() calls
					simulator->llr_to_probability(llr_values, N);
					for (int i = 0; i < N; i++)
					{
						for (int j = 0; j < q; j++)
						{
							llrs_n[i * q + j] = static_cast<float>(llr_values[i * q + j]);
						}
					}
				}

				// Decode

				else if constexpr (SimulationParams::method ==
									   SimulationParams::LLRMethod::FAST_LUT)
				{
					// Fast method: lookup table
					float *probabilities =
						simulator->llr_to_probability_fast(llr_values, N, thread_id);
					for (int i = 0; i < N; i++)
					{
						for (int j = 0; j < q; j++)
						{
							llrs_n[i * q + j] = probabilities[i * q + j];
						}
					}
				}

				for (const uint16_t pos : short_positions)
				{
					llrs_n[pos * q] = 1.0f - 1e-12f;
					for (int symbol = 1; symbol < q; ++symbol)
						llrs_n[pos * q + symbol] = 1e-12f;
				}

				dec->setResult(r_symb.data());

				dec->execute(llrs_n.data(), decoded_n.data());

				//
				// Check for errors
				//
#if 0
				for (int i = 0; i < N * q; i++)
					printf("%1.3f ", llrs_n[i]);
				printf("\n");

				for (int i = 0; i < N; i++)
					printf("%d ", r_symb[i]);
				printf("\n");
				for (int i = 0; i < N; i++)
					printf("%d ", u_symb[i]);
				printf("\n");
				for (int i = 0; i < N; i++)
					printf("%d ", decoded_n[i]);
				printf("\n");
				printf("\n");
#endif

#ifdef _CRC_DEBUG_
				printf("DEC : "); for (int i = 0; i < N; i++) printf("%2d ", decoded_n[i]); printf("\n");
				printf("Kou : "); for (int i = 0; i < K; i++) printf("%2d ", decoded_n[ k_pos[i] ]); printf("\n");
#endif

				//
				// On récupère les K symboles décodés (dans le bon ordre)
				//
				for (int i = 0; i < K; i++){
					F_symb[i] = decoded_n[ k_pos[i] ];
				}
#ifdef _CRC_DEBUG_
				printf("Fou : "); for (int i = 0; i < K; i++) printf("%2d ", F_symb[i]); printf("\n");
#endif

				//
				// On vérifie que la trame recu = trame émise
				//
				bool succ_dec = true;
				for (int i = 0; i < K; i += 1)
				{
					succ_dec &= ( K_symb[i] == F_symb[i] );
				}

				//
				// On verifie le CRC 16b
				//
				const bool dec_crc_ok = crc16_verify(F_symb.data(), K, q);
				if( dec_crc_ok ^ succ_dec )
				{
					if( print_crc_errors == true )
						printf("#(DD) CRC error : the frame is wrong but the CRC is correct !\n");
				}
				//
				//
				//


				//
				//
				//
				global_counter.fetch_add(1);
				// uint64_t succ_now = global_counter.load() - FER.load();
				if (!succ_dec)
				{
					FER.fetch_add(1);
				}
				// succ_now = global_counter.load() - FER.load();

#if 0				
				if( (succ_dec == false) && (dump_err_frames == true) )
				{
					mtx.lock();  // Lock the mutex before accessing the shared variable
					frame_store.write_k_symbols ( K_symb );
					frame_store.write_u_symbols ( r_symb );
					frame_store.write_noisy_llrs( llrs_n );
					frame_store.write_number_proc_frames( global_counter.load() );					
					mtx.unlock();  // Unlock the mutex after the critical section
				}
#endif
				//
				// Plus d'openmp critical car on filtre sur thread 1
				//
				if ( thread_id == 0 )
				{
					auto curr = std::chrono::high_resolution_clock::now();
					auto sec  = std::chrono::duration<double>(curr - watchdod).count();
					if (sec >= 1)
					{
						if ((global_counter.load() >= NbMonteCarlo) ||
							(FER.load() >= FER_STOP))
						{
							stop.store(true);
						}
						FER_out = FER.load();
						gen_frames_out = global_counter.load();

						const double FER_ratio = (double)FER_out / gen_frames_out;

						auto end   = std::chrono::high_resolution_clock::now();
						double sec = std::chrono::duration<double>(end - start).count();

						// std::cout << "\r" << std::fixed << std::setprecision(1) << cSNR
						//		<< " dB, FER = " << std::setw(8) << FER_out
						//		<< "/" << std::setw(8) << gen_frames_out
						//		<< " = " << FER_str;

						//
						// NB-polar decoder statistics (thread 0)
						//
						const float d_avg = 0.0f;
						const float d_min = 0.0f;
						const float d_max = 0.0f;

						const float l_avg = 0.0f;
						const float l_min = 0.0f;
						const float l_max = 0.0f;

						//
						// Simulation statistics
						//
						auto   cend = std::chrono::high_resolution_clock::now();
						double csec = std::chrono::duration<double>(cend - start).count();
						const float fps = (double)gen_frames_out / (double)csec;
						const float tgt = (fps * N * p) / 1000.0 / 1000.0;

						if( clean_mode == false )
						{
							if (FER_out != 0)
							{
								double tps_p_err = (double)sec / (double)FER_out;
								double restAnt = (FER_STOP - FER_out);
								double restant = std::max(restAnt, 0.0);
								double tps_rest = (double)(restant)*tps_p_err;
								printf("%6.2f | %6lu | %10lu | %1.3e | %6d | %6d | %6.2f | %6.2f | %6.2f | %6.1f | %6.1f | %6.1f | %9.2f |  \r",
									cSNR,
									FER_out,
									gen_frames_out,
									FER_ratio,
									int(sec), int(tps_rest),
									d_avg, d_min, d_max, l_avg, l_min, l_max, tgt);
							}
							else
							{
								const double FER_ratiO = (double)1.0 / gen_frames_out;
								printf("%6.2f | %6lu | %10lu | %1.3e | %6d | %6d | %6.2f | %6.2f | %6.2f | %6.1f | %6.1f | %6.1f | %9.2f |  \r",
									cSNR,
									FER_out,
									gen_frames_out,
									FER_ratiO,
									int(sec), 0,
									d_avg, d_min, d_max, l_avg, l_min, l_max, tgt);
							}
							fflush(stdout);
						}
						watchdod = std::chrono::high_resolution_clock::now();
					}
				}

				if (stop.load())
				{
					break;
				}

				if (force_quit == true)
				{
					break;
				}

				//
				// Used to limit simulation time of SNR point /OR/ draw throughput curves for SCL/SCF decoders
				//
				if( time_limit_ena == true )
				{
					auto   cend = std::chrono::high_resolution_clock::now();
					double csec = std::chrono::duration<double>(cend - start).count();
					if( csec >= time_limit_val ){
						force_quit = true;
						break;
					}
				}

			}

			delete dec;
		}

		auto   end = std::chrono::high_resolution_clock::now();
		double sec = std::chrono::duration<double>(end - start).count();

		double FER_ratio = (double)FER_out / gen_frames_out;
		// std::ostringstream oss;
		//	FER_ratio < 0.0001 ? oss << std::scientific << std::setprecision(3) <<
		// std::setw(10) << FER_ratio : oss << std::fixed << std::setprecision(6) <<
		// std::setw(10) << FER_ratio;
		// oss << std::scientific << std::setprecision(3) << std::setw(10) <<
		// FER_ratio;

		if( debug >= 1 )
		{
			double fps = (double)gen_frames_out / (double)sec;
			double tgt = (fps * N * p) / 1000.0 / 1000.0;
			printf("#(DD) Simulation throughput = %f Mbps\n", tgt);
		}

		// std::string FER_str = oss.str();

		// std::cout << "\rSNR: " << std::fixed << std::setprecision(1) << cSNR
		//		<< " dB, FER = " << std::setw(8) << FER_out
		//		<< "/" << std::setw(8) << gen_frames_out
		//		<< " = " << FER_str
		//		<< std::flush;
		printf("\r%6.2f | %6lu | %10lu | %1.3e | %6d | ------ |", cSNR, FER_out, gen_frames_out, FER_ratio, (int)sec);
		printf("\n");
		fflush(stdout);

		append_results_to_file1(dec_type, q, N, K, cSNR, FER_out, gen_frames_out, forced_EbN0, forced_mode, llr_sigma, (int)sec, nbits);

		delete simulator;

		//
		// Si CTRL+C alors on quitte la simulation
		//
		if (force_quit == true)
			break;

	} // fin de la boucle sur le SNR
#ifdef find_llr_rang
	std::cout << "global_llr_max: " << global_llr_max << std::endl;
	std::cout << "global_llr_min: " << global_llr_min << std::endl;
#endif
}
