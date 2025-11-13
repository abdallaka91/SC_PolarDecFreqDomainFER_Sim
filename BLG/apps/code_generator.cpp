#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "../src/definitions/code.hpp"

// Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

#include "../src/generator/dec_generator.hpp"

//
//
//
// In frozen symbol array, the value -1 means the symbol is frozen => (symbol = 0)
//
int main(int argc, char * argv[])
{
    float code_rate = 0.75f;
    int   N         = _N_;
    int   K         = (int) (((float) N) * code_rate);
    bool  verbose   = true;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    printf("# Run command:\n# ");
    for (int i = 0; i < argc; i += 1)
    {
        printf("%s ", argv[i]);
    }
    printf("\n");
    //
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    for (int i = 1; i < argc; i++)
    {
        //      printf("(II) option : [%s]\n", argv[i]);
        if (std::string(argv[i]) == "--rate")
        {
            code_rate = std::atof(argv[i + 1]);
            K         = round(code_rate * N);
            i += 1;
        }
        else if (std::string(argv[i]) == "--code-rate")
        {
            code_rate = std::atof(argv[i + 1]);
            K         = round(code_rate * N);
            i += 1;
        }
        else if (std::string(argv[i]) == "--no-verbose")
        {
            verbose = false;
        }
        else if (std::string(argv[i]) == "-verbose")
        {
            verbose = true;
        }
        else
        {
            printf("(EE) Unknown argument: %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    std::cout << "#(II)" << std::endl;
    std::cout << "#(II) Polar code parameters" << std::endl;
    std::cout << "#(II) ---------------------" << std::endl;
    std::cout << "#(II)" << std::endl;
    std::cout << "#(II) + GF equals : " << _GF_ << std::endl;
    std::cout << "#(II) +  N equals : " << N << std::endl;
    std::cout << "#(II) +  K equals : " << K << std::endl;
    std::cout << "#(II) +  R equals : " << (int) (100.f * code_rate) << "\%" << std::endl;
    std::cout << "#(II)" << std::endl;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // initialize the frozen symbols array
    //
    int * frozen_symbols = new int[_N_];
    for (int i = 0; i < N; i += 1)
        frozen_symbols[i] = true;

    for (int i = 0; i < K; i += 1)
        frozen_symbols[reliab_seq[i]] = false; // i c'est pour le DEBUG, on pourrait mettre 0
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    if (verbose == true)
    {
        printf("\nFrozen matrix:\n");
        for (int i = 0; i < N; i += 1)
        {
            if ((i % 8) == 0)
                printf(" | ");
            if ((i % 16) == 0)
                printf("\n | ");
            printf("%2d ", frozen_symbols[i]);
        }
        printf(" |\n");
    }
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    dec_generator pruned_tree(N, _GF_);
    pruned_tree.verbose = verbose;
    pruned_tree.analyze(frozen_symbols, N);

    delete[] frozen_symbols;

    return EXIT_SUCCESS;
}
