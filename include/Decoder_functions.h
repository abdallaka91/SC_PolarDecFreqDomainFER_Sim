#ifndef DECODER_FUNCTIONS
#define DECODER_FUNCTIONS
#include "struct.h"

#include <cstdint>
#include <vector>

namespace PoAwN
{

    namespace decoding
    {

        using structures::decoder_parameters;
        using structures::decoder_t;
        using structures::softdata_t;
        using structures::vector;

        void Channel_LLR(const vector<vector<softdata_t>> &chan_observ,
                         const vector<vector<uint16_t>> &bin_symb_seq,
                         uint16_t q,
                         softdata_t sigma,
                         vector<vector<softdata_t>> &chan_LLR);

        void ECN_FFT(const decoder_t &theta_1,
                     const decoder_t &phi_1,
                     const vector<vector<uint16_t>> &ADDGF,
                     const vector<vector<uint16_t>> &DIVGF,
                     const decoder_parameters &dec_param,
                     const uint16_t coef,
                     decoder_t &theta);

        void VN_update_FFT(const decoder_t &theta_1,
                       const decoder_t &phi_1,
                       const vector<vector<uint16_t>> &ADDGF,
                       const vector<vector<uint16_t>> &DIVGF,
                       const decoder_parameters &dec_param,
                       uint16_t coef,
                       uint16_t hard_decision,
                       decoder_t &phi);

        void decode_SC_FFT(const decoder_parameters &dec_param,
                                  const vector<vector<uint16_t>> &ADDGF,
                                  const vector<vector<uint16_t>> &MULGF,
                                  const vector<vector<uint16_t>> &DIVGF,
                                  vector<vector<decoder_t>> &L,
                                  vector<uint16_t> &info_sec_rec);

        void frozen_lay_pos(const decoder_parameters &dec_param,
                            vector<vector<uint16_t>> &ufrozen,
                            vector<vector<bool>> &clst_frozen);

    } 

}

#endif