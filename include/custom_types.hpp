#pragma once
//
//
#include "code.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace I_type
{
constexpr int NBITS = 12;

}
//
//
// For generic NB polar decoders
//
//
struct symbols_t
{
	float value[_GF_];
	bool is_freq;
};
//
//
inline symbols_t convert_to_symbols_t(const float *values, const int gf_size, const bool is_freq)
{
	symbols_t result;
	result.is_freq = is_freq;
	for (int i = 0; i < gf_size; i++)
	{
		result.value[i] = values[i];
	}
	return result;
}
//
//
inline void convert_from_symbols_t(float *dst, const symbols_t &src, const int gf_size)
{
	for (int i = 0; i < gf_size; i++)
	{
		dst[i] = src.value[i];
	}
}
//
//
//
// For specialized NB polar decoders
//
//
//
template <int gf_size>
struct symbols_s
{
	float value[gf_size];
};
//
//
template <int gf_size>
inline symbols_s<gf_size> convert_to_symbols_s(const float *values)
{
	symbols_s<gf_size> result;
	for (int i = 0; i < gf_size; i++)
	{
		result.value[i] = values[i];
	}
	return result;
}
//
//
template <int gf_size>
inline void convert_from_symbols_s(float *dst, const symbols_s<gf_size> &src)
{
	for (int i = 0; i < gf_size; i++)
	{
		dst[i] = src.value[i];
	}
}
//
//
//
// For specialized NB polar decoders
//
//
//
template <int gf_size>
struct symbols_i
{
	int32_t value[gf_size];
	bool is_freq;
};
//
//
#if 0
#define NEW_QUANTIF 0
#define symbols_i_iscale 268435456
#define symbols_i_fscale 268435456.f
#define symbols_i_shift 28
#elif 0
#define NEW_QUANTIF 1
#define symbols_i_iscale (int64_t)65535
#define symbols_i_fscale 65535.f
#define symbols_i_shift 16
#elif 0
#define NEW_QUANTIF 1
#define symbols_i_iscale (int64_t)131071
#define symbols_i_fscale 131071.f
#define symbols_i_shift 17
#elif 1
#define NEW_QUANTIF 1
#define symbols_i_iscale (int64_t)262144
#define symbols_i_fscale 262144.f
#define symbols_i_shift 18
#else
#define NEW_QUANTIF 1
#define symbols_i_iscale (int64_t)1048576
#define symbols_i_fscale 1048576.f
#define symbols_i_shift 20
#endif
//
//
template <int gf_size>
inline void show(int32_t *symb)
{
	for (int i = 0; i < gf_size; i++)
	{
		if (i == 0)
			printf("%3d :", i);
		else if (i % 8 == 0)
			printf("\n%3d :", i);
		printf("%+12d ", symb[i]);
	}
	printf("\n");
}
template <int gf_size>
inline void show(int64_t *symb)
{
	for (int i = 0; i < gf_size; i++)
	{
		if (i == 0)
			printf("%3d :", i);
		else if (i % 8 == 0)
			printf("\n%3d :", i);
		printf("%+12lld ", symb[i]);
	}
	printf("\n");
}
//
//
template <int gf_size>
inline bool is_null(int32_t *symb)
{
	int64_t sum = 0;
	for (int i = 0; i < gf_size; i++)
	{
		sum += symb[i];
	}
	return (sum == 0);
}
//
//
template <int gf_size>
inline bool is_in_range(int32_t *symb)
{
	for (int i = 0; i < gf_size; i++)
	{
		if (symb[i] > symbols_i_iscale)
			return false;
		if (symb[i] < -symbols_i_iscale)
			return false;
	}
	return true;
}
//
//
template <int gf_size>
inline symbols_i<gf_size> convert_to_symbols_i(const symbols_s<gf_size> symb)
{
	symbols_i<gf_size> result;
	for (int i = 0; i < gf_size; i++)
	{
		const double v = symb.value[i];
		const int32_t w = (int32_t)round(v * symbols_i_fscale);
		result.value[i] = w;
#if 0
        printf("%3d : %8.6f = %d\n", i, symb.value[i], result.value[i]);
#endif
	}
	result.is_freq = false;
	return result;
}
//
//
template <int gf_size>
inline void convert_from_symbols_i(float *dst, const symbols_i<gf_size> &src)
{
	for (int i = 0; i < gf_size; i++)
	{
		const double v = src.value[i];
		const float w = (float)(v / (double)symbols_i_fscale);
		dst[i] = w;
	}
}
//
//
template <int gf_size>
struct symbols_i32
{
	int32_t value[gf_size];
	bool is_freq;
};
#if 0
template <int gf_size>
inline symbols_i32<gf_size> convert_to_symbols_i32(const symbols_s<gf_size> symb)
{

	symbols_i32<gf_size> result;
	float max1 = *std::max_element(symb.value, symb.value + gf_size);

	for (size_t i = 0; i < gf_size; ++i)
	{
		double normalized = static_cast<double>(symb.value[i]) / max1;
		double scaled = normalized * (0.999 * static_cast<double>(1u << 31));
		result.value[i] = static_cast<int32_t>(std::floor(scaled));
	}
	LZC_normalize<gf_size, I_type::NBITS>(result.value);
	result.is_freq = false;
	return result;
}
#endif
