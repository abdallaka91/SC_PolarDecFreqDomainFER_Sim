#include "init.h"
#include "GF_tools.h"
#include "struct.h"
#include "tools.h"
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

void PoAwN::init::LoadCode(PoAwN::structures::base_code_t &code, float SNR) {
  code.Rate = (float)code.K / (float)code.N;

  // ------------------------------------------------------
  // Select directory depending on modulation
  // ------------------------------------------------------
  std::string base_dir;

  base_dir = "./BLG/matrices/";

  // ------------------------------------------------------
  // Build folder + file name
  // ------------------------------------------------------
  std::ostringstream folder;
  folder << base_dir << "GF" << code.q << "/";

  std::ostringstream fname;
  fname << folder.str() << "GF" << code.q << "N" << code.N << ".txt";

  std::cout << "Reading reliability file: " << fname.str() << std::endl;

  std::ifstream opfile(fname.str());
  if (!opfile) {
    std::cerr << "Cannot open reliability file!" << std::endl;
    exit(-1010);
  }

  // ------------------------------------------------------
  // Read SNR + sequence blocks
  // ------------------------------------------------------
  struct entry_t {
    float snr;
    std::vector<uint16_t> seq;
  };

  std::vector<entry_t> entries;

  while (true) {
    std::string tag;
    float snr_val;

    if (!(opfile >> tag >> snr_val))
      break; // End of file

    if (tag != "SNR") {
      std::cerr << "Invalid format: expected 'SNR'" << std::endl;
      exit(EXIT_FAILURE);
    }

    // Read reliability sequence (1 line containing N ints)
    std::vector<uint16_t> seq(code.N);
    for (int i = 0; i < code.N; i++) {
      int t;
      if (!(opfile >> t)) {
        std::cerr << "Missing sequence entry at index " << i << std::endl;
        exit(EXIT_FAILURE);
      }
      seq[i] = (uint16_t)t;
    }

    entries.push_back({snr_val, seq});
  }

  if (entries.empty()) {
    std::cerr << "No SNR entries found in file!" << std::endl;
    exit(EXIT_FAILURE);
  }

  // ------------------------------------------------------
  // Find nearest SNR to the requested SNR
  // ------------------------------------------------------
  float best_dist = 2;
  int best_idx = -1;

  for (int i = 0; i < (int)entries.size(); i++) {
    float d = std::abs(entries[i].snr - SNR);
    if (d < best_dist) {
      best_dist = d;
      best_idx = i;
    }
  }

  if (best_idx < 0) {
    std::cerr << "Nearest SNR not found!" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::cout << "Requested SNR: " << SNR
            << "  --> Using nearest SNR: " << entries[best_idx].snr
            << std::endl;

  code.reliab_sequence = entries[best_idx].seq;

  code.polar_coeff.resize(code.n, std::vector<uint16_t>(code.N / 2));
  for (int i = 0; i < code.n; i++) {
    for (int j = 0; j < code.N / 2; j++) {
      code.polar_coeff[i][j] = 1;
    }
  }
}

void PoAwN::init::LoadTables(PoAwN::structures::base_code_t &code,
                             PoAwN::structures::table_GF &table,
                             const uint16_t *GF_polynom_primitive) {
  uint16_t prim_pol = GF_polynom_primitive[code.p - 2];

  std::cout << "(II) - PoAwN::GFtools::GF_bin_seq_gen" << std::endl;
  PoAwN::GFtools::GF_bin_seq_gen(code.q, prim_pol, table.BINGF, table.BINDEC);

  std::cout << "(II) - PoAwN::GFtools::GF_bin2GF" << std::endl;
  PoAwN::GFtools::GF_bin2GF(
      table.BINGF, table.DECGF,
      table.GFDEC); // x^-inf=GFDEC[0]=0, x^0=GFDEC[1]=1, , x^1=GFDEC[2]=2,.., ,
                    // x^(q-2)=GFDEC[q-1] and GFDEC[DECGF[i]]=i

  std::cout << "(II) - PoAwN::GFtools::GF_add_mat_gen" << std::endl;
  PoAwN::GFtools::GF_add_mat_gen(table.BINGF, table.DECGF, table.GFDEC,
                                 table.ADDGF, table.ADDDEC);

  std::cout << "(II) - PoAwN::GFtools::GF_mul_mat_gen" << std::endl;
  PoAwN::GFtools::GF_mul_mat_gen(table.DECGF, table.MULGF, table.MULDEC);

  std::cout << "(II) - PoAwN::GFtools::GF_div_mat_gen" << std::endl;
  //  PoAwN::GFtools::GF_div_mat_gen(table.DECGF, table.DIVGF, table.DIVDEC);
  std::cout << "(II) - End of PoAwN::GFtools section" << std::endl;
}