#ifndef INIT_H
#define INIT_H

#include "struct.h"
#include <vector>
namespace PoAwN
{
    namespace init
    {
        void LoadCode(PoAwN::structures::base_code_t &code, float SNR,
                      const std::string &base_dir = "./BLG/matrices/", const bool debug = false);
    }
}
#endif
