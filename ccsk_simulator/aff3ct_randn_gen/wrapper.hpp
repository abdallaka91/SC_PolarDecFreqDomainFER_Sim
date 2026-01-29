#include "./MIPP/src/mipp.h"
#include <cmath>

class FastNoiseGenerator
{
public:
    void generate(float *noise, int length, float sigma)
    {
        const float twopi = 2.0f * 3.14159265358979323846f;

        // SIMD version
        const int vec_loop_size = (length / (mipp::nElReg<float>() * 2)) * mipp::nElReg<float>() * 2;
        for (int i = 0; i < vec_loop_size; i += mipp::nElReg<float>() * 2)
        {
            // Generate uniform random numbers (use your own RNG here)
            mipp::Reg<float> u1 = /* uniform random 0-1 */;
            mipp::Reg<float> u2 = /* uniform random 0-1 */;

            // Box-Muller SIMD
            mipp::Reg<float> radius = mipp::sqrt(mipp::log(u1) * -2.0f) * sigma;
            mipp::Reg<float> theta = u2 * twopi;

            mipp::Reg<float> sintheta, costheta;
            mipp::sincos(theta, sintheta, costheta);

            auto awgn1 = radius * costheta;
            auto awgn2 = radius * sintheta;

            awgn1.store(&noise[i]);
            awgn2.store(&noise[i + mipp::nElReg<float>()]);
        }

        // Sequential version for remainder
        // ...
    }
};