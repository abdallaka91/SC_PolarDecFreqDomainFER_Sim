#include "Decoder_functions.h"
#include "GF_tools.h"
#include "HelperFunc.h"
#include "channel.h"
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

#include "decoders/basic/decoder_basic.hpp"
// #include "decoders/dedicated/decoder_dedicated.hpp"
#include "decoders/naive/decoder_naive.hpp"
#include "decoders/naive_cfloat/decoder_naive_cfloat.hpp"
#include "decoders/naive_fixed/decoder_naive_fixed.hpp"
#include "decoders/naive_int32_t/decoder_naive_int32_t.hpp"
#include "decoders/naive_integer/decoder_naive_integer.hpp"
#include "decoders/specialized/decoder_specialized.hpp"
#include "decoders/specialized_pruning/decoder_specialized_pruning.hpp"
#include "demodulator/demodulator.hpp"
#include "encoder/polar_encoder.hpp"
#include "features/fwht/fwht_counter.hpp"

#include "utilities/utility_functions.hpp"

using namespace PoAwN::structures;
using namespace PoAwN::tools;
using namespace PoAwN::init;
using namespace PoAwN::decoding;
using namespace PoAwN::channel;
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
                             unsigned long nb_gen_frame) {
  // Directory path
  fs::path dir = "results";

  // Create directory if not exists
  std::error_code ec;
  if (!fs::exists(dir)) {
    if (!fs::create_directories(dir, ec)) {
      std::cerr << "Error creating directory " << dir << ": " << ec.message()
                << "\n";
      return;
    }
  }

  // Compose filename
  fs::path filename =
      dir / ("GF" + std::to_string(GFx) + "_N" + std::to_string(Nx) + "_K" +
             std::to_string(Kx) + "_" + dec.c_str() + ".txt");

  // Open file in append mode
  FILE *fou = fopen(filename.c_str(), "a");

  if (fou == nullptr) {
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
                            unsigned long nb_gen_frame) {
  // Directory path
  fs::path dir = "results";

  // Create directory if not exists
  std::error_code ec;
  if (!fs::exists(dir)) {
    if (!fs::create_directories(dir, ec)) {
      std::cerr << "Error creating directory " << dir << ": " << ec.message()
                << "\n";
      return;
    }
  }

  // Compose filename
  fs::path filename =
      dir / (modulation + "_GF" + std::to_string(GFx) + "_N" +
             std::to_string(Nx) + "_K" + std::to_string(Kx) + ".txt");

  // Open file in append mode
  std::ofstream file(filename, std::ios::app);
  if (!file.is_open()) {
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

int main(int argc, char *argv[]) {
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

  if (argc < 7) {
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

  // FWHT counter

  int FER_STOP = 25;
  if (argc == 8)
    FER_STOP = stoi(argv[7]);
  // N = 1024;
  // K = 513;
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
  // void LoadTables(base_code_t & code, table_GF & table,  const uint16_t
  // *GF_polynom_primitive)

  cout << "(II) Loading tables [START]" << endl;
  LoadTables(code_param, table, GF_polynom_primitive.data());
  cout << "(II) Loading tables [END OK]" << endl;

  cout << EVAL(FWHT) " and " EVAL(FWHT_NORM) " are used for FWHT operations."
       << endl;

  cout << "Simulation starts..." << endl;

  decoder_parameters dec_param(code_param);

  printf("\n");
  CCSK_seq ccsk_seq;
  vector<vector<uint16_t>> CCSK_rotated_codes(q, vector<uint16_t>());
  if (code_param.sig_mod == "CCSK_BIN")
    create_ccsk_rotated_table(ccsk_seq.CCSK_bin_seq[code_param.p - 2],
                              ccsk_seq.CCSK_bin_seq[code_param.p - 2].size(),
                              CCSK_rotated_codes);
  else if (code_param.sig_mod == "CCSK_NB")
    create_ccsk_rotated_table(ccsk_seq.CCSK_GF_seq[code_param.p - 2],
                              ccsk_seq.CCSK_GF_seq[code_param.p - 2].size(),
                              CCSK_rotated_codes);

  vector<vector<vector<int16_t>>> hst1(
      n, vector<vector<int16_t>>(N, vector<int16_t>(dec_param.nm, 0)));

  q = code_param.q;
  p = code_param.p;
  vector<vector<softdata_t>> bin_mod_dict;
  if (code_param.sig_mod == "CCSK_BIN") {
    bin_mod_dict.resize(q, vector<softdata_t>(q, 0));

    for (int i = 0; i < q; i++)
      for (int j = 0; j < q; j++)
        bin_mod_dict[i][j] = (CCSK_rotated_codes[i][j] == 0) ? 1 : -1;
  }

  dec_param.ucap.resize(n + 1, vector<uint16_t>(N, dec_param.MxUS));
  dec_param.ucap[n].assign(N, dec_param.frozen_val);
  uint64_t FER_out = 0, gen_frames_out = 0;
  std::atomic<int> global_counter(0);
  std::atomic<int> FER(0);
  std::atomic<bool> stop(false);
  unsigned base_seed =
      0; // std::chrono::system_clock::now().time_since_epoch().count();

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //

  int frozen_symbols[N];
  for (int i = 0; i < N; i += 1)
    frozen_symbols[i] = true;
  for (int i = 0; i < K; i += 1)
    frozen_symbols[dec_param.reliab_sequence[i]] = false;
  //
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  double time_base[64];
  for (int i = 0; i < 64; i += 1)
    time_base[i] = 0.0;

  const auto s_start = std::chrono::system_clock::now();

  if (dec_type == "dec1") {
    printf("Simulate decoder 1...\n");
  } else if (dec_type == "dec1_int32") {
    printf("Simulate decoder 1 int32...\n");
  } else if (dec_type == "dec1_integer") {
    printf("Simulate decoder 1 integer...\n");
  } else if (dec_type == "dec1_fixed") {
    printf("Simulate decoder 1 fixed...\n");
  } else if (dec_type == "dec1_cfloat") {
  } else if (dec_type == "dec3") {
    printf("Simulate decoder 3...\n");
  } else if (dec_type == "dec4") {
    printf("Simulate decoder 4...\n");
  } else if (dec_type == "dec0") {
    printf("Simulate decoder 0...\n");
  } else {
    printf("#(II) Error : unknown decoder type\n");
    exit(1);
  }

#pragma omp parallel
  {
#pragma omp single
    printf("Used threads = %d\n", omp_get_num_threads());
    PoAwN::structures::decoder_parameters dec_param_local = dec_param;
    int thread_id = omp_get_thread_num();
    std::mt19937 gen(thread_id + base_seed);
    vector<vector<decoder_t>> L(n + 1, vector<decoder_t>(N));

    for (int i = 0; i <= n; i++)
      for (int j = 0; j < N; j++) {
        L[i][j].intrinsic_LLR.resize(q, 0);
        L[i][j].is_freq = false;
      }

    vector<uint16_t> info_sec_rec(K, dec_param_local.MxUS);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    std::vector<uint16_t> decoded_n(N);

    decoder *dec;
    if (dec_type == "dec1") {
      dec = new decoder_naive<_GF_>(N, frozen_symbols);
    } else if (dec_type == "dec1_int32") {
      dec = new decoder_naive_int32_t<_GF_>(N, frozen_symbols);
    } else if (dec_type == "dec1_integer") {
      dec = new decoder_naive_integer<_GF_>(N, frozen_symbols);
    } else if (dec_type == "dec1_fixed") {
      dec = new decoder_naive_fixed<_GF_>(N, frozen_symbols);
    } else if (dec_type == "dec1_cfloat") {
      dec = new decoder_naive_cfloat<_GF_>(N, frozen_symbols);
    } else if (dec_type == "dec3") {
      dec = new decoder_specialized<_GF_>(N, frozen_symbols);
    } else if (dec_type == "dec4") {
      dec = new decoder_specialized_pruning<_GF_>(N, frozen_symbols);
    } else if (dec_type == "dec0") {
      dec = new decoder_basic<_GF_>(N, frozen_symbols);
    } else {
      exit(1);
    }

    std::vector<symbols_s<_GF_>> llrs_n(N);

    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    while (true) {

      bool succ_dec = true;
      vector<uint16_t> KSYMB(K);

      EncodeChanBPSK_BinCCSK(gen, dec_param_local, table, EbN0,
                             CCSK_rotated_codes, L[0], KSYMB, bin_mod_dict);

      for (int i = 0; i < N; i++) {
        for (int j = 0; j < _GF_; j++)
          llrs_n[i].value[j] = L[0][i].intrinsic_LLR[j];
      }

      const auto m_start = std::chrono::system_clock::now();
      dec->execute(llrs_n.data(), decoded_n.data());
      const auto m_stop = std::chrono::system_clock::now();
      time_base[thread_id] +=
          std::chrono::duration_cast<std::chrono::microseconds>(m_stop -
                                                                m_start)
              .count();

      for (int i = 0; i < K; i++)
        info_sec_rec[i] = decoded_n[dec_param.reliab_sequence[i]];

      for (uint16_t i = 0; i < dec_param_local.K; i++) {
        if (KSYMB[i] != info_sec_rec[i]) {
          succ_dec = false;
          break;
        }
      }

      global_counter.fetch_add(1);
      int succ_now = global_counter.load() - FER.load();
      if (!succ_dec) {
        FER.fetch_add(1);
      }
      succ_now = global_counter.load() - FER.load();
      if ((global_counter % 1000) == 0) {

#pragma omp critical
        {
          int local_success = global_counter.load() - FER.load();
          if ((global_counter.load() >= NbMonteCarlo) ||
              (FER.load() >= FER_STOP))
            stop.store(true); // Set the flag
          FER_out = FER.load();
          gen_frames_out = global_counter.load();
          cout << "\rSNR: " << EbN0 << " dB, FER = " << FER << "/"
               << global_counter << " = " << (float)FER_out / gen_frames_out
               << std::flush;
        }
      }
      if (stop.load())
        break;
    }
  }
  const auto s_stop = std::chrono::system_clock::now();
  const int tSimuSec =
      std::chrono::duration_cast<std::chrono::seconds>(s_stop - s_start)
          .count();

  double total_us = 0.0;
  for (int i = 0; i < 64; i += 1)
    total_us = (total_us >= time_base[i]) ? total_us : time_base[i];
  const float time_run = (total_us / (double)gen_frames_out);
  const float debit = ((double)N * (double)_logGF_) / time_run;

  cout << "\rSNR: " << EbN0 << " dB, FER = " << FER_out << "/" << gen_frames_out
       << " = " << (float)FER_out / (float)gen_frames_out << std::flush;
  cout << " :: débit = " << debit << " Mbps";
  cout << endl;

  append_results_to_file1(dec_type, dec_param.q, dec_param.N, dec_param.K, EbN0,
                          FER_out, gen_frames_out);
}