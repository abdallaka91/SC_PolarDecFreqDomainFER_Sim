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
#include <cctype>
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
#include <utility>

#include "decoders/naive/decoder_naive.hpp"
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

#define STR(S) #S

#define EVAL(x) STR(x)

int main(int argc, char *argv[]) {

  string dec_type;

  string mode = "entrop";
  if (argc > 5) {
    mode = argv[5];
  }
  uint16_t q, N, n, p, frozen_val = 0;
  softdata_t offset;
  uint64_t NbMonteCarlo = stoi(argv[1]);
  float EbN0 = stod(argv[2]);
  q = stoi(argv[3]);
  p = log2(q);
  N = stoi(argv[4]);

  n = log2(N);

  base_code_t code_param(N, N, n, q, p, frozen_val);
  code_param.sig_mod = "CCSK_BIN";

  int gf_rand_SEED = 0;
  float nse_rand_SEED = 1.2544;
  bool repeatable_randgen = 0;

  table_GF table;
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
  unsigned base_seed =
      0; // std::chrono::system_clock::now().time_since_epoch().count();

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  dec_param.reliab_sequence.resize(N);
  int frozen_symbols[N];
  for (int i = 0; i < N; i += 1)
    frozen_symbols[i] = true;
  for (int i = 0; i < N; i += 1) {
    dec_param.reliab_sequence[i] = i;
    frozen_symbols[i] = false;
  }
  //
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  double time_base[64];
  for (int i = 0; i < 64; i += 1)
    time_base[i] = 0.0;

  const auto s_start = std::chrono::system_clock::now();

  int max_threads = omp_get_max_threads();
  vector<vector<float>> thread_entrop(max_threads, vector<float>(N, 0.0f));
  vector<vector<float>> thread_one_err_prob(max_threads,
                                            vector<float>(N, 0.0f));
  vector<vector<int>> thread_succ(max_threads, vector<int>(N, 0));
  std::atomic<uint64_t> frame_counter(0);

#pragma omp parallel
  {
    PoAwN::structures::decoder_parameters dec_param_local = dec_param;
    int thread_id = omp_get_thread_num();
    std::mt19937 gen(thread_id + base_seed);
    vector<decoder_t> L(N);

    vector<uint16_t> KSYMB(N);
    vector<uint16_t> decoded_n(N);
    vector<float> entrop(N);
    vector<float> one_err_prob(N);

    decoder *dec;
    dec = new decoder_naive<_GF_>(N, frozen_symbols);

    std::vector<symbols_s<_GF_>> llrs_n(N);

#pragma omp for
    for (int n_frame = 0; n_frame < NbMonteCarlo; n_frame++) {

      bool succ_dec[N] = {false};

      EncodeChanBPSK_BinCCSK(gen, dec_param_local, table, EbN0,
                             CCSK_rotated_codes, L, KSYMB, bin_mod_dict);

      for (int i = 0; i < N; i++) {
        for (int j = 0; j < _GF_; j++)
          llrs_n[i].value[j] = L[i].intrinsic_LLR[j];
      }

      dec->execute(llrs_n.data(), decoded_n.data(), KSYMB.data(), entrop.data(),
                   one_err_prob.data());
      for (int i = 0; i < N; i++) {
        if (decoded_n[i] == KSYMB[i]) {
          succ_dec[i] = true;
        }
      }

      // Accumulate per thread
      for (int i = 0; i < N; i++) {
        thread_entrop[thread_id][i] += entrop[i];
        thread_one_err_prob[thread_id][i] += one_err_prob[i];
        if (succ_dec[i]) {
          thread_succ[thread_id][i]++;
        }
      }

      frame_counter++;
      if (frame_counter % 1000 == 0) {
        printf("Processed %lu frames\r", frame_counter.load());
        fflush(stdout);
      }
    }
    delete dec;
  }

  printf("\n");

  // Combine thread accumulators into global
  vector<float> global_entrop(N, 0.0f);
  vector<int> global_succ(N, 0);
  vector<float> global_one_err_prob(N, 0.0f);
  for (int t = 0; t < max_threads; t++) {
    for (int i = 0; i < N; i++) {
      global_entrop[i] += thread_entrop[t][i];
    }
    for (int i = 0; i < N; i++) {
      global_succ[i] += thread_succ[t][i];
    }
    for (int i = 0; i < N; i++) {
      global_one_err_prob[i] += thread_one_err_prob[t][i];
    }
  }
  const float inv_log_q = 1.0f / logf(static_cast<float>(_GF_));

  for (int i = 0; i < N; i++) {
    global_entrop[i] /= static_cast<float>(NbMonteCarlo);
    global_entrop[i] *= inv_log_q;
    global_one_err_prob[i] /= static_cast<float>(NbMonteCarlo);
  }

  // Export to files
  std::string folder1 = "entropies_probabilities/GF" + std::to_string(q) + "/";
  fs::create_directories(folder1);
  std::string file1 =
      folder1 + "GF" + std::to_string(q) + "N" + std::to_string(N) + ".txt";

  std::string folder2 = "matrices/GF" + std::to_string(q) + "/";
  fs::create_directories(folder2);
  std::string file2 =
      folder2 + "GF" + std::to_string(q) + "N" + std::to_string(N) + ".txt";

  std::ofstream out1(file1);
  for (int i = 0; i < N; i++) {
    out1 << i << "   " << std::scientific << std::setprecision(10)
         << global_one_err_prob[i] << std::scientific << std::setprecision(10)
         << "   " << global_entrop[i] << "\n";
  }
  out1.close();

  std::vector<std::pair<float, int>> one_err_pairs;
  for (int i = N - 1; i >= 0; i--) {
    one_err_pairs.push_back({global_one_err_prob[i], i});
  }
  std::sort(one_err_pairs.begin(), one_err_pairs.end(),
            [](const std::pair<float, int> &a, const std::pair<float, int> &b) {
              if (a.first != b.first)
                return a.first < b.first; // Ascending error prob
              return a.second > b.second; // Descending index for ties
            });
  std::vector<std::pair<float, int>> entrop_pairs;
  for (int i = N - 1; i >= 0; i--) {
    entrop_pairs.push_back({global_entrop[i], i});
  }
  std::sort(entrop_pairs.begin(), entrop_pairs.end(),
            [](const std::pair<float, int> &a, const std::pair<float, int> &b) {
              if (a.first != b.first)
                return a.first < b.first; // Ascending entropy
              return a.second > b.second; // Descending index for ties
            });

  std::ofstream out2(file2, std::ios::app);
  std::string lower_mode = mode;
  std::transform(lower_mode.begin(), lower_mode.end(), lower_mode.begin(),
                 ::tolower);
  std::string mode_string =
      (lower_mode == "probability") ? "probability" : "entropy";
  out2 << "SNR " << std::fixed << std::setprecision(3) << std::setw(6) << EbN0
       << " " << mode_string << "\n";
  if (lower_mode == "probability") {
    for (int i = 0; i < N; i++) {
      out2 << std::setw(4) << one_err_pairs[i].second;
    }
  } else {
    for (int i = 0; i < N; i++) {
      out2 << std::setw(4) << entrop_pairs[i].second;
    }
  }
  out2 << "\n";
  out2.close();
}