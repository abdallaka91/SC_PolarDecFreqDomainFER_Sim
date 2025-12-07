#include "init.h"
#include "GF_tools.h"
#include "struct.h"
#include "tools.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using Mat = std::vector<std::vector<uint16_t>>;

void PoAwN::init::LoadCode(PoAwN::structures::base_code_t &code, float SNR) {
  code.Rate = (float)code.K / (float)code.N;

  std::string base_dir;

  if (code.sig_mod == "CCSK_BIN")
    base_dir = "./matrices/ccsk_bin/";
  else
    base_dir = "./matrices/ccsk_nb/";

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

void PoAwN::init::LoadBubblesIndcatorlists(
    PoAwN::structures::decoder_parameters &dec, const float SNR) {
  uint16_t n = dec.n, nH = dec.nH, nL = dec.nL;
  std::ostringstream fname;
  std::string mat_direct;
  if (dec.sig_mod == "BPSK")
    mat_direct = "./BubblesPattern/bpsk/N";
  else if (dec.sig_mod == "CCSK_BIN")
    mat_direct = "./BubblesPattern/ccsk_bin/N";
  else
    mat_direct = "./BubblesPattern/ccsk_nb/N";

  fname << mat_direct << dec.N << "/BubblesIndicatorsLists"
        << "/bubbles_N" << dec.N << "_K" << dec.K << "_GF" << dec.q << "_SNR"
        << std::fixed << std::setprecision(3) << SNR << "_" << dec.nH << "x"
        << dec.nL << "_Bt_lsts.txt";
  std::string filename = fname.str();

  std::ifstream file(filename);
  std::vector<std::string> lines;
  std::string line;
  int linecount = 0;
  while (std::getline(file, line)) {
    if (!line.empty()) {
      lines.push_back(line);
      linecount++;
    }
  }
  if (linecount < (1u << n) - 1) {
    std::cerr
        << "File data is not enough, it should  contain " << (1u << n) - 1
        << " lines, each contains the list of (i,j) coordinates of cluster s "
           "at layer l, the lines format should be: l s, i0 j0 i1 j1..."
        << std::endl;
    std::exit(EXIT_FAILURE);
  }

  int line_cnt0, line_cnt = 0;
  dec.Bubb_Indicator.resize(n);
  for (int l = 0; l < n; l++) {
    dec.Bubb_Indicator[l].resize(1 << l);
    line_cnt0 = (1u << l) - 1;
    for (int s = 0; s < (1u << l); s++) {
      line_cnt = line_cnt0 + s;
      dec.Bubb_Indicator[l][s].resize(2);
      {
        std::istringstream iss(lines[line_cnt]);
        std::vector<int> numbers;
        std::string token;
        int skipped = 0;

        while (iss >> token) {

          if (skipped < 2) {
            skipped++;
            continue;
          }

          std::string clean;
          for (char c : token)
            if (std::isdigit(c))
              clean += c;
            else if (!clean.empty()) {
              numbers.push_back(std::stoi(clean));
              clean.clear();
            }
          if (!clean.empty())
            numbers.push_back(std::stoi(clean));
        }

        for (size_t i = 0; i < numbers.size(); i += 2) {
          dec.Bubb_Indicator[l][s][0].push_back(numbers[i]);
          dec.Bubb_Indicator[l][s][1].push_back(numbers[i + 1]);
        }
      }
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

Mat PoAwN::init::kron(const Mat &A, const Mat &B) {
  int aR = A.size(), aC = A[0].size();
  int bR = B.size(), bC = B[0].size();

  Mat R(aR * bR, std::vector<uint16_t>(aC * bC));

  for (int i = 0; i < aR; i++)
    for (int j = 0; j < aC; j++)
      for (int p = 0; p < bR; p++)
        for (int q = 0; q < bC; q++)
          R[i * bR + p][j * bC + q] = A[i][j] * B[p][q];

  return R;
}

std::vector<uint16_t>
PoAwN::init::shortened_sequence(const std::vector<uint16_t> &reliab_seq,
                                uint16_t N) {
  Mat H0 = {{1, 0}, {1, 1}};
  Mat H = {{1}}; // MATLAB H = [1]

  // H = kron(H, H0) applied log2(N) times
  for (int k = 0; k < std::log2(N); k++)
    H = kron(H, H0);

  std::vector<uint16_t> s(N);

  for (int iter = 0; iter < N; iter++) {

    // I = find(sum(H) == 1)
    std::vector<int> I;
    I.reserve(N);

    for (int col = 0; col < N; col++) {
      int colSum = 0;
      for (int row = 0; row < N; row++)
        colSum += H[row][col];

      if (colSum == 1)
        I.push_back(col);
    }

    // [a b] = min(reliab_seq(I))
    int best_idx = I[0];
    uint16_t best_val = reliab_seq[best_idx];

    for (int k = 1; k < I.size(); k++) {
      int idx = I[k];
      if (reliab_seq[idx] < best_val) {
        best_val = reliab_seq[idx];
        best_idx = idx;
      }
    }

    s[iter] = best_idx; // s(i) = I(b)

    // H(s(i),:) = 0
    for (int j = 0; j < N; j++)
      H[best_idx][j] = 0;

    // H(:, s(i)) = 0
    for (int i = 0; i < N; i++)
      H[i][best_idx] = 0;
  }

  return s;
}

std::vector<uint16_t>
PoAwN::init::froz_from_short(int N, const std::vector<uint16_t> &reliab_seq,
                             int Froz_cnt, std::vector<uint16_t> short_pos) {
  int n_short = short_pos.size();
  int n = std::log2(N);

  Mat G0 = {{1, 0}, {1, 1}};
  Mat G = G0;

  for (int k = 1; k < n; k++)
    G = kron(G, G0);

  std::sort(short_pos.begin(), short_pos.end());

  std::vector<uint16_t> OR1(N, 0);

  for (int idx = 0; idx < n_short; idx++) {
    int c = short_pos[idx];
    for (int r = 0; r < N; r++)
      OR1[r] |= G[r][c]; // OR over column
  }

  std::vector<uint16_t> FS_p;
  for (int i = 0; i < N; i++)
    if (OR1[i])
      FS_p.push_back(i);

  std::vector<uint16_t> FS_p_concat;

  if (FS_p.size() < (size_t)Froz_cnt) {
    FS_p_concat.reserve(Froz_cnt - FS_p.size());

    for (int i = 0; i < N; i++) {
      uint16_t pos = reliab_seq[i];

      // mimic MATLAB: if not in FS_p and not in FS_p_concat
      if (std::find(FS_p.begin(), FS_p.end(), pos) == FS_p.end() &&
          std::find(FS_p_concat.begin(), FS_p_concat.end(), pos) ==
              FS_p_concat.end()) {
        FS_p_concat.push_back(pos);

        if (FS_p_concat.size() + FS_p.size() == (size_t)Froz_cnt)
          break;
      }
    }
  }

  FS_p.insert(FS_p.end(), FS_p_concat.begin(), FS_p_concat.end());
  std::sort(FS_p.begin(), FS_p.end());

  return FS_p;
}
