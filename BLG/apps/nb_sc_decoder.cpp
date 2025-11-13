#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "definitions/const_config_GF64_N64.hpp"

#include "decoders/naive/decoder_naive.hpp"
#include "decoders/naive_pruning/decoder_naive_pruning.hpp"
#include "decoders/specialized/decoder_specialized.hpp"
#include "decoders/specialized_pruning/decoder_specialized_pruning.hpp"


#include "definitions/const_config_GF64_N64.hpp"
#include "utilities/utility_functions.hpp"

// Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"


#include "../src/decoders/specialized/decoder_specialized.hpp"

#include "frozen_tree.hpp"
#include "encoder/polar_encoder.hpp"
#include "demodulator/demodulator.hpp"


//
//
//
// In frozen symbol array, the value -1 means the symbol is frozen => (symbol = 0)
//
int frozen_symbols[64];

int main(int argc, char* argv[])
{
    if ( argc != 1 ) {
        printf("(EE) One or more arugments ?!\n");
        printf("(EE) - argv[Ø] = %s\n", argv[0]);
        printf("(EE) - argv[1] = %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // initialize the frozen symbols array
    //
    uint16_t * symbols = new uint16_t[_N_];
    for (int i = 0; i < _N_; i += 1)
        frozen_symbols[i] = true;
    for (int i = 0; i <  _K_; i += 1)
        frozen_symbols[reliab_seq[i]] = false; // i c'est pour le DEBUG, on pourrait mettre 0
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //
    // load the channel symbols
    //
    symbols_t * channel = new symbols_t[_N_];
    for (int i = 0; i < _N_; i += 1) {
        for (int j = 0; j < _GF_; j += 1) {
            channel[i].value[j] = chan[_GF_ * i + j];
            channel[i].is_freq = false;
        }
        normalize<_GF_>(channel[i].value);
    }

    //
    // Clear the internal memory space
    //
    symbols_t * internal = new symbols_t[_N_];
    for (int i = 0; i < _N_; i += 1) {
        for (int j = 0; j < _GF_; j += 1) {
            internal[i].value[j] = 0.f;
        }
    }


//  frozen_tree pruned_tree(_N_);
//  pruned_tree.analyze(frozen_symbols, _N_);
//  pruned_tree.dump();


    printf("\nFrozen matrix:\n");
    for (int i = 0; i < _N_; i += 1) {
        if ((i % 8) == 0)
            printf(" | ");
        if ((i % 16) == 0)
            printf("\n | ");
        printf("%2d ", frozen_symbols[i]);
    }
    printf(" |\n");

    //
    // Clear the decoded symbols array
    //
    uint16_t * decoded = new uint16_t[_N_];
    for (int i = 0; i < _N_; i += 1)
        decoded[i] = 0;

    //
    //
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Call the top node function to decode the symbols
    //
    for (int i = 0; i < _N_; i += 1) { decoded[i] = -1; }
    decoder_naive<_GF_> dec_naive(_N_, frozen_symbols);
    dec_naive.execute(channel, decoded);
    printf("\n\nDecoded symbols (normal):\n");
    for (int i = 0; i < _N_; i += 1) {
        if ((i % 16) == 0)
            printf("\n ");
        if (decoded[i] == ref_out[i]) {
            printf("\e[1;32m%2d\e[0m ", decoded[i]);
        } else {
            printf("\e[1;31m%2d\e[0m ", decoded[i]);
        }
    }
    printf("\n");
    //
    //
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Call the top node function to decode the symbols
    //
    for (int i = 0; i < _N_; i += 1) { decoded[i] = -1; }
    decoder_naive_pruning<_GF_> dec_naive_p(_N_, frozen_symbols);
    dec_naive_p.execute(channel, decoded);
    printf("\n\nDecoded symbols (pruning):\n");
    for (int i = 0; i < _N_; i += 1) {
        if ((i % 16) == 0)
            printf("\n ");
        if (decoded[i] == ref_out[i]) {
            printf("\e[1;32m%2d\e[0m ", decoded[i]);
        } else {
            printf("\e[1;31m%2d\e[0m ", decoded[i]);
        }
    }
    printf("\n");
    //
    //
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Décodeur spécialisé mais sans pruning
    //
    for (int i = 0; i < _N_; i += 1) { decoded[i] = -1; }
    decoder_specialized<_GF_> dec(_N_, frozen_symbols);
    dec.execute(channel, decoded);
    printf("\n\nDecoded symbols (class):\n");
    for (int i = 0; i < _N_; i += 1) {
        if ((i % 16) == 0)
            printf("\n ");
        if (decoded[i] == ref_out[i]) {
            printf("\e[1;32m%2d\e[0m ", decoded[i]);
        } else {
            printf("\e[1;31m%2d\e[0m ", decoded[i]);
        }
    }
    printf("\n");
    //
    //
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Décodeur spécialisé AVEC pruning
    //
    for (int i = 0; i < _N_; i += 1) decoded[i] = -1;
    decoder_specialized_pruning<_GF_> dec_pruned(_N_, frozen_symbols);   // Ici
//  dec_pruned.f_tree = &pruned_tree;                   // Ici
    dec_pruned.execute(channel, decoded);
    printf("\n\nDecoded symbols (final):\n");
    for (int i = 0; i < _N_; i += 1)
    {
        if( (i%16) == 0 )
            printf("\n ");
        if (decoded[i] == ref_out[i]){
            printf("\e[1;32m%2d\e[0m ", decoded[i]);
        }else{
            printf("\e[1;31m%2d\e[0m ", decoded[i]);
        }
    }printf("\n");
    //
    //
    ///////////////////////////////////////////////////////////////////////////////
    //
    //
    const int32_t nTest = (256 * 1024);
    auto start_x86 = std::chrono::system_clock::now();
    for (int32_t loop = 0; loop < nTest; loop += 1) {
        dec_naive.execute(channel, decoded);
    }
    auto stop_x86 = std::chrono::system_clock::now();

    float time_ns   = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_x86 - start_x86).count();
    float time_sec  = time_ns / 1000.f / 1000.f / 1000.f; // in seconds
    float time_msec = time_ns / 1000.f / 1000.f;          // in seconds
    float time_usec = time_ns / 1000.f;                   // in seconds
    float time_run  = (time_usec / (float) nTest);

    float debit = ((float) _N_ * (float) _logGF_) / time_run; // in Ksymbols/s
    printf("[normal]  experiments  : %1.3f sec\n", time_sec);
    printf("[normal]  experiments  : %1.2f ms\n", time_msec);
    printf("[normal]  one decoding : %1.2f us\n", time_run);
    printf("[normal]  debit coded  : %1.2f Mbps\n", debit);
    //
    //
    /////////////////////////////////////////////////////////////////////////////////
    //
    //
    start_x86 = std::chrono::system_clock::now();
    for (int32_t loop = 0; loop < nTest; loop += 1) {
        dec_naive_p.execute(channel, decoded);
    }
    stop_x86 = std::chrono::system_clock::now();

    time_ns   = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_x86 - start_x86).count();
    time_sec  = time_ns / 1000.f / 1000.f / 1000.f; // in seconds
    time_msec = time_ns / 1000.f / 1000.f;          // in seconds
    time_usec = time_ns / 1000.f;                   // in seconds
    time_run  = (time_usec / (float) nTest);

    debit = ((float) _N_ * (float) _logGF_) / time_run; // in Ksymbols/s
    printf("[pruning] experiments  : %1.3f sec\n", time_sec);
    printf("[pruning] experiments  : %1.2f ms\n", time_msec);
    printf("[pruning] one decoding : %1.2f us\n", time_run);
    printf("[pruning] debit coded  : %1.2f Mbps\n", debit);
    //
    //
    /////////////////////////////////////////////////////////////////////////////////
    //
    //
    start_x86 = std::chrono::system_clock::now();
    for (int loop = 0; loop < nTest; loop += 1) {
        dec.execute(channel, decoded);
    }
    stop_x86 = std::chrono::system_clock::now();

    time_ns   = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_x86 - start_x86).count();
    time_sec  = time_ns / 1000.f / 1000.f / 1000.f; // in seconds
    time_msec = time_ns / 1000.f / 1000.f;          // in seconds
    time_usec = time_ns / 1000.f;                   // in seconds
    time_run  = (time_usec / (float) nTest);

    debit = ((float) _N_ * (float) _logGF_) / time_run; // in Ksymbols/s
    printf("[special] experiments  : %1.3f sec\n", time_sec);
    printf("[special] experiments  : %1.2f ms\n", time_msec);
    printf("[special] one decoding : %1.2f us\n", time_run);
    printf("[special] debit coded  : %1.2f Mbps\n", debit);
    //
    //
    /////////////////////////////////////////////////////////////////////////////////
    //
    //
    for (int x = 0; x < 60; x += 1) {
        start_x86 = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
        {
            dec_pruned.execute(channel, decoded);
        }
        stop_x86 = std::chrono::system_clock::now();

        time_ns   = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_x86 - start_x86).count();
        time_sec  = time_ns / 1000.f / 1000.f / 1000.f; // in seconds
        time_msec = time_ns / 1000.f / 1000.f; // in seconds
        time_usec = time_ns / 1000.f; // in seconds
        time_run  = (time_usec / (float)nTest);

        debit = ((float)_N_ * (float)_logGF_) / time_run; // in Ksymbols/s
        if ( x == 0 ) {
            printf("[final  ] experiments  : %1.3f sec\n",  time_sec);
            printf("[final  ] experiments  : %1.2f ms\n",   time_msec);
            printf("[final  ] one decoding : %1.2f us\n",   time_run);
        }
        printf("[final  ] debit coded  : %1.2f Mbps\n", debit);
    }
    //
    //
    /////////////////////////////////////////////////////////////////////////////////
    //
    //

    delete[] channel;
    delete[] internal;
    delete[] symbols;
    delete[] decoded;

    return EXIT_SUCCESS;
}
