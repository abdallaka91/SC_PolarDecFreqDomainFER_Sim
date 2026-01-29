#pragma once
#include "Gaussian_noise_generator_fast.hpp"

namespace aff3ct_wrapper {
    template<typename R = float>
    class FastNoiseGenerator {
        aff3ct::tools::Gaussian_noise_generator_fast<R> gen;
    public:
        FastNoiseGenerator(int seed = 0) : gen(seed) {}
        
        void generate(R* noise, int length, R sigma, R mu = 0.0) {
            gen.generate(noise, (unsigned)length, sigma, mu);
        }
        
        void set_seed(int seed) {
            gen.set_seed(seed);
        }
    };
}
