#include "Decoder_functions.h"
#include "channel.h"
#include "definitions/code.hpp"
#include "struct.h"
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

int main(int argc, char *argv[])
{

  if (argc < 5)
  {
    std::cerr << "Usage: " << argv[0]
              << " <frames> <SNR_dB> <GF_size> <N> [entropy|probability]\n";
    return EXIT_FAILURE;
  }

  string dec_type;

  string mode = "entrop";
  if (argc > 5)
  {
    mode = argv[5];
  }
  uint16_t q, N, n, frozen_val = 0;
  uint64_t NbMonteCarlo = stoi(argv[1]);
  float EbN0 = stod(argv[2]);
  q = stoi(argv[3]);
  N = stoi(argv[4]);

  if (q != _GF_)
  {
    std::cerr << "This executable was compiled for GF=" << _GF_
              << "; received GF=" << q << ".\n";
    return EXIT_FAILURE;
  }

  n = log2(N);

  vector<uint16_t> reliability_order(N);
  vector<int> frozen_symbols(N, false);
  for (int i = 0; i < N; i += 1)
  {
    reliability_order[i] = i;
  }

  unsigned base_seed =
      0; // std::chrono::system_clock::now().time_since_epoch().count();

  int max_threads = omp_get_max_threads();
  vector<vector<float>> thread_entrop(max_threads, vector<float>(N, 0.0f));
  vector<vector<float>> thread_one_err_prob(max_threads,
                                            vector<float>(N, 0.0f));
  vector<vector<int>> thread_succ(max_threads, vector<int>(N, 0));
  std::atomic<uint64_t> frame_counter(0);

#pragma omp parallel
  {
    int thread_id = omp_get_thread_num();
    std::mt19937 gen(thread_id + base_seed);
    vector<decoder_t> L(N);

    vector<uint16_t> KSYMB(N);
    vector<uint16_t> decoded_n(N);
    vector<float> entrop(N);
    vector<float> one_err_prob(N);

    decoder *dec;
    dec = new decoder_naive<_GF_>(N, frozen_symbols.data());

    std::vector<symbols_s<_GF_>> llrs_n(N);

#pragma omp for
    for (int n_frame = 0; n_frame < NbMonteCarlo; n_frame++)
    {

      bool succ_dec[N] = {false};

      EncodeChanBPSK_BinCCSK(gen, N, q, n, frozen_val, reliability_order,
                             EbN0, L, KSYMB);

      for (int i = 0; i < N; i++)
      {
        for (int j = 0; j < _GF_; j++)
          llrs_n[i].value[j] = L[i].intrinsic_LLR[j];
      }

      dec->execute(llrs_n.data(), decoded_n.data(), KSYMB.data(), entrop.data(),
                   one_err_prob.data());
      for (int i = 0; i < N; i++)
      {
        if (decoded_n[i] == KSYMB[i])
        {
          succ_dec[i] = true;
        }
      }

      // Accumulate per thread
      for (int i = 0; i < N; i++)
      {
        thread_entrop[thread_id][i] += entrop[i];
        thread_one_err_prob[thread_id][i] += one_err_prob[i];
        if (succ_dec[i])
        {
          thread_succ[thread_id][i]++;
        }
      }

      frame_counter++;
      if (frame_counter % 1000 == 0)
      {
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
  for (int t = 0; t < max_threads; t++)
  {
    for (int i = 0; i < N; i++)
    {
      global_entrop[i] += thread_entrop[t][i];
    }
    for (int i = 0; i < N; i++)
    {
      global_succ[i] += thread_succ[t][i];
    }
    for (int i = 0; i < N; i++)
    {
      global_one_err_prob[i] += thread_one_err_prob[t][i];
    }
  }
  for (int i = 0; i < N; i++)
  {
    global_entrop[i] /= static_cast<float>(NbMonteCarlo);
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
  for (int i = 0; i < N; i++)
  {
    out1 << i << "   " << std::scientific << std::setprecision(10)
         << global_one_err_prob[i] << std::scientific << std::setprecision(10)
         << "   " << global_entrop[i] << "   " << std::scientific
         << std::setprecision(7) << global_succ[i] << "\n";
  }
  out1.close();

  std::vector<std::pair<float, int>> one_err_pairs;
  for (int i = N - 1; i >= 0; i--)
  {
    one_err_pairs.push_back({global_one_err_prob[i], i});
  }
  std::sort(one_err_pairs.begin(), one_err_pairs.end(),
            [](const std::pair<float, int> &a, const std::pair<float, int> &b)
            {
              if (a.first != b.first)
                return a.first < b.first; // Ascending error prob
              return a.second > b.second; // Descending index for ties
            });
  std::vector<std::pair<float, int>> entrop_pairs;
  for (int i = N - 1; i >= 0; i--)
  {
    entrop_pairs.push_back({global_entrop[i], i});
  }
  std::sort(entrop_pairs.begin(), entrop_pairs.end(),
            [](const std::pair<float, int> &a, const std::pair<float, int> &b)
            {
              if (a.first != b.first)
                return a.first < b.first; // Ascending entropy
              return a.second > b.second; // Descending index for ties
            });

  std::ofstream out2(file2);
  std::string lower_mode = mode;
  std::transform(lower_mode.begin(), lower_mode.end(), lower_mode.begin(),
                 ::tolower);
  std::string mode_string =
      (lower_mode == "probability") ? "probability" : "entropy";
  out2 << "SNR " << std::fixed << std::setprecision(3) << std::setw(6) << EbN0
       <<  "\n";
  if (lower_mode == "probability")
  {
    for (int i = 0; i < N; i++)
    {
      out2 << std::setw(6) << one_err_pairs[i].second << " ";
    }
  }
  else
  {
    for (int i = 0; i < N; i++)
    {
      out2 << std::setw(6) << entrop_pairs[i].second << " ";
    }
  }
  out2 << "\n";
  out2.close();
}
