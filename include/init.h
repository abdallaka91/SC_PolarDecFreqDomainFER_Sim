#ifndef INIT_H
#define INIT_H

#include "struct.h"
#include <string>
#include <vector>
namespace PoAwN
{
    namespace init
    {
        struct iterative_shortening_code
        {
            int mother_length = 0;
            int shortened_length = 0;
            int gf_size = 0;
            float construction_snr = 0.0f;
            std::string selection_metric;
            std::vector<uint16_t> shortened_positions;
            std::vector<uint16_t> active_reliability_order;
            std::vector<uint16_t> information_positions;
            std::vector<int> frozen_symbols;
        };

        void LoadCode(PoAwN::structures::base_code_t &code, float SNR,
                      const std::string &base_dir = "./BLG/matrices/", const bool debug = false);

        iterative_shortening_code LoadIterativeShorteningCode(
            const std::string &construction_directory, int N, int NS, int K,
            int GF, bool debug = false);

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
