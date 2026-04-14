#include "init.h"
#include "struct.h"
#include "tools.h"
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
