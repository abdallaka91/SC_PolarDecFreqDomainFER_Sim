#include "definitions/code.hpp"
#include "Hadamard.hpp"

#if (_GF_ == 8) || defined(ALL_GFs)
    #include "Hadamard_8.hpp"
#endif

#if (_GF_ == 16) || defined(ALL_GFs)
    #include "Hadamard_16.hpp"
#endif

#if (_GF_ == 32) || defined(ALL_GFs)
    #include "Hadamard_32.hpp"
#endif

#if (_GF_ == 64) || defined(ALL_GFs)
    #include "Hadamard_64.hpp"
#endif

#if (_GF_ == 128) || defined(ALL_GFs)
    #include "Hadamard_128.hpp"
#endif

#if (_GF_ == 256) || defined(ALL_GFs)
    #include "Hadamard_256.hpp"
#endif

#if (_GF_ == 512) || defined(ALL_GFs)
    #include "Hadamard_512.hpp"
#endif

#if (_GF_ == 1024) || defined(ALL_GFs)
    #include "Hadamard_1024.hpp"
#endif

#if (_GF_ == 2048) || defined(ALL_GFs)
    #include "Hadamard_2048.hpp"
#endif

#if (_GF_ == 4096) || defined(ALL_GFs)
    #include "Hadamard_4096.hpp"
#endif
