#include "dec_generator_abd.hpp"
#include <cstdlib>
#include <iostream>
#include <utilities/reliab_loader.hpp>
#include <vector>

int main(int argc, char * argv[])
{
    if (argc < 5)
    {
        std::cerr << "Usage: " << argv[0] << " <N> <K> <GF> <SNR>\n";
        std::cerr << "Example: " << argv[0] << " 64 42 64 1.0\n";
        return 1;
    }

    const int   N   = std::atoi(argv[1]);
    const int   K   = std::atoi(argv[2]);
    const int   GF  = std::atoi(argv[3]);
    const float SNR = std::atof(argv[4]);

    std::cout << "Running decoder generator with:\n"
              << "  N   = " << N << "\n"
              << "  K   = " << K << "\n"
              << "  GF  = " << GF << "\n"
              << "  SNR = " << SNR << "\n";

    std::vector<uint16_t> reliab_sequence = load_reliability_sequence(GF, N, SNR);

    std::vector<int> frozen(N, 1); // all frozen initially
    for (int i = 0; i < K && i < (int) reliab_sequence.size(); i++)
        frozen[reliab_sequence[i]] = 0;

    dec_generator gen(N, GF);
    gen.verbose = true;
    gen.analyze(frozen.data(), N);

    return 0;
}
