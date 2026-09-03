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
#include <limits>
#include <omp.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

using generator_matrix_t = std::vector<std::vector<uint8_t>>;

struct ReliabilitySnapshot
{
  std::vector<double> entropy;
  std::vector<double> one_error_probability;
};

struct ShorteningStep
{
  int depth;
  std::vector<uint16_t> candidates;
  uint16_t selected;
  double selected_entropy;
  double selected_one_error_probability;
};

struct BlockCandidate
{
  uint16_t root_column;
  std::vector<uint16_t> indices;
  double summed_entropy;
  double summed_one_error_probability;
};

struct BlockShorteningStep
{
  int iteration;
  int starting_depth;
  BlockCandidate selected;
  size_t candidate_count;
};

generator_matrix_t build_polar_generator(const int N)
{
  generator_matrix_t generator(N, std::vector<uint8_t>(N, 0));

  // For G_N = [[1,0],[1,1]]^(Kronecker log2(N)), G[row][column] is
  // one exactly when every set bit of column is also set in row.
  for (int row = 0; row < N; ++row)
    for (int column = 0; column < N; ++column)
      generator[row][column] = ((column & ~row) == 0);

  return generator;
}

void validate_shortening_state(
    const generator_matrix_t &generator, const std::vector<bool> &active,
    const std::vector<uint16_t> &shortened_positions)
{
  const size_t active_count = std::count(active.begin(), active.end(), true);
  if (active_count + shortened_positions.size() != active.size())
    throw std::runtime_error("Active and shortened index sets do not partition N");

  for (const uint16_t column : shortened_positions)
  {
    if (active[column])
      throw std::runtime_error("A shortened index is still active");

    // The active mask defines the reduced transform used to construct the
    // nested shortening chain.
    for (size_t row = 0; row < active.size(); ++row)
      if (active[row] && generator[row][column])
        throw std::runtime_error(
            "A shortened output still depends on an active input");
  }
}

std::vector<uint16_t>
find_weight_one_candidates(const generator_matrix_t &generator,
                           const std::vector<bool> &active)
{
  const int N = active.size();
  std::vector<uint16_t> candidates;

  for (int column = 0; column < N; ++column)
  {
    if (!active[column])
      continue;

    int weight = 0;
    int unique_row = -1;
    for (int row = 0; row < N; ++row)
    {
      if (active[row] && generator[row][column])
      {
        ++weight;
        unique_row = row;
        if (weight > 1)
          break;
      }
    }

    if (weight == 1)
    {
      if (unique_row != column)
        throw std::runtime_error(
            "Weight-one column does not map to the same-index input");
      candidates.push_back(static_cast<uint16_t>(column));
    }
  }

  if (candidates.empty())
    throw std::runtime_error("No active weight-one shortening candidate");

  return candidates;
}

std::vector<BlockCandidate>
find_block_candidates(const generator_matrix_t &generator,
                      const std::vector<bool> &active,
                      const int remaining_shortening,
                      const ReliabilitySnapshot &snapshot)
{
  const int N = active.size();
  std::vector<BlockCandidate> candidates;

  for (int column = 0; column < N; ++column)
  {
    if (!active[column])
      continue;

    BlockCandidate candidate{static_cast<uint16_t>(column), {}, 0.0, 0.0};
    for (int row = 0; row < N; ++row)
    {
      if (!active[row] || !generator[row][column])
        continue;

      candidate.indices.push_back(static_cast<uint16_t>(row));
      candidate.summed_entropy += snapshot.entropy[row];
      candidate.summed_one_error_probability +=
          snapshot.one_error_probability[row];
    }

    const int block_weight = static_cast<int>(candidate.indices.size());
    if (block_weight > 0 && block_weight <= remaining_shortening)
      candidates.push_back(std::move(candidate));
  }

  if (candidates.empty())
    throw std::runtime_error(
        "No active shortening block fits the remaining shortening budget");

  return candidates;
}

template <int GF, int N>
ReliabilitySnapshot simulate_reliability(
    const uint64_t frame_count, CCSK_Simulator<GF, N> &simulator,
    const std::vector<bool> &active,
    const std::vector<uint16_t> &shortened_positions,
    const bool freeze_inactive_inputs)
{
  const int max_threads = omp_get_max_threads();
  std::vector<std::vector<double>> thread_entropy(
      max_threads, std::vector<double>(N, 0.0));
  std::vector<std::vector<double>> thread_one_error(
      max_threads, std::vector<double>(N, 0.0));
  std::atomic<uint64_t> processed_frames(0);

  std::vector<int> frozen_symbols(N, false);
  if (freeze_inactive_inputs)
    for (int i = 0; i < N; ++i)
      frozen_symbols[i] = !active[i];

#pragma omp parallel
  {
    const int thread_id = omp_get_thread_num();
    std::vector<uint16_t> random_symbols(N);
    std::vector<uint16_t> u_symbols(N, 0);
    std::vector<uint16_t> true_u_symbols(N, 0);
    std::vector<uint16_t> decoded(N);
    std::vector<float> entropy(N);
    std::vector<float> one_error_probability(N);
    std::vector<symbols_s<GF>> channel_probabilities(N);

    decoder_naive<GF> decoder(N, frozen_symbols.data());

#pragma omp for
    for (uint64_t frame = 0; frame < frame_count; ++frame)
    {
      simulator.generate_random_symbols(random_symbols.data(), N, thread_id);

      for (int i = 0; i < N; ++i)
        u_symbols[i] =
            freeze_inactive_inputs && !active[i] ? 0 : random_symbols[i];
      true_u_symbols = u_symbols;

      polar_encode<N>(u_symbols.data());

      double *llr_values = simulator.simulate_frame(u_symbols.data(), thread_id);
      simulator.template llr_to_probability<GF>(llr_values, N);

      for (int i = 0; i < N; ++i)
        for (int symbol = 0; symbol < GF; ++symbol)
          channel_probabilities[i].value[symbol] =
              static_cast<float>(llr_values[i * GF + symbol]);

      // In diagnostic iterative mode this is the actual random encoded symbol.
      // In conventional non-iterative mode it is zero because the associated
      // inputs have been frozen before encoding.
      for (const uint16_t position : shortened_positions)
      {
        const uint16_t encoded_symbol = u_symbols[position];
        for (int symbol = 0; symbol < GF; ++symbol)
          channel_probabilities[position].value[symbol] =
              symbol == encoded_symbol ? 1.0f : 0.0f;
      }

      decoder.execute(channel_probabilities.data(), decoded.data(),
                      true_u_symbols.data(), entropy.data(),
                      one_error_probability.data());

      for (int i = 0; i < N; ++i)
      {
        thread_entropy[thread_id][i] += entropy[i];
        thread_one_error[thread_id][i] += one_error_probability[i];
      }

      const uint64_t processed = ++processed_frames;
      if (processed % 1000 == 0)
      {
        std::printf("  processed %lu/%lu frames\r", processed, frame_count);
        std::fflush(stdout);
      }
    }
  }

  std::printf("  processed %lu/%lu frames\n", frame_count, frame_count);

  ReliabilitySnapshot snapshot{
      std::vector<double>(N, std::numeric_limits<double>::quiet_NaN()),
      std::vector<double>(N, std::numeric_limits<double>::quiet_NaN())};

  for (int i = 0; i < N; ++i)
  {
    snapshot.entropy[i] = 0.0;
    snapshot.one_error_probability[i] = 0.0;
    for (int thread = 0; thread < max_threads; ++thread)
    {
      snapshot.entropy[i] += thread_entropy[thread][i];
      snapshot.one_error_probability[i] += thread_one_error[thread][i];
    }
    snapshot.entropy[i] /= static_cast<double>(frame_count);
    snapshot.one_error_probability[i] /= static_cast<double>(frame_count);
  }

  return snapshot;
}

double reliability_metric(const ReliabilitySnapshot &snapshot,
                          const uint16_t index,
                          const std::string &mode)
{
  return mode == "probability" ? snapshot.one_error_probability[index]
                               : snapshot.entropy[index];
}

uint16_t select_weakest_candidate(const std::vector<uint16_t> &candidates,
                                  const ReliabilitySnapshot &snapshot,
                                  const std::string &mode)
{
  uint16_t weakest = candidates.front();
  for (size_t i = 1; i < candidates.size(); ++i)
  {
    const uint16_t candidate = candidates[i];
    const double candidate_metric =
        reliability_metric(snapshot, candidate, mode);
    const double weakest_metric = reliability_metric(snapshot, weakest, mode);

    if (candidate_metric > weakest_metric ||
        (candidate_metric == weakest_metric && candidate < weakest))
      weakest = candidate;
  }
  return weakest;
}

double block_reliability_metric(const BlockCandidate &candidate,
                                const std::string &mode)
{
  return mode == "probability" ? candidate.summed_one_error_probability
                               : candidate.summed_entropy;
}

const BlockCandidate &
select_weakest_block(const std::vector<BlockCandidate> &candidates,
                     const std::string &mode)
{
  const BlockCandidate *weakest = &candidates.front();
  for (size_t i = 1; i < candidates.size(); ++i)
  {
    const BlockCandidate &candidate = candidates[i];
    const double candidate_metric = block_reliability_metric(candidate, mode);
    const double weakest_metric = block_reliability_metric(*weakest, mode);

    if (candidate_metric > weakest_metric ||
        (candidate_metric == weakest_metric &&
         (candidate.indices.size() > weakest->indices.size() ||
          (candidate.indices.size() == weakest->indices.size() &&
           candidate.root_column < weakest->root_column))))
      weakest = &candidate;
  }
  return *weakest;
}

std::vector<uint16_t>
sort_all_by_reliability(const ReliabilitySnapshot &snapshot,
                        const std::string &mode)
{
  std::vector<uint16_t> order(snapshot.entropy.size());
  for (size_t i = 0; i < order.size(); ++i)
    order[i] = static_cast<uint16_t>(i);

  std::sort(order.begin(), order.end(), [&](const uint16_t a,
                                             const uint16_t b) {
    const double metric_a = reliability_metric(snapshot, a, mode);
    const double metric_b = reliability_metric(snapshot, b, mode);
    if (metric_a != metric_b)
      return metric_a < metric_b;
    return a > b;
  });
  return order;
}

std::string join_indices(const std::vector<uint16_t> &indices)
{
  std::ostringstream output;
  for (size_t i = 0; i < indices.size(); ++i)
  {
    if (i != 0)
      output << ' ';
    output << indices[i];
  }
  return output.str();
}

void write_snapshot(const fs::path &output_directory, const int GF, const int N,
                    const double snr, const uint64_t frame_count,
                    const std::string &mode, const int depth,
                    const int reliability_measurement_depth,
                    const std::vector<bool> &active,
                    const std::vector<uint16_t> &shortened_positions,
                    const std::vector<uint16_t> &candidates,
                    const ReliabilitySnapshot &snapshot,
                    const std::string &candidate_column =
                        "is_weight_one_candidate")
{
  std::ostringstream filename;
  filename << "reliability_S" << std::setw(4) << std::setfill('0') << depth
           << "_NS" << std::setw(4) << N - depth << ".txt";
  std::ofstream output(output_directory / filename.str());

  const std::vector<uint16_t> order =
      sort_all_by_reliability(snapshot, mode);
  output << "# N " << N << "\n# GF " << GF << "\n# SNR " << std::fixed
         << std::setprecision(3) << snr << "\n# frames " << frame_count
         << "\n# selection_metric " << mode << "\n# shortening_depth "
         << depth << "\n# shortened_length " << N - depth
         << "\n# reliability_measurement_depth "
         << reliability_measurement_depth
         << "\n# reliability_order\n# " << join_indices(order)
         << "\n# shortened_positions\n# "
         << join_indices(shortened_positions)
         << "\n# index average_entropy average_one_error_probability "
            "is_active "
         << candidate_column << '\n';

  for (int index = 0; index < N; ++index)
  {
    const bool is_candidate =
        std::find(candidates.begin(), candidates.end(), index) !=
        candidates.end();
    output << index << ' ' << std::scientific << std::setprecision(12)
           << snapshot.entropy[index] << ' '
           << snapshot.one_error_probability[index] << ' ' << active[index]
           << ' ' << is_candidate << '\n';
  }
}

void write_shortening_order(const fs::path &output_directory, const int GF,
                            const int N, const double snr,
                            const uint64_t frame_count,
                            const std::string &mode,
                            const std::vector<ShorteningStep> &steps)
{
  std::ofstream output(output_directory / "shortening_order.txt");
  output << "# N " << N << "\n# GF " << GF << "\n# SNR " << std::fixed
         << std::setprecision(3) << snr << "\n# frames_per_depth "
         << frame_count << "\n# selection_metric " << mode
         << "\n# step selected_index selected_one_error_probability "
            "selected_entropy candidate_count candidates\n";

  for (const ShorteningStep &step : steps)
    output << step.depth + 1 << ' ' << step.selected << ' ' << std::scientific
           << std::setprecision(12) << step.selected_one_error_probability
           << ' ' << step.selected_entropy << ' ' << step.candidates.size()
           << ' ' << join_indices(step.candidates) << '\n';

  output << "# shortening_order";
  for (const ShorteningStep &step : steps)
    output << ' ' << step.selected;
  output << '\n';
}

void write_block_shortening_order(
    const fs::path &output_directory, const int GF, const int N,
    const double snr, const uint64_t frame_count, const std::string &mode,
    const std::vector<BlockShorteningStep> &steps,
    const std::vector<uint16_t> &shortened_positions)
{
  std::ofstream output(output_directory / "shortening_order.txt");
  output << "# N " << N << "\n# GF " << GF << "\n# SNR " << std::fixed
         << std::setprecision(3) << snr << "\n# frames_per_iteration "
         << frame_count << "\n# selection_metric summed_" << mode
         << "\n# iteration starting_depth selected_root block_weight "
            "summed_one_error_probability summed_entropy candidate_count "
            "shortened_block\n";

  for (const BlockShorteningStep &step : steps)
    output << step.iteration << ' ' << step.starting_depth << ' '
           << step.selected.root_column << ' ' << step.selected.indices.size()
           << ' ' << std::scientific << std::setprecision(12)
           << step.selected.summed_one_error_probability << ' '
           << step.selected.summed_entropy << ' ' << step.candidate_count
           << ' ' << join_indices(step.selected.indices) << '\n';

  output << "# shortening_order";
  for (const uint16_t index : shortened_positions)
    output << ' ' << index;
  output << '\n';
}

int main(int argc, char *argv[])
{
  if (argc < 5 || argc > 9)
  {
    std::cerr << "Usage: " << argv[0]
              << " <frames_per_depth> <SNR_dB> <GF_size> <N> "
                 "[entropy|probability] [max_shortening] "
                 "[iterative|non-iterative|block-iterative] "
                 "[refresh_interval]\n";
    return EXIT_FAILURE;
  }

  const uint64_t frame_count = std::stoull(argv[1]);
  const float snr = std::stof(argv[2]);
  const uint16_t GF = std::stoi(argv[3]);
  const uint16_t N = std::stoi(argv[4]);
  std::string mode = argc > 5 ? argv[5] : "entropy";
  std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
  const int max_shortening = argc > 6 ? std::stoi(argv[6]) : N - 1;
  std::string strategy = argc > 7 ? argv[7] : "iterative";
  std::transform(strategy.begin(), strategy.end(), strategy.begin(),
                 ::tolower);
  const int refresh_interval = argc > 8 ? std::stoi(argv[8]) : 1;

  if (GF != _GF_ || N != _N_)
  {
    std::cerr << "This executable was compiled for GF=" << _GF_ << ", N="
              << _N_ << "; received GF=" << GF << ", N=" << N << ".\n";
    return EXIT_FAILURE;
  }
  if (mode != "entropy" && mode != "probability")
  {
    std::cerr << "Selection metric must be entropy or probability.\n";
    return EXIT_FAILURE;
  }
  if (strategy != "iterative" && strategy != "non-iterative" &&
      strategy != "block-iterative")
  {
    std::cerr << "Strategy must be iterative, non-iterative, or "
                 "block-iterative.\n";
    return EXIT_FAILURE;
  }
  if (frame_count == 0 || max_shortening < 0 || max_shortening >= N)
  {
    std::cerr << "frames must be positive and max_shortening must satisfy "
                 "0 <= max_shortening < N.\n";
    return EXIT_FAILURE;
  }
  if ((strategy == "non-iterative" || strategy == "block-iterative") &&
      max_shortening == 0)
  {
    std::cerr << strategy
              << " strategy requires a positive shortening count.\n";
    return EXIT_FAILURE;
  }
  if (refresh_interval <= 0)
  {
    std::cerr << "Refresh interval must be positive.\n";
    return EXIT_FAILURE;
  }
  if (strategy != "iterative" && refresh_interval != 1)
  {
    std::cerr << "Refresh interval applies only to iterative strategy.\n";
    return EXIT_FAILURE;
  }

  const float noise_sigma = std::sqrt(1.0f / std::pow(10.0f, snr / 10.0f));
  const int max_threads = omp_get_max_threads();
  CCSK_Simulator<_GF_, _N_> simulator(noise_sigma, noise_sigma, max_threads);

  const generator_matrix_t generator = build_polar_generator(N);
  std::vector<bool> active(N, true);
  std::vector<uint16_t> shortened_positions;
  std::vector<ShorteningStep> shortening_steps;
  std::vector<BlockShorteningStep> block_shortening_steps;

  std::ostringstream snr_text;
  snr_text << std::fixed << std::setprecision(3) << snr;
  std::string construction_name = "SNR" + snr_text.str() + "_" + mode;
  if (strategy == "non-iterative")
  {
    std::ostringstream suffix;
    suffix << "_non_iterative_S" << std::setw(4) << std::setfill('0')
           << max_shortening;
    construction_name += suffix.str();
  }
  else if (strategy == "block-iterative")
  {
    std::ostringstream suffix;
    suffix << "_block_iterative_S" << std::setw(4) << std::setfill('0')
           << max_shortening;
    construction_name += suffix.str();
  }
  else if (refresh_interval > 1)
  {
    std::ostringstream suffix;
    suffix << "_refresh_B" << std::setw(4) << std::setfill('0')
           << refresh_interval;
    construction_name += suffix.str();
  }
  const fs::path output_directory =
      fs::path("constructions") / ("GF" + std::to_string(GF)) /
      ("N" + std::to_string(N)) / construction_name;
  fs::create_directories(output_directory);

  if (strategy == "iterative")
  {
    ReliabilitySnapshot current_snapshot;
    int reliability_measurement_depth = -1;
    for (int depth = 0; depth <= max_shortening; ++depth)
    {
      std::cout << "Depth S=" << depth << ", shortened length NS="
                << N - depth << ", active inputs=" << N - depth << '\n';

      validate_shortening_state(generator, active, shortened_positions);
      const std::vector<uint16_t> candidates =
          find_weight_one_candidates(generator, active);

      const bool refresh_reliability =
          depth == 0 || depth == max_shortening ||
          depth % refresh_interval == 0;
      if (refresh_reliability)
      {
        std::cout << "  computing reliability at S=" << depth << '\n';
        current_snapshot = simulate_reliability<_GF_, _N_>(
            frame_count, simulator, active, shortened_positions, false);
        reliability_measurement_depth = depth;
      }
      else
      {
        std::cout << "  reusing reliability measured at S="
                  << reliability_measurement_depth << '\n';
      }

      write_snapshot(output_directory, GF, N, snr, frame_count, mode, depth,
                     reliability_measurement_depth, active,
                     shortened_positions, candidates, current_snapshot);

      if (depth == max_shortening)
        break;

      const uint16_t selected =
          select_weakest_candidate(candidates, current_snapshot, mode);
      shortening_steps.push_back(
          {depth, candidates, selected, current_snapshot.entropy[selected],
           current_snapshot.one_error_probability[selected]});

      std::cout << "  candidates: " << join_indices(candidates) << '\n'
                << "  selected x[" << selected << "] / u[" << selected
                << "]\n";

      active[selected] = false;
      shortened_positions.push_back(selected);
      write_shortening_order(output_directory, GF, N, snr, frame_count, mode,
                             shortening_steps);
    }
  }
  else if (strategy == "non-iterative")
  {
    std::cout << "Pass 1/2: unshortened reliability\n";
    const std::vector<uint16_t> initial_candidates =
        find_weight_one_candidates(generator, active);
    const ReliabilitySnapshot initial_snapshot =
        simulate_reliability<_GF_, _N_>(frame_count, simulator, active,
                                        shortened_positions, false);
    write_snapshot(output_directory, GF, N, snr, frame_count, mode, 0, 0,
                   active, shortened_positions, initial_candidates,
                   initial_snapshot);

    for (int depth = 0; depth < max_shortening; ++depth)
    {
      validate_shortening_state(generator, active, shortened_positions);
      const std::vector<uint16_t> candidates =
          find_weight_one_candidates(generator, active);
      const uint16_t selected =
          select_weakest_candidate(candidates, initial_snapshot, mode);
      shortening_steps.push_back(
          {depth, candidates, selected, initial_snapshot.entropy[selected],
           initial_snapshot.one_error_probability[selected]});
      active[selected] = false;
      shortened_positions.push_back(selected);
    }

    write_shortening_order(output_directory, GF, N, snr, frame_count, mode,
                           shortening_steps);
    validate_shortening_state(generator, active, shortened_positions);
    const std::vector<uint16_t> final_candidates =
        find_weight_one_candidates(generator, active);
    std::cout << "Pass 2/2: reliability after S=" << max_shortening
              << " conventional shortenings\n";
    const ReliabilitySnapshot final_snapshot =
        simulate_reliability<_GF_, _N_>(frame_count, simulator, active,
                                        shortened_positions, true);
    write_snapshot(output_directory, GF, N, snr, frame_count, mode,
                   max_shortening, max_shortening, active, shortened_positions,
                   final_candidates, final_snapshot);
  }

  else
  {
    int iteration = 0;
    while (static_cast<int>(shortened_positions.size()) <= max_shortening)
    {
      const int depth = static_cast<int>(shortened_positions.size());
      const int remaining_shortening = max_shortening - depth;
      std::cout << "Block iteration " << iteration << ", depth S=" << depth
                << ", remaining=" << remaining_shortening << '\n';

      validate_shortening_state(generator, active, shortened_positions);
      const ReliabilitySnapshot snapshot =
          simulate_reliability<_GF_, _N_>(frame_count, simulator, active,
                                          shortened_positions, true);

      std::vector<BlockCandidate> candidates;
      std::vector<uint16_t> candidate_roots;
      if (remaining_shortening > 0)
      {
        candidates = find_block_candidates(generator, active,
                                           remaining_shortening, snapshot);
        candidate_roots.reserve(candidates.size());
        for (const BlockCandidate &candidate : candidates)
          candidate_roots.push_back(candidate.root_column);
      }

      write_snapshot(output_directory, GF, N, snr, frame_count, mode, depth,
                     depth, active, shortened_positions, candidate_roots,
                     snapshot, "is_block_candidate");

      if (remaining_shortening == 0)
        break;

      BlockCandidate selected = select_weakest_block(candidates, mode);
      std::sort(selected.indices.begin(), selected.indices.end());
      block_shortening_steps.push_back(
          {iteration + 1, depth, selected, candidates.size()});

      std::cout << "  selected root x[" << selected.root_column
                << "] with current block weight " << selected.indices.size()
                << ": " << join_indices(selected.indices) << '\n';

      for (const uint16_t index : selected.indices)
      {
        if (!active[index])
          throw std::runtime_error(
              "Selected shortening block contains an inactive index");
        active[index] = false;
        shortened_positions.push_back(index);
      }

      write_block_shortening_order(
          output_directory, GF, N, snr, frame_count, mode,
          block_shortening_steps, shortened_positions);
      ++iteration;
    }
  }

  if (strategy == "block-iterative")
    write_block_shortening_order(output_directory, GF, N, snr, frame_count,
                                 mode, block_shortening_steps,
                                 shortened_positions);
  else
    write_shortening_order(output_directory, GF, N, snr, frame_count, mode,
                           shortening_steps);
  std::cout << "Construction written to " << output_directory << '\n';
  return EXIT_SUCCESS;
}
