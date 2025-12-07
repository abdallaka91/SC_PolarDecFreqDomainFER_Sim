#ifndef INIT_H
#define INIT_H

#include "struct.h"
#include <vector>
using std::vector;
using Mat = std::vector<std::vector<uint16_t>>;

namespace PoAwN {
namespace init {
void LoadCode(PoAwN::structures::base_code_t &code, float SNR);
void LoadTables(PoAwN::structures::base_code_t &code,
                PoAwN::structures::table_GF &table,
                const uint16_t *GF_polynom_primitive);
void LoadBubblesIndcatorlists(PoAwN::structures::decoder_parameters &dec,
                              const float SNR);

Mat kron(const Mat &A, const Mat &B);
std::vector<uint16_t>
shortened_sequence(const std::vector<uint16_t> &reliab_seq, uint16_t N);
std::vector<uint16_t> froz_from_short(int N,
                                      const std::vector<uint16_t> &reliab_seq,
                                      int Froz_cnt,
                                      std::vector<uint16_t> short_pos);
} // namespace init
} // namespace PoAwN
#endif