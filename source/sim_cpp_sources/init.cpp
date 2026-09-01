#include "init.h"
#include "struct.h"
#include "tools.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

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
