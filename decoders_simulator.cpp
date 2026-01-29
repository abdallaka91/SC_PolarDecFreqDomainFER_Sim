#include "GF_tools.h"
#include "definitions/code.hpp"
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
#include <iomanip>
#include <iostream>
#include <memory>
#include <omp.h>
#include <random>
#include <string>
#include <vector>

#include "./ccsk_simulator/ccsk_simulator.hpp"
#include <iostream>
#include <chrono>

#include "decoders/basic/decoder_basic.hpp"
// #include "decoders/dedicated/decoder_dedicated.hpp"
#include "decoders/naive/decoder_naive.hpp"
#include "decoders/naive_cfloat/decoder_naive_cfloat.hpp"
#include "decoders/naive_fixed/decoder_naive_fixed.hpp"
// #include "decoders/naive_int32_t/decoder_naive_int32_t.hpp"
#include "decoders/specialized/decoder_specialized.hpp"
#include "decoders/specialized_pruning/decoder_specialized_pruning.hpp"
#include "demodulator/demodulator.hpp"
#include "encoder/encoder_1.hpp"
#include "features/fwht/fwht_counter.hpp"

#include "utilities/utility_functions.hpp"

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

void append_results_to_file1(const std::string &dec, int GFx, int Nx, int Kx,
                             double SNR, unsigned long nb_err,
                             unsigned long nb_gen_frame)
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

  double FER_value =
      (nb_gen_frame == 0) ? 0.0 : static_cast<double>(nb_err) / nb_gen_frame;

  fprintf(fou, "%+7.3f %1.8f %6d %8d", SNR, FER_value, nb_err, nb_gen_frame);
  fprintf(fou, "\n");
  fclose(fou);
}

void append_results_to_file(const std::string &modulation, int GFx, int Nx,
                            int Kx, double SNR, unsigned long nb_err,
                            unsigned long nb_gen_frame)
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
      dir / (modulation + "_GF" + std::to_string(GFx) + "_N" +
             std::to_string(Nx) + "_K" + std::to_string(Kx) + ".txt");

  std::ofstream file(filename, std::ios::app);
  if (!file.is_open())
  {
    std::cerr << "Error opening file " << filename << " for appending.\n";
    return;
  }

  double FER_value =
      (nb_gen_frame == 0) ? 0.0 : static_cast<double>(nb_err) / nb_gen_frame;

  file << "SNR=" << SNR << " db,    FER = " << nb_err << "/" << nb_gen_frame
       << " = " << FER_value << "\n";
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

  string dec_type;

  if (argc < 7)
  {
    cout << "validate: NbMonteCarlo, SNR, q, N, K, dec1...dec5 and optionnally "
            "nb of FER"
         << std::endl;
    return 1;
  }
  uint16_t q, N, K, n, p, frozen_val = 0;
  softdata_t offset;
  uint64_t NbMonteCarlo = stoi(argv[1]);
  float EbN0 = stod(argv[2]);
  q = stoi(argv[3]);
  p = log2(q);
  N = stoi(argv[4]);
  K = stoi(argv[5]);
  dec_type = std::string(argv[6]);
  std::transform(dec_type.begin(), dec_type.end(), dec_type.begin(), ::tolower);

  int num_threads = omp_get_max_threads();
  std::cout << "Using " << num_threads << " threads" << std::endl;

  int FER_STOP = 25;
  if (argc == 8)
    FER_STOP = stoi(argv[7]);

  n = log2(N);

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

  const auto s_start = std::chrono::system_clock::now();

  if (dec_type == "dec1")
  {
    printf("Simulate decoder 1...\n");
    // } else if (dec_type == "dec1_int32") {
    //   dec = new decoder_naive_int32_t<_GF_>(N, frozen_symbols);
  }
  else if (dec_type == "dec1_fixed")
  {
    printf("Simulate decoder 1 fixed...\n");
  }
  else if (dec_type == "dec1_cfloat")
  {
  }
  else if (dec_type == "dec3")
  {
    printf("Simulate decoder 3...\n");
  }
  else if (dec_type == "dec4")
  {
    printf("Simulate decoder 4...\n");
  }
  else if (dec_type == "dec0")
  {
    printf("Simulate decoder 0...\n");
  }
  else
  {
    printf("#(II) Error : unknown decoder type\n");
    exit(1);
  }

  float sigma = sqrt(1.0 / (pow(10, EbN0 / 10.0)));
  CCSK_Simulator<_GF_, _N_> simulator(sigma, sigma, num_threads);

  std::atomic<uint64_t> frame_errors(0);
  std::atomic<uint64_t> frames_simulated(0);

  uint64_t FER_out = 0, gen_frames_out = 0;
  std::atomic<int> global_counter(0);
  std::atomic<int> FER(0);
  std::atomic<bool> stop(false);

  auto start = std::chrono::high_resolution_clock::now();

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
    std::vector<symbols_s<_GF_>> llrs_n(N);
    std::vector<uint16_t> decoded_n(N);

    // Initialize decoder
    decoder *dec = nullptr;
    if (dec_type == "dec1")
    {
      dec = new decoder_naive<_GF_>(N, frozen_symbols);
    }
    else if (dec_type == "dec1_fixed")
    {
      dec = new decoder_naive_fixed<_GF_>(N, frozen_symbols);
    }
    else if (dec_type == "dec1_cfloat")
    {
      dec = new decoder_naive_cfloat<_GF_>(N, frozen_symbols);
    }
    else if (dec_type == "dec3")
    {
      dec = new decoder_specialized<_GF_>(N, frozen_symbols);
    }
    else if (dec_type == "dec4")
    {
      dec = new decoder_specialized_pruning<_GF_>(N, frozen_symbols);
    }
    else if (dec_type == "dec0")
    {
      dec = new decoder_basic<_GF_>(N, frozen_symbols);
    }
    else
    {
#pragma omp critical
      {
        std::cerr << "Error: Unknown decoder type: " << dec_type << std::endl;
      }
      exit(1);
    }
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
      polar_encode<_N_>(u_symb);

      // Simulate CCSK transmission
      double *llr_values = simulator.simulate_frame(u_symb, thread_id);
      simulator.llr_to_probability<_GF_>(llr_values, N);

      // Convert to decoder format
      for (int i = 0; i < N; i++)
      {
        for (int j = 0; j < _GF_; j++)
          llrs_n[i].value[j] = llr_values[i * _GF_ + j];
      }

      // Decode
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
      int succ_now = global_counter.load() - FER.load();
      if (!succ_dec)
      {
        FER.fetch_add(1);
      }
      succ_now = global_counter.load() - FER.load();
      if ((global_counter % 1000) == 0)
      {

#pragma omp critical
        {
          int local_success = global_counter.load() - FER.load();
          if ((global_counter.load() >= NbMonteCarlo) ||
              (FER.load() >= FER_STOP))
            stop.store(true);
          FER_out = FER.load();
          gen_frames_out = global_counter.load();

          std::cout << "\rSNR: " << std::fixed << std::setprecision(1) << EbN0
                    << " dB, FER = " << std::setw(8) << FER_out
                    << "/" << std::setw(8) << gen_frames_out
                    << " = " << std::setprecision(6)
                    << (double)FER_out / gen_frames_out
                    << std::flush;
        }
      }
      if (stop.load())
        break;
    }

    delete dec;
  }

  auto end = std::chrono::high_resolution_clock::now();
  double sec = std::chrono::duration<double>(end - start).count();

  std::cout << "\rSNR: " << std::fixed << std::setprecision(1) << EbN0
            << " dB, FER = " << std::setw(8) << FER_out
            << "/" << std::setw(8) << gen_frames_out
            << " = " << std::setprecision(6)
            << (double)FER_out / gen_frames_out
            << std::flush;

  // append_results_to_file1(dec_type, _GF_, _N_, K, EbN0,
  //                         FER_out, gen_frames_out);

  // Final results
  std::cout << "\nPolar Code: N=" << N << ", K=" << K << ", GF=" << _GF_ << std::endl;
  std::cout << "Decoder: " << dec_type << std::endl;
  std::cout << "Eb/N0: " << EbN0 << " dB, Sigma: " << sigma << std::endl;
  std::cout << "Actual frames: " << gen_frames_out << std::endl;
  std::cout << "Time: " << sec << " seconds" << std::endl;
  std::cout << "Throughput: " << gen_frames_out / sec << " fps" << std::endl;
  std::cout << "Throughput info: " << (gen_frames_out * K * _logGF_) / sec / 1e6 << " Mbps ( bits/symbol)" << std::endl;
}