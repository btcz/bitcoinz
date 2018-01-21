/** @file
 *****************************************************************************

 Implementation of interfaces for (square-and-multiply) exponentiation.

 See exponentiation.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef EXPONENTIATION_TCC_
#define EXPONENTIATION_TCC_

#include "common/utils.hpp"

namespace libsnark {

template<typename FieldT, mp_size_t m>
FieldT power(const FieldT &base, const bigint<m> &exponent)
{
    FieldT result = FieldT::one();

    bool found_one = false;

#ifdef _WIN32
    for (int64_t i = exponent.max_bits() - 1; i >= 0; --i)
#else
    for (long i = exponent.max_bits() - 1; i >= 0; --i)
#endif
    {
        if (found_one)
        {
            result = result * result;
        }

        if (exponent.test_bit(i))
        {
            found_one = true;
            result = result * base;
        }
    }

    return result;
}

template<typename FieldT>
#ifdef _WIN32
FieldT power(const FieldT &base, const uint64_t exponent)
#else
FieldT power(const FieldT &base, const unsigned long exponent)
#endif
{
    return power<FieldT>(base, bigint<1>(exponent));
}

} // libsnark

#endif // EXPONENTIATION_TCC_
