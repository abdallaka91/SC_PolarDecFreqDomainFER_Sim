#ifndef TOOLS
#define TOOLS

#include "struct.h"
#include <cstdint>
#include <vector>

namespace PoAwN
{
    namespace tools
    {

        using std::vector;
        using namespace PoAwN::structures;

        void Encoder(vector<vector<uint16_t>> &ucap, vector<uint16_t> &NSYMB);

    } // namespace tools
} // namespace PoAwN
#endif
