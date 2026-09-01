#ifndef INIT_H
#define INIT_H

#include "struct.h"
#include <vector>
namespace PoAwN
{
    namespace init
    {
        void LoadCode(PoAwN::structures::base_code_t &code, float SNR,
                      const std::string &base_dir = "./BLG/matrices/", const bool debug = false);

        using Mat = std::vector<std::vector<uint16_t>>;

        Mat kron(const Mat &A, const Mat &B);
        std::vector<uint16_t>
        shortened_sequence(const std::vector<uint16_t> &reliab_seq, uint16_t N);
        std::vector<uint16_t>
        froz_from_short(int N, const std::vector<uint16_t> &reliab_seq,
                        int frozen_count, std::vector<uint16_t> short_pos);
    }
}
#endif
