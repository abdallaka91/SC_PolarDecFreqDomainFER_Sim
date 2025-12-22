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
#include "decoders/dedicated/decoder_dedicated.hpp"
#include "decoders/naive/decoder_naive.hpp"
#include "decoders/naive_cfloat/decoder_naive_cfloat.hpp"
#include "decoders/naive_fixed/decoder_naive_fixed.hpp"
// #include "decoders/naive_int32_t/decoder_naive_int32_t.hpp"
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

int main(int argc, char *argv[]) {

  string dec_type;

  if (argc != 7) {
    cout << "validate: NbMonteCarlo, SNR, q, N, K, dec1...dec5" << std::endl;
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
  n = log2(N);

  base_code_t code_param(N, K, n, q, p, frozen_val);

  int gf_rand_SEED = 0;

  cout << "(II) Loading code_param [START]" << endl;
  LoadCode(code_param, EbN0, "./matrices/");
  cout << "(II) Loading code_param [END OK]" << endl;

  decoder_parameters dec_param(code_param);
  q = code_param.q;
  p = code_param.p;
  int frozen_symbols[N];
  for (int i = 0; i < N; i += 1)
    frozen_symbols[i] = true;
  for (int i = 0; i < K; i += 1)
    frozen_symbols[dec_param.reliab_sequence[i]] = false;

  std::vector<uint16_t> decoded_n(N);

  decoder *dec;
  if (dec_type == "dec1") {
    dec = new decoder_naive<_GF_>(N, frozen_symbols);
    // } else if (dec_type == "dec1_int32") {
    //   dec = new decoder_naive_int32_t<_GF_>(N, frozen_symbols);
  } else if (dec_type == "dec1_fixed") {
    dec = new decoder_naive_fixed<_GF_>(N, frozen_symbols);
  } else if (dec_type == "dec1_cfloat") {
    dec = new decoder_naive_cfloat<_GF_>(N, frozen_symbols);
  } else if (dec_type == "dec3") {
    dec = new decoder_specialized<_GF_>(N, frozen_symbols);
  } else if (dec_type == "dec4") {
    dec = new decoder_specialized_pruning<_GF_>(N, frozen_symbols);
  } else if (dec_type == "dec5") {
    dec = new decoder_dedicated<_GF_>(N, frozen_symbols);
  } else if (dec_type == "dec0") {
    dec = new decoder_basic<_GF_>(N, frozen_symbols);
  } else {
    printf("#(II) Error : unknown decoder type\n");
    exit(1);
  }

  std::vector<symbols_s<_GF_>> llrs_n(N);

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < _GF_; j++)
      llrs_n[i].value[j] = 0;
  }

  const auto m_start = std::chrono::system_clock::now();
  dec->execute(llrs_n.data(), decoded_n.data());
  printf("%d\n", get_fwht_counter());
}