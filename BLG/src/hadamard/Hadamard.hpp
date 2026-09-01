extern const float Hadamard_8x8[8][8];
extern const float Hadamard_16x16[16][16];
extern const float Hadamard_32x32[32][32];
extern const float Hadamard_64x64[64][64];
extern const float Hadamard_128x128[128][128];
extern const float Hadamard_256x256[256][256];
extern const float Hadamard_512x512[512][512];
extern const float Hadamard_1024x1024[1024][1024];
extern const float Hadamard_2048x2048[2048][2048];
extern const float Hadamard_4096x4096[4096][4096];

template<int gf_size>
inline constexpr auto get_Hadamard_matrix() {
         if constexpr( gf_size ==    8 ) { return Hadamard_8x8;   }
    else if constexpr( gf_size ==   16 ) { return Hadamard_16x16; }
    else if constexpr( gf_size ==   32 ) { return Hadamard_32x32; }
    else if constexpr( gf_size ==   64 ) { return Hadamard_64x64; }
    else if constexpr( gf_size ==  128 ) { return Hadamard_128x128; }
    else if constexpr( gf_size ==  256 ) { return Hadamard_256x256; }
    else if constexpr( gf_size ==  512 ) { return Hadamard_512x512; }
    else if constexpr( gf_size == 1024 ) { return Hadamard_1024x1024; }
    else if constexpr( gf_size == 2048 ) { return Hadamard_2048x2048; }
    else if constexpr( gf_size == 4096 ) { return Hadamard_4096x4096; }
    else {
        static_assert(false, "We never should be here !");
    }
}

template<int gf_size>
inline const float* get_Hadamard_line(const int line) {
         if constexpr( gf_size ==    8 ) { return &Hadamard_8x8[line][0];   }
    else if constexpr( gf_size ==   16 ) { return &Hadamard_16x16[line][0]; }
    else if constexpr( gf_size ==   32 ) { return &Hadamard_32x32[line][0]; }
    else if constexpr( gf_size ==   64 ) { return &Hadamard_64x64[line][0]; }
    else if constexpr( gf_size ==  128 ) { return &Hadamard_128x128[line][0]; }
    else if constexpr( gf_size ==  256 ) { return &Hadamard_256x256[line][0]; }
    else if constexpr( gf_size ==  512 ) { return &Hadamard_512x512[line][0]; }
    else if constexpr( gf_size == 1024 ) { return &Hadamard_1024x1024[line][0]; }
    else if constexpr( gf_size == 2048 ) { return &Hadamard_2048x2048[line][0]; }
    else if constexpr( gf_size == 4096 ) { return &Hadamard_4096x4096[line][0]; }
    else {
        static_assert(false, "We never should be here !");
    }
}