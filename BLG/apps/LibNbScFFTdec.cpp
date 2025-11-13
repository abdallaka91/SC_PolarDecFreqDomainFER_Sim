#include <chrono>
#include <cstdlib>
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
#include "encoder/polar_encoder.hpp"
#include "decoders/nb_sc_decoder/nb_sc_decoder.cpp"     // pas bien d'inclure le CPP !!!
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
extern "C" nb_sc_dec* allocate_dec( const int N, const int GF, const int* f_vector)
{
    nb_sc_dec* ptr = nullptr;
          if ( GF ==    8 ) { ptr =  new nb_sc_decoder<   8>(N, f_vector);
    }else if ( GF ==   16 ) { ptr =  new nb_sc_decoder<  16>(N, f_vector);
    }else if ( GF ==   32 ) { ptr =  new nb_sc_decoder<  32>(N, f_vector);
    }else if ( GF ==   64 ) { ptr =  new nb_sc_decoder<  64>(N, f_vector);
    }else if ( GF ==  128 ) { ptr =  new nb_sc_decoder< 128>(N, f_vector);
    }else if ( GF ==  256 ) { ptr =  new nb_sc_decoder< 256>(N, f_vector);
    }else if ( GF ==  512 ) { ptr =  new nb_sc_decoder< 512>(N, f_vector);
    }else if ( GF == 1024 ) { ptr =  new nb_sc_decoder<1024>(N, f_vector);
    }else if ( GF == 2048 ) { ptr =  new nb_sc_decoder<2048>(N, f_vector);
    }else if ( GF == 4096 ) {
        ptr =  new nb_sc_decoder<4096>(N, f_vector);
    }

    if ( ptr == nullptr ) {
        printf("(EE) Error we should never be there...\n");
        printf("(EE) %s %d\n", __FILE__, __LINE__);
        printf("(EE) An issue was detected with following parameters:\n");
        printf("(EE) - N  = %d\n", N);
        printf("(EE) - GF = %d\n", GF);
        exit( EXIT_FAILURE );
    }

    return ptr;
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
extern "C" polar_encoder* allocate_enc( const int N, const int K, const int GF, const int* f_vector)
{
    if ( (GF >= 16) && (GF <= 4096) ) {
        return new polar_encoder(f_vector, K, N);
    }else {
        printf("(EE) Error we should never be there...\n");
        printf("(EE) %s %d\n", __FILE__, __LINE__);
        printf("(EE) An issue was detected with following parameters:\n");
        printf("(EE) - N  = %d\n", N);
        printf("(EE) - K  = %d\n", K);
        printf("(EE) - GF = %d\n", GF);
        exit( EXIT_FAILURE );
    }
    return nullptr;
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
