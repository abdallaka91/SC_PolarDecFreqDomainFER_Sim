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

void append_results_to_file1(const std::string &dec, int GFx, int Nx, int Kx, double SNR, unsigned long nb_err, uint64_t nb_gen_frame, float fake_sigma, int seconds)
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
    
	signal(SIGINT, intHandler);

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
	bool dump_frames      = false;

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
        } else if (std::string(argv[i]) == "-dump") {
            dump_frames = true;
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

	printf("(II) Reliability sequence:\n");
	printf("(II) ");
	for(int i = 0; i < K; i += 1){
		printf("%2d ", code_param.reliab_sequence[i]);
	}
	printf("\n");


	const auto s_start = std::chrono::system_clock::now();

	float sigma = sqrt(1.0 / (pow(10, EbN0 / 10.0)));
	float fake_sigma = sigma;

	CCSK_Simulator<_GF_, _N_> simulator(sigma, fake_sigma, num_threads);

	std::atomic<uint64_t> frame_errors(0);
	std::atomic<uint64_t> frames_simulated(0);

	std::atomic<double> global_llr_min {std::numeric_limits<double>::max()};
	std::atomic<double> global_llr_max {std::numeric_limits<double>::lowest()};

    //auto start = std::chrono::high_resolution_clock::now();

    int thread_id = omp_get_thread_num();
	uint16_t K_symb[K];
    uint16_t u_symb[N];
    uint16_t r_symb[N];
	std::vector<symbols_s<_GF_>> llrs_n(N);
	std::vector<uint16_t> decoded_n(N);

	//
	//
	//
	uint64_t decoded_frames;
	char filen[256];
	sprintf(filen, "./replay/N_%d/K_%d/SNR_%f/corrected_frames.raw\0", code_param.N, code_param.K, EbN0);
	FILE* hh = fopen(filen, "rb");
	if( hh == nullptr ){
		exit( EXIT_FAILURE );
	}
	fread((void*)&decoded_frames, sizeof(uint64_t), 1, hh);
	fclose(hh);
	printf("(II) Monte-carlo decoded frames : %ld\n", decoded_frames);

	//
	// Initialize decoder
	//
	decoder *dec = loader_so::allocate_dec(dec_type, N, _GF_, frozen_symbols);

	int nErrors = 0;
	int nFrames = 0;

	sprintf(filen, "./replay/N_%d/K_%d/SNR_%f/frame_symb_k.raw\0", code_param.N, code_param.K, EbN0);					
	FILE* file_k = fopen(filen, "rb");
	if( file_k == nullptr ){
		printf("(EE) Error during file opening %s\n", filen);
		exit( EXIT_FAILURE );
	}

	sprintf(filen, "./replay/N_%d/K_%d/SNR_%f/frame_symb_n.raw\0", code_param.N, code_param.K, EbN0);					
	FILE* file_n = fopen(filen, "rb");
	if( file_n == nullptr ){ 
		printf("(EE) Error during file opening %s\n", filen);
		exit( EXIT_FAILURE );
	}

	sprintf(filen, "./replay/N_%d/K_%d/SNR_%f/frame_noise.raw\0", code_param.N, code_param.K, EbN0);					
	FILE* file_r = fopen(filen, "rb");
	if( file_r == nullptr ){
		printf("(EE) Error during file opening %s\n", filen);
		exit( EXIT_FAILURE );
	}

	double exec_time = 0.0;
    auto start = std::chrono::high_resolution_clock::now();
	while (true)
	{
		//
		//
		int nBytes = fread((void*)K_symb, sizeof(uint16_t), K, file_k);
		if( nBytes != K ) {
			break;
		}
		//
		//
		nBytes = fread((void*)u_symb, sizeof(uint16_t), N, file_n);
		if( nBytes != N ) {
			break;
		}
		//
		//
		for(int n = 0; n < N; n += 1){
			nBytes = fread((void*)&llrs_n[n], sizeof(symbols_s<_GF_>), 1, file_r);
			if( nBytes != 1 ) {
				break;
			}
		}
		//
		//
        for (int u = 0; u < N; u++) // on conserve une copie des données afin d'utiliser
            r_symb[u] = u_symb[u]; // le mode génie dans la simulation

		//
		// Decode
		//
        dec->setResult(r_symb);
	    auto e_start = std::chrono::high_resolution_clock::now();
		dec->execute(llrs_n.data(), decoded_n.data());
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

	}
    const auto stop      = std::chrono::high_resolution_clock::now();
    const float time_sec = (float)std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() / 1000.f;
    const float exec_sec =  exec_time / 1000.f;

	fclose(file_k);
	fclose(file_n);
	fclose(file_r);

	delete dec;

	const float FER         = ((float)nErrors) / (float)(decoded_frames + nFrames - nErrors);
	const float fps         = ((float)nFrames / exec_sec);
	const float thgt_n      = ((float)p * (float)N) * fps;
	const float thgt_k      = ((float)p * (float)K) * fps;
	const float thgt_mbps_n = thgt_n / 1000.f / 1000.f;
	const float thgt_mbps_k = thgt_k / 1000.f / 1000.f;

	printf("nFrames = %d\n", nFrames);
	printf("nErrors = %d\n", nErrors);
	std::cout << "Overall time     : " << time_sec    << " seconds" << std::endl;
	std::cout << "Execution time   : " << exec_sec    << " seconds" << std::endl;
	std::cout << "Throughput coded : " << thgt_mbps_n << " Mbps" << std::endl;
	std::cout << "Throughput info. : " << thgt_mbps_k << " Mbps" << std::endl;

	printf("FER = %1.3e\n", FER);

	return EXIT_SUCCESS;
}
