#pragma once
#include "MIPP/src/mipp.h"
#include <random>

template <typename R = float>
class FastGaussianNoise
{
    std::mt19937 rng;
    std::uniform_real_distribution<R> uniform;
    R sigma;
    R buffer[mipp::nElReg<R>()]; // Small buffer for generate_one()

public:
    FastGaussianNoise(R sigma = 1.0, int seed = 42)
        : rng(seed), uniform(0.0, 1.0), sigma(sigma) {}

    void generate(R *noise, unsigned length, R mu = 0.0)
    {
        const auto twopi = (R)(2.0 * 3.14159265358979323846);
        const int simd_size = mipp::nElReg<R>();
        const int vec_loop_size = (length / (simd_size * 2)) * simd_size * 2;

        for (int i = 0; i < vec_loop_size; i += simd_size * 2)
        {
            R u1_arr[simd_size], u2_arr[simd_size];
            for (int j = 0; j < simd_size; j++)
            {
                u1_arr[j] = uniform(rng);
                u2_arr[j] = uniform(rng);
            }

            mipp::Reg<R> u1 = u1_arr;
            mipp::Reg<R> u2 = u2_arr;

            u1 = mipp::max(u1, mipp::Reg<R>((R)1e-10));
            const auto radius = mipp::sqrt(mipp::log(u1) * (R)-2.0) * this->sigma;
            const auto theta = u2 * twopi;

            mipp::Reg<R> sintheta, costheta;
            mipp::sincos(theta, sintheta, costheta);

            auto awgn1 = radius * costheta + mu;
            auto awgn2 = radius * sintheta + mu;

            awgn1.store(&noise[i]);
            awgn2.store(&noise[i + simd_size]);
        }

        // Sequential for remainder
        for (int i = vec_loop_size; i < (int)length; i += 2)
        {
            if (i + 1 < (int)length)
            {
                R u1 = uniform(rng), u2 = uniform(rng);
                R radius = std::sqrt(std::log(u1) * (R)-2.0) * this->sigma;
                R theta = u2 * twopi;
                noise[i] = radius * std::cos(theta) + mu;
                noise[i + 1] = radius * std::sin(theta) + mu;
            }
            else
            {
                R u1 = uniform(rng), u2 = uniform(rng);
                R radius = std::sqrt(std::log(u1) * (R)-2.0) * this->sigma;
                R theta = u2 * twopi;
                noise[i] = radius * std::sin(theta) + mu;
            }
        }
    }

    // Generate single sample
    R generate_one(R mu = 0.0)
    {
        generate(buffer, 1, mu);
        return buffer[0];
    }

    void set_seed(int seed)
    {
        rng.seed(seed);
    }

    void set_sigma(R new_sigma)
    {
        sigma = new_sigma;
    }
};