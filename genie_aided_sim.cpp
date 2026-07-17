#include "ccsk_simulator/ccsk_simulator.hpp"
#include "ccsk_simulator/simul_parameters.hpp"
#include "definitions/code.hpp"
#include "decoders/naive/decoder_naive.hpp"
#include "encoder/encoder_1.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
  if (argc < 5)
  {
    std::cerr << "Usage: " << argv[0]
              << " <frames> <SNR_dB> <GF_size> <N> [entropy|probability]\n";
    return EXIT_FAILURE;
  }

  const uint64_t NbMonteCarlo = std::stoull(argv[1]);
  const float EbN0 = std::stof(argv[2]);
  const uint16_t q = std::stoi(argv[3]);
  const uint16_t N = std::stoi(argv[4]);
  const std::string mode = (argc > 5) ? argv[5] : "entropy";

  if (q != _GF_ || N != _N_)
  {
    std::cerr << "This executable was compiled for GF=" << _GF_
              << ", N=" << _N_ << "; received GF=" << q << ", N=" << N
              << ".\n";
    return EXIT_FAILURE;
  }

  constexpr uint16_t K = _N_;
  std::vector<uint16_t> reliability_order(_N_);
  std::vector<int> frozen_symbols(_N_, false);
  for (uint16_t i = 0; i < _N_; ++i)
    reliability_order[i] = i;

  const float noise_sigma = std::sqrt(1.0f / std::pow(10.0f, EbN0 / 10.0f));
  const float llr_sigma = noise_sigma;
  const int max_threads = omp_get_max_threads();
  CCSK_Simulator<_GF_, _N_> simulator(noise_sigma, llr_sigma, max_threads);

  std::vector<std::vector<float>> thread_entrop(max_threads, std::vector<float>(_N_, 0.0f));
  std::vector<std::vector<float>> thread_one_err_prob(max_threads, std::vector<float>(_N_, 0.0f));
  std::vector<std::vector<int>> thread_succ(max_threads, std::vector<int>(_N_, 0));
  std::atomic<uint64_t> frame_counter(0);

#pragma omp parallel
  {
    const int thread_id = omp_get_thread_num();
    uint16_t K_symb[K];
    uint16_t u_symb[_N_];
    uint16_t true_u_symb[_N_];
    std::vector<uint16_t> decoded_n(_N_);
    std::vector<float> entrop(_N_);
    std::vector<float> one_err_prob(_N_);
    std::vector<symbols_s<_GF_>> llrs_n(_N_);

    decoder_naive<_GF_> dec(_N_, frozen_symbols.data());

#pragma omp for
    for (uint64_t n_frame = 0; n_frame < NbMonteCarlo; ++n_frame)
    {
      simulator.generate_random_symbols(K_symb, K, thread_id);

      for (uint16_t u = 0; u < K; ++u)
        u_symb[reliability_order[u]] = K_symb[u];

      for (uint16_t u = 0; u < _N_; ++u)
        true_u_symb[u] = u_symb[u];

      polar_encode<_N_>(u_symb);

      double *llr_values = simulator.simulate_frame(u_symb, thread_id);
      simulator.llr_to_probability<_GF_>(llr_values, _N_);

      for (uint16_t i = 0; i < _N_; ++i)
        for (uint16_t j = 0; j < _GF_; ++j)
          llrs_n[i].value[j] = static_cast<float>(llr_values[i * _GF_ + j]);

      dec.execute(llrs_n.data(), decoded_n.data(), true_u_symb, entrop.data(),
                  one_err_prob.data());

      for (uint16_t i = 0; i < _N_; ++i)
      {
        thread_entrop[thread_id][i] += entrop[i];
        thread_one_err_prob[thread_id][i] += one_err_prob[i];
        if (decoded_n[i] == true_u_symb[i])
          thread_succ[thread_id][i]++;
      }

      const uint64_t processed = ++frame_counter;
      if (processed % 1000 == 0)
      {
        std::printf("Processed %lu frames\r", processed);
        std::fflush(stdout);
      }
    }
  }

  std::printf("\n");

  std::vector<float> global_entrop(_N_, 0.0f);
  std::vector<float> global_one_err_prob(_N_, 0.0f);
  std::vector<int> global_succ(_N_, 0);

  for (int t = 0; t < max_threads; ++t)
  {
    for (uint16_t i = 0; i < _N_; ++i)
    {
      global_entrop[i] += thread_entrop[t][i];
      global_one_err_prob[i] += thread_one_err_prob[t][i];
      global_succ[i] += thread_succ[t][i];
    }
  }

  for (uint16_t i = 0; i < _N_; ++i)
  {
    global_entrop[i] /= static_cast<float>(NbMonteCarlo);
    global_one_err_prob[i] /= static_cast<float>(NbMonteCarlo);
  }

  const std::string folder1 = "entropies_probabilities/GF" + std::to_string(q) + "/";
  fs::create_directories(folder1);
  const std::string file1 = folder1 + "GF" + std::to_string(q) + "N" + std::to_string(N) + ".txt";

  std::ofstream out1(file1);
  for (uint16_t i = 0; i < _N_; ++i)
  {
    out1 << i << "   " << std::scientific << std::setprecision(10)
         << global_one_err_prob[i] << "   " << global_entrop[i] << "   "
         << global_succ[i] << "\n";
  }

  std::vector<std::pair<float, int>> one_err_pairs;
  std::vector<std::pair<float, int>> entrop_pairs;
  for (int i = _N_ - 1; i >= 0; --i)
  {
    one_err_pairs.push_back({global_one_err_prob[i], i});
    entrop_pairs.push_back({global_entrop[i], i});
  }

  auto sort_reliability = [](const std::pair<float, int> &a,
                             const std::pair<float, int> &b)
  {
    if (a.first != b.first)
      return a.first < b.first;
    return a.second > b.second;
  };
  std::sort(one_err_pairs.begin(), one_err_pairs.end(), sort_reliability);
  std::sort(entrop_pairs.begin(), entrop_pairs.end(), sort_reliability);

  const std::string folder2 = "matrices/GF" + std::to_string(q) + "/";
  fs::create_directories(folder2);
  const std::string file2 = folder2 + "GF" + std::to_string(q) + "N" + std::to_string(N) + ".txt";

  std::string lower_mode = mode;
  std::transform(lower_mode.begin(), lower_mode.end(), lower_mode.begin(), ::tolower);

  std::ofstream out2(file2);
  out2 << "SNR " << std::fixed << std::setprecision(3) << std::setw(6) << EbN0 << "\n";
  const auto &selected = (lower_mode == "probability") ? one_err_pairs : entrop_pairs;
  for (uint16_t i = 0; i < _N_; ++i)
    out2 << std::setw(6) << selected[i].second << " ";
  out2 << "\n";
}
