
#include "Decoder_functions.h"
#include "struct.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <vector>
#include <iostream>
#include <queue>
#include <limits>
#include <cmath>
#include <fwht.hpp>

constexpr uint16_t q_fixed = 64;
void PoAwN::decoding::Channel_LLR(const vector<vector<softdata_t>> &chan_observ,
                                  const vector<vector<uint16_t>> &bin_symb_seq,
                                  uint16_t q,
                                  softdata_t sigma,
                                  vector<vector<softdata_t>> &chan_LLR)
{
    const int N = chan_observ.size();
    const int q1 = chan_observ[0].size();

    constexpr const softdata_t two_pow16 = softdata_t(1 << 16);

    vector<softdata_t> hard_decison(q1, two_pow16 - 1);

    const softdata_t fct = softdata_t(2) / (sigma * sigma);
    for (int i = 0; i < N; i++)
    {
        softdata_t mn_llr = std::numeric_limits<softdata_t>::max() / 2;
        for (int j = 0; j < q; j++)
        {
            softdata_t temp = 0;

            for (int k = 0; k < q1; k++)
            {
                // true == 1 and false == 0
                hard_decison[k] = softdata_t(uint8_t(chan_observ[i][k] <= 0));
                temp += chan_observ[i][k] * ((softdata_t)bin_symb_seq[j][k] - hard_decison[k]);
            }
            temp *= fct;
            chan_LLR[i][j] = temp;
            if (temp < mn_llr)
                mn_llr = temp;
        }
        if (mn_llr > std::numeric_limits<softdata_t>::min())
            for (int j = 0; j < q; j++)
            {
                chan_LLR[i][j] -= mn_llr;
            }
        softdata_t s1 = 0;
        for (int j = 0; j < q; j++)
        {
            chan_LLR[i][j] = exp(-chan_LLR[i][j]);
            s1 += chan_LLR[i][j];
        }
        for (int j = 0; j < q; j++)
        {
            chan_LLR[i][j] /= s1;
        }
    }
}

void PoAwN::decoding::VN_update_FFT(const decoder_t &theta_1,
                                    const decoder_t &phi_1,
                                    const decoder_parameters &dec_param,
                                    uint16_t hard_decision,
                                    decoder_t &phi)
{
    uint16_t q = dec_param.q;
    vector<softdata_t> theta1_llr(q);
    vector<softdata_t> phi1_llr(q);
    vector<softdata_t> temp_llr(q);
    for (uint16_t i = 0; i < q; i++)
    {
        phi1_llr[i] = phi_1.intrinsic_LLR[i];
        theta1_llr[hard_decision ^ i] = theta_1.intrinsic_LLR[i];
    }

    softdata_t s1 = 0;

    for (int i = 0; i < q; i++)
    {
        temp_llr[i] = theta1_llr[i] * phi1_llr[i];
        s1 += temp_llr[i];
    }

    for (int i = 0; i < q; i++)
    {
        temp_llr[i] /= s1;
    }

    phi.intrinsic_LLR = temp_llr;
    bool PAUSE = false;
}

void PoAwN::decoding::decode_SC_FFT(const decoder_parameters &dec_param,
                                    vector<vector<decoder_t>> &L,
                                    vector<uint16_t> &info_sec_rec)
{
    uint16_t MxUS = dec_param.MxUS, n = dec_param.n, N = dec_param.N;
    softdata_t soft_q = softdata_t(q_fixed);
    vector<vector<bool>> Roots(n + 1);
    for (int i = 0; i < n; i++)
        Roots[i] = dec_param.clst_frozen[i];

    Roots[n].assign(N, false);
    for (int i = dec_param.K; i < N; i++)
    {
        Roots[n][dec_param.reliab_sequence[i]] = true;
    }

    int l = 0, s = 0;
    uint16_t hard_decsion, i1, i2, i3, SZc, SZc1, l1;
    vector<uint16_t> Root;

    vector<vector<uint16_t>> V = dec_param.ufrozen;
    softdata_t sqrt_q = sqrt(softdata_t(q_fixed));
    bool c1;
    while (l > -1)
    {
        if (Roots[l][s])
        {
            if (s % 2 == 1)
            {
                l -= 1;
                s = (s - 1) / 2;
                Root = dec_param.Roots_indices[l][s];
                SZc = Root.size();
                SZc1 = SZc >> 1;

                for (uint16_t t = 0; t < SZc1; t++)
                {
                    l1 = l + 1;
                    i1 = Root[t], i2 = Root[t + SZc1];
                    V[l][i1] = V[l1][i1] ^ V[l1][i2];
                    V[l][i2] = V[l1][i2];
                }
                Roots[l][s] = true;
            }
            else
            {
                l -= 1;
                s = (s >> 1u);
            }
        }
        else if (Roots[l + 1][2 * s])
        {
            Root = dec_param.Roots_indices[l][s];
            SZc = Root.size();
            SZc1 = SZc >> 1;
            for (uint16_t t = 0; t < SZc1; t++)
            {
                i3 = dec_param.coefs_id[l][s][t];
                hard_decsion = V[l + 1][Root[t]];
                bool cnd1 = hard_decsion != dec_param.ucap[l + 1][Root[t]];
                softdata_t f1 = 1;// /*L[l][Root[t]].intrinsic_LLR[0] * */soft_q;
                softdata_t f2 = 1;// /*L[l][Root[t + SZc1]].intrinsic_LLR[0] * */soft_q;
                if (L[l][Root[t]].is_freq)
                {
                    PoAwN::fwht<q_fixed>(L[l][Root[t]].intrinsic_LLR.data(),
                                         L[l][Root[t]].intrinsic_LLR.data());
                    L[l][Root[t]].is_freq = false;
                    for (int yy = 0; yy < q_fixed; yy++)
                        L[l][Root[t]].intrinsic_LLR[yy] /= f1;
                }
                if (L[l][Root[t + SZc1]].is_freq)
                {

                    PoAwN::fwht<q_fixed>(L[l][Root[t + SZc1]].intrinsic_LLR.data(),
                                         L[l][Root[t + SZc1]].intrinsic_LLR.data());
                    L[l][Root[t + SZc1]].is_freq = false;
                    for (int yy = 0; yy < q_fixed; yy++)
                        L[l][Root[t + SZc1]].intrinsic_LLR[yy] /= f2;
                }

                VN_update_FFT(L[l][Root[t]], L[l][Root[t + SZc1]], dec_param, hard_decsion, L[l + 1][Root[t + SZc1]]);
                L[l + 1][Root[t + SZc1]].is_freq = false;
                bool PAUSE = false;
            }
            l += 1;
            s = (s << 1u) + 1;
            if (l == n)
            {
                Roots[n][s] = true;
                if (V[n][s] == MxUS)
                {
                    auto max_ptr = std::max_element(L[n][s].intrinsic_LLR.begin(), L[n][s].intrinsic_LLR.end());
                    uint16_t aa = std::distance(L[n][s].intrinsic_LLR.begin(), max_ptr);
                    V[n][s] = aa;
                    bool PAUSE = false;
                }
                if (s == N - 1)
                    break;
            }
        }
        else
        {
            Root = dec_param.Roots_indices[l][s];
            SZc = Root.size();
            SZc1 = SZc >> 1;
            uint16_t q = dec_param.q;

            for (uint16_t t = 0; t < SZc1; t++)
            {

                if (!L[l][Root[t]].is_freq)
                {
                    PoAwN::fwht<q_fixed>(L[l][Root[t]].intrinsic_LLR.data(),
                                         L[l][Root[t]].intrinsic_LLR.data());
                    L[l][Root[t]].is_freq = true;
                }
                if (!L[l][Root[t + SZc1]].is_freq)
                {

                    PoAwN::fwht<q_fixed>(L[l][Root[t + SZc1]].intrinsic_LLR.data(),
                                         L[l][Root[t + SZc1]].intrinsic_LLR.data());
                    L[l][Root[t + SZc1]].is_freq = true;
                }

                std::array<softdata_t, q_fixed> C1 =
                    elementwise_mul<softdata_t, q_fixed>(L[l][Root[t]].intrinsic_LLR.data(),
                                                         L[l][Root[t + SZc1]].intrinsic_LLR.data());
                L[l + 1][Root[t]].intrinsic_LLR = std::vector<softdata_t>(C1.begin(), C1.end());
                L[l + 1][Root[t]].is_freq = true;

                bool PAUSE = 0;
            }
            l = l + 1;
            s = (s << 1u);
            if (l == n)
            {
                Roots[n][s] = true;
                if (V[n][s] == MxUS)
                {
                    PoAwN::fwht<q_fixed>(L[n][Root[s]].intrinsic_LLR.data(),
                                         L[n][Root[s]].intrinsic_LLR.data());
                    L[n][Root[s]].is_freq=false;
                                         auto max_ptr = std::max_element(L[n][s].intrinsic_LLR.begin(), L[n][s].intrinsic_LLR.end());
                    uint16_t aa = std::distance(L[n][s].intrinsic_LLR.begin(), max_ptr);
                    V[n][s] = aa;
                    bool PAUSE = false;
                }
            }
        }
    }
    info_sec_rec.resize(dec_param.K, dec_param.MxUS);
    for (uint16_t i = 0; i < dec_param.K; i++)
    {
        info_sec_rec[i] = V[n][dec_param.reliab_sequence[i]];
    }
}
void PoAwN::decoding::frozen_lay_pos(const decoder_parameters &dec_param,
                                     vector<vector<uint16_t>> &ufrozen,
                                     vector<vector<bool>> &clst_frozen)
{
    clst_frozen.resize(dec_param.n);
    ufrozen.resize(dec_param.n + 1, vector<uint16_t>(dec_param.N, dec_param.MxUS));
    for (int i = dec_param.N - 1; i >= dec_param.K; i--)
        ufrozen[dec_param.n][dec_param.reliab_sequence[i]] = dec_param.frozen_val;
    uint16_t i1, i2;
    for (int l = dec_param.n; l > 0; l--)
    {
        clst_frozen[l - 1].resize(1 << (l - 1), false);
        int sz1 = dec_param.clusts_CNs[l - 1].size();
        for (int s = 0; s < sz1; s++)
        {
            int sz2 = dec_param.clusts_CNs[l - 1][s].size();
            int cnt2 = 0;
            for (int k = 0; k < sz2; k++)
            {
                i1 = dec_param.clusts_CNs[l - 1][s][k];
                i2 = dec_param.clusts_VNs[l - 1][s][k];
                if (ufrozen[l][i1] == dec_param.frozen_val && ufrozen[l][i2] == dec_param.frozen_val)
                {
                    ufrozen[l - 1][i1] = dec_param.frozen_val;
                    ufrozen[l - 1][i2] = dec_param.frozen_val;
                    cnt2++;
                }
            }
            if (cnt2 == sz2)
                clst_frozen[l - 1][s] = true;
        }
    }
}
