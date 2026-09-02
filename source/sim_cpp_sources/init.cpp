#include "init.h"
#include "struct.h"
#include "tools.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

void PoAwN::init::LoadCode(PoAwN::structures::base_code_t &code, float SNR,
                           const std::string &base_dir, const bool debug)
{
  code.Rate = (float)code.K / (float)code.N;

  // ------------------------------------------------------
  // Build folder + file name
  // ------------------------------------------------------
  std::ostringstream folder;
  folder << base_dir << "GF" << code.q << "/";

  std::ostringstream fname;
  fname << folder.str() << "GF" << code.q << "N" << code.N << ".txt";

  if( debug == true )
    std::cout << "#(DD) Reading reliability file: " << fname.str() << std::endl;

  std::ifstream opfile(fname.str());
  if (!opfile)
  {
    std::cerr << "Cannot open reliability file!" << std::endl;
    exit(-1010);
  }

  // ------------------------------------------------------
  // Read SNR + sequence blocks
  // ------------------------------------------------------
  struct entry_t
  {
    float snr;
    std::vector<uint16_t> seq;
  };

  std::vector<entry_t> entries;

  while (true)
  {
    std::string tag;
    float snr_val;

    if (!(opfile >> tag >> snr_val))
      break; // End of file

    if (tag != "SNR")
    {
      std::cerr << "Invalid format: expected 'SNR'" << std::endl;
      exit(EXIT_FAILURE);
    }

    // Read reliability sequence (1 line containing N ints)
    std::vector<uint16_t> seq(code.N);
    for (int i = 0; i < code.N; i++)
    {
      int t;
      if (!(opfile >> t))
      {
        std::cerr << "Missing sequence entry at index " << i << std::endl;
        exit(EXIT_FAILURE);
      }
      seq[i] = (uint16_t)t;
    }

    entries.push_back({snr_val, seq});
  }

  if (entries.empty())
  {
    std::cerr << "No SNR entries found in file!" << std::endl;
    exit(EXIT_FAILURE);
  }

  // ------------------------------------------------------
  // Find nearest SNR to the requested SNR
  // ------------------------------------------------------
  float best_dist = 2;
  int best_idx = -1;

  for (int i = 0; i < (int)entries.size(); i++)
  {
    float d = std::abs(entries[i].snr - SNR);
    if (d < best_dist)
    {
      best_dist = d;
      best_idx = i;
    }
  }

  if (best_idx < 0)
  {
    std::cerr << "#(DD) Nearest SNR not found!" << std::endl;
    exit(EXIT_FAILURE);
  }

  if( debug == true )
    std::cout << "#(DD) Requested SNR: " << SNR
            << "  --> Using nearest SNR: " << entries[best_idx].snr
            << std::endl;

  code.reliab_sequence = entries[best_idx].seq;

  code.polar_coeff.resize(code.n, std::vector<uint16_t>(code.N / 2));
  for (int i = 0; i < code.n; i++)
  {
    for (int j = 0; j < code.N / 2; j++)
    {
      code.polar_coeff[i][j] = 1;
    }
  }
}

PoAwN::init::Mat PoAwN::init::kron(const Mat &A, const Mat &B)
{
  const int a_rows = A.size();
  const int a_cols = A[0].size();
  const int b_rows = B.size();
  const int b_cols = B[0].size();

  Mat result(a_rows * b_rows,
             std::vector<uint16_t>(a_cols * b_cols));

  for (int i = 0; i < a_rows; ++i)
    for (int j = 0; j < a_cols; ++j)
      for (int p = 0; p < b_rows; ++p)
        for (int q = 0; q < b_cols; ++q)
          result[i * b_rows + p][j * b_cols + q] = A[i][j] * B[p][q];

  return result;
}

std::vector<uint16_t>
PoAwN::init::shortened_sequence(const std::vector<uint16_t> &reliab_seq,
                                uint16_t N)
{
  Mat kernel = {{1, 0}, {1, 1}};
  Mat transform = {{1}};

  for (int level = 0; level < std::log2(N); ++level)
    transform = kron(transform, kernel);

  std::vector<uint16_t> sequence(N);

  for (int iter = 0; iter < N; ++iter)
  {
    std::vector<int> candidates;
    candidates.reserve(N);

    for (int col = 0; col < N; ++col)
    {
      int column_sum = 0;
      for (int row = 0; row < N; ++row)
        column_sum += transform[row][col];

      if (column_sum == 1)
        candidates.push_back(col);
    }

    int best_idx = candidates[0];
    uint16_t best_value = reliab_seq[best_idx];

    for (size_t k = 1; k < candidates.size(); ++k)
    {
      const int idx = candidates[k];
      if (reliab_seq[idx] < best_value)
      {
        best_value = reliab_seq[idx];
        best_idx = idx;
      }
    }

    sequence[iter] = best_idx;

    for (int col = 0; col < N; ++col)
      transform[best_idx][col] = 0;
    for (int row = 0; row < N; ++row)
      transform[row][best_idx] = 0;
  }

  return sequence;
}

std::vector<uint16_t>
PoAwN::init::froz_from_short(int N,
                             const std::vector<uint16_t> &reliab_seq,
                             int frozen_count,
                             std::vector<uint16_t> short_pos)
{
  const int n_short = short_pos.size();
  const int n = std::log2(N);

  Mat kernel = {{1, 0}, {1, 1}};
  Mat generator = kernel;
  for (int level = 1; level < n; ++level)
    generator = kron(generator, kernel);

  std::sort(short_pos.begin(), short_pos.end());

  std::vector<uint16_t> affected_rows(N, 0);
  for (int idx = 0; idx < n_short; ++idx)
  {
    const int col = short_pos[idx];
    for (int row = 0; row < N; ++row)
      affected_rows[row] |= generator[row][col];
  }

  std::vector<uint16_t> frozen_positions;
  for (int pos = 0; pos < N; ++pos)
    if (affected_rows[pos])
      frozen_positions.push_back(pos);

  if (frozen_positions.size() < static_cast<size_t>(frozen_count))
  {
    for (int i = 0; i < N; ++i)
    {
      const uint16_t pos = reliab_seq[i];
      if (std::find(frozen_positions.begin(), frozen_positions.end(), pos) ==
          frozen_positions.end())
        frozen_positions.push_back(pos);

      if (frozen_positions.size() == static_cast<size_t>(frozen_count))
        break;
    }
  }

  std::sort(frozen_positions.begin(), frozen_positions.end());
  return frozen_positions;
}

PoAwN::init::iterative_shortening_code
PoAwN::init::LoadIterativeShorteningCode(
    const std::string &construction_directory, int N, int NS, int K, int GF,
    bool debug)
{
  iterative_shortening_code code;
  const fs::path directory(construction_directory);
  const fs::path summary_path = directory / "shortening_order.txt";
  std::ifstream summary(summary_path);
  if (!summary)
    throw std::runtime_error("Cannot open shortening order: " +
                             summary_path.string());

  std::vector<uint16_t> full_shortening_order;
  std::string line;
  while (std::getline(summary, line))
  {
    if (line.rfind("# N ", 0) == 0)
      code.mother_length = std::stoi(line.substr(4));
    else if (line.rfind("# GF ", 0) == 0)
      code.gf_size = std::stoi(line.substr(5));
    else if (line.rfind("# SNR ", 0) == 0)
      code.construction_snr = std::stof(line.substr(6));
    else if (line.rfind("# selection_metric ", 0) == 0)
      code.selection_metric =
          line.substr(std::string("# selection_metric ").size());
    else if (line.rfind("# shortening_order", 0) == 0)
    {
      std::istringstream values(line);
      std::string hash;
      std::string label;
      values >> hash >> label;
      int index;
      while (values >> index)
        full_shortening_order.push_back(static_cast<uint16_t>(index));
    }
  }

  if (code.mother_length != N || code.gf_size != GF)
    throw std::runtime_error(
        "Construction metadata does not match requested N/GF");

  code.shortened_length = NS;
  const int shortening_depth = N - NS;
  if (shortening_depth < 0 || K < 1 || K > NS)
    throw std::runtime_error("The code must satisfy 1 <= K <= NS <= N");
  if (full_shortening_order.size() <
      static_cast<size_t>(shortening_depth))
    throw std::runtime_error(
        "Construction does not reach the requested shortening depth");

  code.shortened_positions.assign(
      full_shortening_order.begin(),
      full_shortening_order.begin() + shortening_depth);

  std::ostringstream snapshot_name;
  snapshot_name << "reliability_S" << std::setw(4) << std::setfill('0')
                << shortening_depth << "_NS" << std::setw(4) << NS << ".txt";
  const fs::path snapshot_path = directory / snapshot_name.str();
  std::ifstream snapshot(snapshot_path);
  if (!snapshot)
    throw std::runtime_error("Cannot open reliability snapshot: " +
                             snapshot_path.string());

  std::vector<uint16_t> reliability_order;
  std::vector<bool> snapshot_active(N, false);
  std::vector<bool> snapshot_row_seen(N, false);
  bool read_reliability_order_values = false;
  while (std::getline(snapshot, line))
  {
    if (line == "# reliability_order")
    {
      read_reliability_order_values = true;
      continue;
    }
    if (read_reliability_order_values)
    {
      if (line.empty() || line[0] != '#')
        throw std::runtime_error("Missing reliability order values in " +
                                 snapshot_path.string());
      std::istringstream order_values(line.substr(1));
      int index;
      while (order_values >> index)
      {
        if (index < 0 || index >= N)
          throw std::runtime_error("Invalid reliability order index in " +
                                   snapshot_path.string());
        reliability_order.push_back(static_cast<uint16_t>(index));
      }
      read_reliability_order_values = false;
      continue;
    }
    if (line.empty() || line[0] == '#')
      continue;

    std::istringstream values(line);
    int index;
    double entropy;
    double one_error_probability;
    int is_active;
    int is_candidate;
    if (!(values >> index >> entropy >> one_error_probability >> is_active >>
          is_candidate))
      throw std::runtime_error("Invalid reliability row in " +
                               snapshot_path.string());
    if (index < 0 || index >= N || snapshot_row_seen[index] ||
        (is_active != 0 && is_active != 1) ||
        (is_candidate != 0 && is_candidate != 1))
      throw std::runtime_error("Invalid indexed reliability data in " +
                               snapshot_path.string());
    snapshot_row_seen[index] = true;
    snapshot_active[index] = is_active != 0;
  }

  if (read_reliability_order_values ||
      reliability_order.size() != static_cast<size_t>(N) ||
      std::find(snapshot_row_seen.begin(), snapshot_row_seen.end(), false) !=
          snapshot_row_seen.end())
    throw std::runtime_error(
        "Reliability snapshot does not describe exactly N input channels");

  std::vector<bool> reliability_seen(N, false);
  for (const uint16_t index : reliability_order)
  {
    if (reliability_seen[index])
      throw std::runtime_error("Duplicate reliability order index in " +
                               snapshot_path.string());
    reliability_seen[index] = true;
    if (snapshot_active[index])
      code.active_reliability_order.push_back(index);
  }

  if (code.active_reliability_order.size() != static_cast<size_t>(NS))
    throw std::runtime_error(
        "Reliability snapshot does not contain exactly NS active inputs");

  std::vector<bool> seen(N, false);
  for (const uint16_t index : code.shortened_positions)
  {
    if (index >= N || seen[index])
      throw std::runtime_error("Invalid or duplicate shortened index");
    seen[index] = true;
  }
  for (const uint16_t index : code.active_reliability_order)
  {
    if (index >= N || seen[index])
      throw std::runtime_error(
          "Shortened and active index sets are not disjoint and unique");
    seen[index] = true;
  }
  if (std::find(seen.begin(), seen.end(), false) != seen.end())
    throw std::runtime_error(
        "Shortened and active index sets do not partition the mother code");

  code.information_positions.assign(code.active_reliability_order.begin(),
                                    code.active_reliability_order.begin() + K);
  std::sort(code.information_positions.begin(),
            code.information_positions.end());

  code.frozen_symbols.assign(N, true);
  for (const uint16_t index : code.information_positions)
    code.frozen_symbols[index] = false;

  if (debug)
  {
    std::cout << "#(II) Loaded iterative shortening construction: "
              << directory << '\n'
              << "#(II) construction SNR=" << code.construction_snr
              << ", metric=" << code.selection_metric
              << ", S=" << shortening_depth << ", NS=" << NS
              << ", K=" << K << '\n';
  }

  return code;
}
