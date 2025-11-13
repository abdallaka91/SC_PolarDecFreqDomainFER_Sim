#include <cmath>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <vector>
#include "Decoder_functions.h"
#include "GF_tools.h"
#include "init.h"
#include "struct.h"
#include "tools.h"
#include "HelperFunc.h"
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <iomanip>
#include <algorithm>
#include "channel.h"
#include <filesystem>
#include <omp.h>
#include <atomic>
#include <random>
// #include "specialized_pruning/decoder_specialized_pruning.hpp"
// #include "naive_pruning/decoder_naive_pruning.hpp"
#include "decoders/naive/decoder_naive.hpp"
// #include "definitions/code.hpp"
// #include "specialized/decoder_specialized.hpp"
// #include "specialized_pruning/decoder_specialized_pruning.hpp"
// #include "dedicated/decoder_dedicated.hpp"

// #include <omp.h>

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

#include <fstream>
#include <string>
#include <cstdio>
#include <iostream>

namespace fs = std::filesystem;

void append_results_to_file(
    const std::string &modulation,
    int GFx,
    int Nx,
    int Kx,
    double SNR,
    unsigned long nb_err,
    unsigned long nb_gen_frame,
    float debit,
    int tSimuSec)
{
    // Directory path
    fs::path dir = "results";

    // Create directory if not exists
    std::error_code ec;
    if (!fs::exists(dir))
    {
        if (!fs::create_directories(dir, ec))
        {
            std::cerr << "Error creating directory " << dir << ": " << ec.message() << "\n";
            return;
        }
    }

    // Compose filename
    fs::path filename = dir / (modulation + "_GF" + std::to_string(GFx) +
                               "_N" + std::to_string(Nx) +
                               "_K" + std::to_string(Kx) + ".txt");

    // Open file in append mode
    FILE *fou = fopen(filename.c_str(), "a");

    if (fou == nullptr)
    {
        std::cerr << "Error opening file " << filename << " for appending.\n";
        return;
    }

    double FER_value = (nb_gen_frame == 0) ? 0.0 : static_cast<double>(nb_err) / nb_gen_frame;

    fprintf(fou, "%+6.2f ", SNR);
    fprintf(fou, "%1.16f ", FER_value);
    fprintf(fou, "%1.2e ", FER_value);
    fprintf(fou, "%5.2f ", debit);
    fprintf(fou, "%6d\n", tSimuSec);
    fclose(fou);
}

void append_results_to_file(
    const std::string &modulation,
    int GFx,
    int Nx,
    int Kx,
    double SNR,
    unsigned long nb_err,
    unsigned long nb_gen_frame)
{
    // Directory path
    fs::path dir = "results";

    // Create directory if not exists
    std::error_code ec;
    if (!fs::exists(dir))
    {
        if (!fs::create_directories(dir, ec))
        {
            std::cerr << "Error creating directory " << dir << ": " << ec.message() << "\n";
            return;
        }
    }

    // Compose filename
    fs::path filename = dir / (modulation + "_GF" + std::to_string(GFx) +
                               "_N" + std::to_string(Nx) +
                               "_K" + std::to_string(Kx) + ".txt");

    // Open file in append mode
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Error opening file " << filename << " for appending.\n";
        return;
    }

    double FER_value = (nb_gen_frame == 0) ? 0.0 : static_cast<double>(nb_err) / nb_gen_frame;

    file << "SNR=" << SNR << " db,    FER = "
         << nb_err << "/" << nb_gen_frame << " = " << FER_value << "\n";
}

int main(int argc, char *argv[])
{
#ifdef __AVX512BW__
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation program (AVX512 version)\n");
#elif __AVX2__
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation program (AVX2 version)\n");
#else
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation program (ARM NEON version)\n");
#endif

    printf("#(II) + developped by Abdallah ABDALLAH in 2025...\n");
    printf("#(II) +        and by Camille MONIERE   in 2025...\n");
    printf("#(II) +        and by Bertrand LE GAL   in 2025...\n");
    printf("#(II)\n");
    printf("#(II) Binary generated : %s - %s\n", __DATE__, __TIME__);

    if (argc != 6)
    {
        cout << "validate: NbMonteCarlo, SNR, q, N, K" << std::endl;
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
    int FER_STOP = 100000;
    // N = 1024;
    // K = 513;
    n = log2(N);

    base_code_t code_param(N, K, n, q, p, frozen_val);
    code_param.sig_mod = "CCSK_BIN";

    int gf_rand_SEED = 0;
    float nse_rand_SEED = 1.2544;
    bool repeatable_randgen = 0;

    table_GF table;

    cout << "(II) Loading code_param [START]" << endl;
    LoadCode(code_param, EbN0);
    cout << "(II) Loading code_param [END OK]" << endl;
    // void LoadTables(base_code_t & code, table_GF & table,  const uint16_t *GF_polynom_primitive)

    cout << "(II) Loading tables [START]" << endl;
    LoadTables(code_param, table, GF_polynom_primitive.data());
    cout << "(II) Loading tables [END OK]" << endl;

    cout << "Simulation starts..." << endl;

    decoder_parameters dec_param(code_param);

    // for (int i = 0; i < N; i++)
    //     dec_param.reliab_sequence[i] = N - 1 - i;
    // dec_param.reliab_sequence[N / 2] = N / 2 - 2;
    // dec_param.reliab_sequence[N / 2 + 1] = N / 2;
    // dec_param.reliab_sequence = {1023, 1022, 1021, 1020, 1019, 1018, 1017, 1016, 1015, 1014, 1013, 1012, 1011, 1010, 1009, 1008, 1007, 1006, 1005, 1004, 1003, 1002, 1001, 1000, 999, 998, 997, 996, 995, 994, 993, 992, 991, 990, 989, 988, 987, 986, 985, 984, 983, 982, 981, 980, 979, 978, 977, 976, 975, 974, 973, 972, 971, 970, 969, 968, 967, 966, 965, 964, 963, 962, 961, 959, 958, 957, 956, 955, 954, 953, 952, 951, 950, 949, 948, 947, 946, 945, 944, 943, 942, 941, 940, 939, 938, 937, 936, 935, 934, 933, 932, 931, 930, 929, 927, 926, 925, 924, 923, 922, 921, 920, 919, 918, 917, 916, 915, 914, 913, 911, 910, 909, 908, 907, 906, 905, 903, 902, 901, 899, 895, 894, 893, 892, 891, 890, 889, 888, 887, 886, 885, 884, 883, 882, 881, 880, 879, 878, 877, 876, 875, 874, 873, 872, 871, 870, 869, 868, 867, 866, 865, 863, 862, 861, 860, 859, 858, 857, 856, 855, 854, 853, 852, 851, 850, 849, 847, 846, 845, 844, 843, 842, 841, 839, 838, 837, 835, 831, 830, 829, 828, 827, 826, 825, 824, 823, 822, 821, 820, 819, 818, 817, 815, 814, 813, 812, 811, 810, 809, 807, 806, 805, 803, 799, 798, 797, 796, 795, 794, 793, 791, 790, 783, 767, 766, 765, 764, 763, 762, 761, 760, 759, 758, 757, 756, 755, 754, 753, 752, 751, 750, 749, 748, 747, 746, 745, 744, 743, 742, 741, 740, 739, 738, 737, 735, 734, 733, 732, 731, 730, 729, 728, 727, 726, 725, 724, 723, 722, 721, 719, 718, 717, 716, 715, 714, 713, 711, 710, 703, 702, 701, 700, 699, 698, 697, 696, 695, 694, 693, 692, 691, 687, 686, 685, 683, 679, 671, 670, 669, 667, 663, 655, 639, 638, 637, 636, 635, 634, 633, 631, 630, 629, 627, 623, 622, 621, 619, 615, 607, 606, 605, 603, 575, 511, 510, 509, 508, 507, 506, 505, 504, 503, 502, 501, 500, 499, 498, 497, 495, 494, 493, 492, 491, 490, 489, 487, 486, 485, 483, 479, 478, 477, 476, 475, 474, 473, 471, 470, 469, 467, 463, 462, 461, 459, 455, 447, 446, 445, 444, 443, 442, 441, 439, 438, 435, 431, 430, 415, 383, 382, 381, 379, 375, 367, 255, 690, 689, 681, 709, 429, 782, 787, 599, 677, 684, 789, 427, 682, 254, 779, 678, 707, 675, 574, 437, 666, 781, 632, 573, 665, 668, 661, 960, 591, 253, 378, 628, 496, 662, 380, 928, 423, 864, 912, 413, 571, 775, 659, 625, 904, 620, 374, 618, 898, 472, 377, 488, 626, 351, 848, 840, 897, 653, 900, 251, 414, 834, 816, 808, 567, 465, 836, 613, 466, 484, 482, 654, 458, 617, 481, 833, 804, 468, 373, 802, 614, 411, 712, 720, 457, 792, 611, 786, 460, 247, 736, 651, 801, 436, 371, 788, 604, 454, 785, 602, 440, 601, 688, 366, 780, 365, 647, 680, 597, 434, 433, 598, 708, 706, 407, 778, 676, 428, 705, 363, 777, 453, 674, 595, 664, 559, 426, 673, 451, 319, 590, 660, 425, 774, 572, 658, 570, 589, 422, 624, 773, 350, 252, 239, 376, 399, 250, 412, 616, 657, 421, 569, 359, 349, 896, 652, 480, 372, 587, 612, 249, 566, 410, 832, 650, 771, 464, 370, 610, 409, 565, 419, 347, 246, 600, 800, 649, 456, 543, 369, 364, 406, 609, 245, 596, 784, 452, 646, 223, 583, 432, 362, 563, 405, 558, 704, 318, 594, 343, 776, 645, 450, 243, 424, 238, 361, 317, 557, 672, 593, 588, 403, 398, 358, 348, 449, 237, 191, 772, 568, 420, 248, 335, 643, 315, 127, 656, 586, 555, 397, 357, 346, 235, 542, 408, 418, 564, 222, 770, 244, 311, 585, 63, 368, 231, 395, 345, 355, 551, 221, 287, 648, 303, 541, 159, 95, 417, 582, 219, 608, 111, 207, 119, 123, 342, 242, 125, 562, 175, 215, 404, 769, 190, 391, 189, 183, 187, 31, 126, 539, 316, 527, 241, 535, 341, 360, 581, 236, 556, 561, 271, 644, 143, 79, 47, 339, 402, 334, 314, 327, 579, 59, 61, 15, 333, 313, 592, 331, 295, 279, 55, 234, 62, 448, 233, 401, 199, 103, 151, 87, 167, 285, 307, 283, 396, 554, 356, 227, 310, 286, 309, 299, 229, 230, 301, 93, 157, 115, 642, 302, 155, 158, 519, 94, 91, 553, 211, 203, 107, 220, 117, 121, 179, 171, 109, 205, 217, 118, 110, 206, 263, 122, 213, 218, 71, 547, 135, 39, 23, 173, 394, 174, 214, 550, 641, 354, 185, 181, 540, 344, 124, 393, 549, 7, 186, 387, 182, 353, 188, 584, 30, 29, 531, 523, 390, 389, 526, 537, 538, 525, 416, 533, 323, 534, 27, 240, 340, 291, 275, 267, 51, 337, 99, 147, 195, 139, 768, 75, 83, 163, 270, 43, 338, 580, 142, 78, 46, 57, 515, 560, 329, 325, 269, 259, 577, 67, 35, 131, 19, 11, 326, 60, 58, 3, 332, 305, 312, 281, 330, 141, 293, 77, 578, 45, 297, 277, 14, 225, 53, 232, 113, 294, 153, 89, 278, 284, 105, 201, 197, 101, 400, 209, 54, 149, 165, 85, 306, 308, 169, 282, 177, 228, 226, 545, 298, 300, 156, 92, 198, 102, 150, 166, 385, 86, 529, 552, 114, 521, 517, 116, 154, 90, 210, 108, 202, 204, 106, 120, 216, 212, 13, 321, 172, 170, 178, 273, 289, 265, 261, 180, 25, 49, 145, 640, 73, 69, 81, 137, 97, 133, 37, 41, 161, 193, 184, 548, 21, 392, 546, 513, 352, 518, 257, 33, 65, 17, 129, 9, 5, 1, 28, 388, 386, 536, 524, 530, 532, 522, 262, 336, 70, 134, 38, 22, 56, 328, 322, 324, 576, 268, 290, 274, 280, 292, 6, 304, 276, 296, 266, 26, 140, 52, 76, 224, 50, 44, 152, 112, 146, 98, 88, 200, 104, 100, 196, 82, 138, 74, 194, 148, 162, 208, 164, 84, 42, 168, 176, 544, 528, 384, 520, 516, 514, 320, 272, 264, 288, 260, 258, 48, 24, 72, 12, 136, 144, 68, 66, 34, 80, 40, 36, 132, 96, 130, 18, 160, 20, 192, 10, 512, 256, 32, 16, 64, 8, 128, 4, 2, 0};

    // for (int i = 0; i < dec_param.reliab_sequence.size(); ++i)
    //     printf("%d ", dec_param.reliab_sequence[i]);
    printf("\n");
    CCSK_seq ccsk_seq;
    vector<vector<uint16_t>> CCSK_rotated_codes(q, vector<uint16_t>());
    if (code_param.sig_mod == "CCSK_BIN")
        create_ccsk_rotated_table(ccsk_seq.CCSK_bin_seq[code_param.p - 2], ccsk_seq.CCSK_bin_seq[code_param.p - 2].size(), CCSK_rotated_codes);
    else if (code_param.sig_mod == "CCSK_NB")
        create_ccsk_rotated_table(ccsk_seq.CCSK_GF_seq[code_param.p - 2], ccsk_seq.CCSK_GF_seq[code_param.p - 2].size(), CCSK_rotated_codes);

    vector<vector<vector<int16_t>>> hst1(n, vector<vector<int16_t>>(N, vector<int16_t>(dec_param.nm, 0)));

    q = code_param.q;
    p = code_param.p;
    vector<vector<softdata_t>> bin_mod_dict;
    if (code_param.sig_mod == "CCSK_BIN")
    {
        bin_mod_dict.resize(q, vector<softdata_t>(q, 0));

        for (int i = 0; i < q; i++)
            for (int j = 0; j < q; j++)
                bin_mod_dict[i][j] = (CCSK_rotated_codes[i][j] == 0) ? 1 : -1;
    }

    dec_param.ucap.resize(n + 1, vector<uint16_t>(N, dec_param.MxUS));
    dec_param.ucap[n].assign(N, dec_param.frozen_val);
    uint64_t FER_out = 0, gen_frames_out = 0;
    std::atomic<int> global_counter(0);
    std::atomic<int> FER(0);
    std::atomic<bool> stop(false);
    unsigned base_seed = 0; // std::chrono::system_clock::now().time_since_epoch().count();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //

    int frozen_symbols[N];
    for (int i = 0; i < N; i += 1)
        frozen_symbols[i] = true;
    for (int i = 0; i < K; i += 1)
        frozen_symbols[dec_param.reliab_sequence[i]] = false;
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    double time_base[64];
    for (int i = 0; i < 64; i += 1)
        time_base[i] = 0.0;

    const auto s_start = std::chrono::system_clock::now();

#pragma omp parallel
    {
        PoAwN::structures::decoder_parameters dec_param_local = dec_param;
        int thread_id = omp_get_thread_num();
        std::mt19937 gen(thread_id + base_seed);
        vector<vector<decoder_t>> L(n + 1, vector<decoder_t>(N));

        for (int i = 0; i <= n; i++)
            for (int j = 0; j < N; j++)
            {
                L[i][j].intrinsic_LLR.resize(q, 0);
                L[i][j].is_freq = false;
            }

        vector<uint16_t> info_sec_rec(K, dec_param_local.MxUS);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        std::vector<uint16_t> decoded_n(N);
        decoder_naive<_GF_> dec(N, frozen_symbols);
        // decoder_specialized_pruning<_GF_> dec(N, frozen_symbols);
        // decoder_naive_pruning<_GF_> dec(N, frozen_symbols);
        // decoder_specialized<_GF_> dec(N, frozen_symbols);
        // decoder_specialized_pruning<_GF_> dec(N, frozen_symbols);
        // decoder_dedicated<_GF_> dec(N, frozen_symbols);

        std::vector<symbols_t> llrs_n(N);
        //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        while (true)
        {

            bool succ_dec = true;
            vector<uint16_t> KSYMB(K);

            EncodeChanBPSK_BinCCSK(gen, dec_param_local, table, EbN0, CCSK_rotated_codes, L[0], KSYMB, bin_mod_dict);

            for (int i = 0; i < N; i++)
            {
                llrs_n[i].is_freq = false;
                for (int j = 0; j < _GF_; j++)
                    llrs_n[i].value[j] = L[0][i].intrinsic_LLR[j];
            }

            const auto m_start = std::chrono::system_clock::now();
            dec.execute(llrs_n.data(), decoded_n.data());
            const auto m_stop = std::chrono::system_clock::now();
            time_base[thread_id] += std::chrono::duration_cast<std::chrono::microseconds>(m_stop - m_start).count();

            for (int i = 0; i < K; i++)
                info_sec_rec[i] = decoded_n[dec_param.reliab_sequence[i]];

            for (uint16_t i = 0; i < dec_param_local.K; i++)
            {
                if (KSYMB[i] != info_sec_rec[i])
                {
                    succ_dec = false;
                    break;
                }
            }

            global_counter.fetch_add(1);
            int succ_now = global_counter.load() - FER.load();
            if (!succ_dec)
            {
                FER.fetch_add(1);
            }
            succ_now = global_counter.load() - FER.load();
            if ((global_counter % 1000) == 0)
            {

#pragma omp critical
                {
                    int local_success = global_counter.load() - FER.load();
                    if ((global_counter.load() >= NbMonteCarlo) || (FER.load() >= FER_STOP))
                        stop.store(true); // Set the flag
                    FER_out = FER.load();
                    gen_frames_out = global_counter.load();
                    cout << "\rSNR: " << EbN0 << " dB, FER = " << FER
                         << "/" << global_counter << " = "
                         << (float)FER_out / gen_frames_out << std::flush;
                }
            }
            if (stop.load())
                break;
        }
    }
    const auto s_stop = std::chrono::system_clock::now();
    const int tSimuSec = std::chrono::duration_cast<std::chrono::seconds>(s_stop - s_start).count();

    double total_us = 0.0;
    for (int i = 0; i < 64; i += 1)
        total_us = (total_us >= time_base[i]) ? total_us : time_base[i];
    const float time_run = (total_us / (double)gen_frames_out);
    const float debit = ((double)N * (double)_logGF_) / time_run;

    cout << "\rSNR: " << EbN0 << " dB, FER = " << FER_out << "/" << gen_frames_out
         << " = " << (float)FER_out / (float)gen_frames_out << std::flush;
    cout << " :: débit = " << debit << " Mbps";
    cout << endl;

    append_results_to_file(
        dec_param.sig_mod.c_str(),
        dec_param.q,
        dec_param.N,
        dec_param.K,
        EbN0,
        FER_out,
        gen_frames_out,
        debit,
        tSimuSec);
}