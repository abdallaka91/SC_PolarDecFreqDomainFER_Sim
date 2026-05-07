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
#include <signal.h>
#include <format>

#include <chrono>
#include <iostream>

#include "./include/loader_so.hpp"
#include "./include/encoder_1.hpp"
#include "./include/custom_types.hpp"
#include "./include/frame_reader.hpp"

using namespace PoAwN::structures;
using namespace PoAwN::tools;
using namespace PoAwN::init;

using namespace std;

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
//
//
//
//
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
	printf("# Non-binary FFT Successive Cancellation wrong frame replay "
		   "program (AVX512 version)\n");
#elif __AVX2__
	printf("# Non-binary FFT Successive Cancellation wrong frame replay "
		   "program (AVX2 version)\n");
#else
	printf("# Non-binary FFT Successive Cancellation wrong frame replay "
		   "program (ARM NEON version)\n");
#endif

//	printf("# + developped by Abdallah ABDALLAH in 2025...\n");
//	printf("# +        and by Camille MONIERE   in 2025...\n");
//	printf("# +        and by Bertrand LE GAL   in 2025...\n");
//	printf("#\n");
	printf("# Binary generated     : %s - %s\n", __DATE__, __TIME__);
#if (defined(__ICC) || defined(__INTEL_COMPILER)) == 0
	std::time_t t = std::time(nullptr);
	std::cout << "# Trace date and time  : "
			  << std::put_time(std::localtime(&t), "%c %Z") << '\n';
#endif

#if defined(__clang__)
	/* Clang/LLVM. ---------------------------------------------- */
	printf("# Clang/LLVM version %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__ICC) || defined(__INTEL_COMPILER)
	/* Intel ICC/ICPC. ------------------------------------------ */
	printf("# + Intel ICC/ICPC version %d.%d\n", __INTEL_COMPILER, __INTEL_COMPILER_BUILD_DATE);
#elif defined(__GNUC__) || defined(__GNUG__)
	/* GNU GCC/G++. --------------------------------------------- */
	printf("# + GNU GCC/G++ version %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
	/* Microsoft Visual Studio. --------------------------------- */
	printf("# + Microsoft Visual Studio\n");
#else
	#error "# + Undetected compiler !"
#endif
	printf("#\n");


    printf("# Run command:\n# ");
    for(uint32_t i = 0; i < argc; i += 1){
        printf("%s ", argv[i]);
    }printf("\n");
	printf("#\n");

	signal(SIGINT, intHandler);

    int num_threads = omp_get_max_threads();

    ////////////////////////////

//    softdata_t offset;

	
	/////////////////////////////////////////////////////////////////


    std::string dec_type  = "";
    uint64_t NbMonteCarlo = 10000000000;

	float EbN0_mini = -1000.f;
	float EbN0_maxi = -1000.f;
	float EbN0_step = -1000.f;

	float forced_EbN0 = -1000.f;
	bool  forced_mode = false;

	uint16_t q            = 0;
    uint16_t p            = 0;
    uint16_t N            = 0;
    uint16_t n            = 0;
    uint16_t K            = 0;
    int FER_STOP          = 100;
    uint16_t frozen_val   = 0;
	
	bool verbose          = false;
	bool debug            = false;
	bool compact          = false;

    /////////////////////////////////////////////////////////////////

    for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "-snr-min")
		{
			EbN0_mini = stod(std::string(argv[i + 1]));
			i += 1;
		}
		else if (std::string(argv[i]) == "-snr-max")
		{
			EbN0_maxi = stod(std::string(argv[i + 1]));
			i += 1;
		}
		else if (std::string(argv[i]) == "-snr-step")
		{
			EbN0_step = stod(std::string(argv[i + 1]));
			i += 1;
		}else if (std::string(argv[i]) == "-target")
		{
			forced_EbN0 = stod(std::string(argv[i + 1]));
			forced_mode = true;
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
        } else if (std::string(argv[i]) == "-compact") {
            compact = true;
        }else{
            printf("(EE) Error during CLI parsing\n");
            printf("(EE) argument = [%s]\n", argv[i]);
            exit( EXIT_FAILURE );
        }
    }

    /////////////////////////////////////////////////////////////////

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

#ifdef __APPLE__
    bool ok = loader_so::open("libNbScFFTdec.dylib");
#else
    bool ok = loader_so::open("libNbScFFTdec.so");
#endif
	if( ok == false ){
		printf("#(EE) + Error during the library loading...\n");
	}

    /////////////////////////////////////////////////////////////////

    std::cout << "# q            : " << q            << std::endl;
    std::cout << "# N            : " << N            << std::endl;
    std::cout << "# K            : " << K            << std::endl;
    std::cout << "# dec_type     : " << dec_type     << std::endl;
	printf("#\n");

    /////////////////////////////////////////////////////////////////
	if(compact == false){
		printf("#------+--------+------------+-----------+--------+--------+-----------+-----------+-----------+\n");
		printf("#         Simulation                     | Elaps. | Remain | Decoder (single-core) | Simulat°  |\n");
		printf("# SNR  | #frame | #simulated |  Measured | Elaps. | Remain | Througput |  Latency  | Simulat°  |\n");
		printf("#  dB  | errors |     frames |       FER | Elaps. | Remain |      MBps |     usec  |     MBps  |\n");
		printf("#------+--------+------------+-----------+--------+--------+-----------+-----------+-----------+\n");
		printf("  SNR  | F.Errs |     frames |       FER | E.Time | R.Time | CodedT    |  CodedL   |    CodedS |\n");
	}else{
		printf("  SNR  | F.Errs |     frames |       FER | CodedT    |  CodedL   |\n");
	}
	//

    std::transform(dec_type.begin(), dec_type.end(), dec_type.begin(), ::tolower);
    
	for (float cSNR = EbN0_mini; cSNR <= EbN0_maxi; cSNR += EbN0_step)
	{
		base_code_t code_param(N, K, n, q, p, frozen_val);
		code_param.sig_mod = "CCSK_BIN";

		table_GF table;

		const float sSNR = (forced_mode == true) ? forced_EbN0 : cSNR;
		if (debug >= 2)
		{
			printf("#(DD)\n");
			printf("#(DD) Frozen vector configured for EbN0 = %f\n", sSNR);
		}
		LoadCode(code_param, sSNR, "./matrices/");

//		cout << EVAL(FWHT) " and " EVAL(FWHT_NORM) " are used for FWHT operations." << endl;
//		cout << "Simulation starts..." << endl;

		int frozen_symbols[N];
		for (int i = 0; i < N; i += 1)
			frozen_symbols[i] = true;		

		for (int i = 0; i < K; i += 1)
			frozen_symbols[code_param.reliab_sequence[i]] = false;

		if( verbose == true )
		{
			printf("(II) Reliability sequence:\n");
			printf("(II) ");
			for(int i = 0; i < K; i += 1){
				printf("%2d ", code_param.reliab_sequence[i]);
			}
			printf("\n");
		} 


		std::vector<uint16_t> K_symb(K);
		std::vector<uint16_t> u_symb(N);
		std::vector<uint16_t> r_symb(N);
		std::vector<float>    llrs_n(N * q);
		std::vector<uint16_t> decoded_n(N);

		//
		//
		//
		const float tSNR = (forced_mode == true) ? forced_EbN0 : cSNR;
		frame_reader freader(code_param.N, code_param.K, q, cSNR, tSNR);

		uint64_t decoded_frames = freader.read_number_proc_frames();
		
		if( verbose == true )
		{
			std::cout << "(II) Monte-carlo decoded frames : " <<  decoded_frames << std::endl;
		}

		//
		// Initialize decoder
		//
		decoder *decoder = loader_so::allocate_dec(dec_type, N, q, frozen_symbols);
		decoder->setReliability( code_param.reliab_sequence.data() ); // fot SC-flip decoders

		int nErrors = 0;
		int nFrames = 0;

		double exec_time = 0.0;
		auto start = std::chrono::high_resolution_clock::now();
		while (true)
		{
			//
			//
			int ok = true;
			ok &= freader.read_k_symbols ( K_symb );
			ok &= freader.read_u_symbols ( u_symb );
			ok &= freader.read_noisy_llrs( llrs_n );

			if( ok == false ) {
				break;
			}
			//
			//
			for (int u = 0; u < N; u++) // on conserve une copie des données afin d'utiliser
				r_symb[u] = u_symb[u]; // le mode génie dans la simulation

			//
			// Decode
			//
			decoder->setResult(r_symb.data());
			auto e_start = std::chrono::high_resolution_clock::now();
			decoder->execute(llrs_n.data(), decoded_n.data());
			auto e_stop  = std::chrono::high_resolution_clock::now();
			exec_time   += (float)std::chrono::duration_cast<std::chrono::microseconds>(e_stop - e_start).count() / 1000.f;

			//
			// Check for errors
			//
			bool succ_dec = true;
			for (uint16_t i = 0; i < code_param.K; i++)
			{
				if (K_symb[i] != decoded_n[code_param.reliab_sequence[i]])
				{
					succ_dec = false;
					break;
				}
			}

			nErrors += (succ_dec == false);
			nFrames += 1;

			if(force_quit == true)
				break;
		}
		const auto stop      = std::chrono::high_resolution_clock::now();
		const float time_sec = (float)std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() / 1000.f;
		const float exec_sec =  exec_time / 1000.f;

		auto   end = std::chrono::high_resolution_clock::now();
		double sec = std::chrono::duration<double>(end - start).count();

		// ATTENTION: decoded_frames contient déjà les trames qui n'avaient pas
		//			  été correctement décodée, donc pas la peine de les rajouter ;-)
		//double FER_ratio = (double)nErrors / (double)(nFrames + decoded_frames);
		double FER_ratio = (double)nErrors / (double)(decoded_frames);
		if( debug >= 1 )
		{
			double fps = (double)nFrames / (double)sec;
			double tgt = (fps * N * p) / 1000.0 / 1000.0;
			printf("#(DD) Simulation throughput = %f Mbps\n", tgt);
		}

		const float d = decoder->dec_avg_coded_mbps();
		const float l = decoder->dec_avg_latency();

		if( compact == false ){
			printf("\r%6.2f | %6lu | %10lu | %1.3e | %6d | ------ |", cSNR, nErrors, nFrames, FER_ratio, (int)sec);
			printf(" %9.2f | %9.2f |\n", d, l);
			fflush(stdout);
		}else{
			printf("\r%6.2f | %6lu | %10lu | %1.3e |", cSNR, nErrors, nFrames, FER_ratio);
			printf(" %9.2f | %9.2f |\n", d, l);
			fflush(stdout);
		}

		delete decoder;

		if(force_quit == true)
			break;
	}

	return EXIT_SUCCESS;
}
