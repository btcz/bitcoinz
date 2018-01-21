/** @file
 *****************************************************************************
 Implementation of misc math and serialization utility functions
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdarg>
#include "common/utils.hpp"

namespace libsnark {

#ifdef _WIN32
uint64_t log2(uint64_t n)
#else
size_t log2(size_t n)
#endif
/* returns ceil(log2(n)), so 1ul<<log2(n) is the smallest power of 2,
   that is not less than n. */
{
#ifdef _WIN32
    uint64_t r = ((n & (n-1)) == 0 ? 0 : 1); // add 1 if n is not power of 2
#else
    size_t r = ((n & (n-1)) == 0 ? 0 : 1); // add 1 if n is not power of 2
#endif

    while (n > 1)
    {
        n >>= 1;
        r++;
    }

    return r;
}

#ifdef _WIN32
uint64_t bitreverse(uint64_t n, const uint64_t l)
#else
size_t bitreverse(size_t n, const size_t l)
#endif
{
#ifdef _WIN32
    uint64_t r = 0;
    for (uint64_t k = 0; k < l; ++k)
#else
    size_t r = 0;
    for (size_t k = 0; k < l; ++k)
#endif
    {
        r = (r << 1) | (n & 1);
        n >>= 1;
    }
    return r;
}

#ifdef _WIN32
bit_vector int_list_to_bits(const std::initializer_list<uint64_t> &l, const uint64_t wordsize)
#else
bit_vector int_list_to_bits(const std::initializer_list<unsigned long> &l, const size_t wordsize)
#endif
{
    bit_vector res(wordsize*l.size());
#ifdef _WIN32
    for (uint64_t i = 0; i < l.size(); ++i)
#else
    for (size_t i = 0; i < l.size(); ++i)
#endif
    {
#ifdef _WIN32
        for (uint64_t j = 0; j < wordsize; ++j)
#else
        for (size_t j = 0; j < wordsize; ++j)
#endif
        {
#ifdef _WIN32
            res[i*wordsize + j] = (*(l.begin()+i) & (UINT64_C(1)<<(wordsize-1-j)));
#else
            res[i*wordsize + j] = (*(l.begin()+i) & (1ul<<(wordsize-1-j)));
#endif
        }
    }
    return res;
}

#ifdef _WIN32
int64_t div_ceil(int64_t x, int64_t y)
#else
long long div_ceil(long long x, long long y)
#endif
{
    return (x + (y-1)) / y;
}

bool is_little_endian()
{
    uint64_t a = 0x12345678;
    unsigned char *c = (unsigned char*)(&a);
    return (*c = 0x78);
}

std::string FORMAT(const std::string &prefix, const char* format, ...)
{
#ifdef _WIN32
    const static uint64_t MAX_FMT = 256;
#else
    const static size_t MAX_FMT = 256;
#endif
    char buf[MAX_FMT];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, MAX_FMT, format, args);
    va_end(args);

    return prefix + std::string(buf);
}

void serialize_bit_vector(std::ostream &out, const bit_vector &v)
{
    out << v.size() << "\n";
#ifdef _WIN32
    for (uint64_t i = 0; i < v.size(); ++i)
#else
    for (size_t i = 0; i < v.size(); ++i)
#endif
    {
        out << v[i] << "\n";
    }
}

void deserialize_bit_vector(std::istream &in, bit_vector &v)
{
#ifdef _WIN32
    uint64_t size;
#else
    size_t size;
#endif
    in >> size;
    v.resize(size);
#ifdef _WIN32
    for (uint64_t i = 0; i < size; ++i)
#else
    for (size_t i = 0; i < size; ++i)
#endif
    {
        bool b;
        in >> b;
        v[i] = b;
    }
}
} // libsnark
