/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef FIELD_UTILS_HPP_
#define FIELD_UTILS_HPP_
#include <cstdint>

#include "common/utils.hpp"
#include "algebra/fields/bigint.hpp"

namespace libsnark {

// returns root of unity of order n (for n a power of 2), if one exists
template<typename FieldT>
#ifdef _WIN32
FieldT get_root_of_unity(const uint64_t n);
#else
FieldT get_root_of_unity(const size_t n);
#endif

template<typename FieldT>
#ifdef _WIN32
std::vector<FieldT> pack_int_vector_into_field_element_vector(const std::vector<uint64_t> &v, const uint64_t w);
#else
std::vector<FieldT> pack_int_vector_into_field_element_vector(const std::vector<size_t> &v, const size_t w);
#endif

template<typename FieldT>
#ifdef _WIN32
std::vector<FieldT> pack_bit_vector_into_field_element_vector(const bit_vector &v, const uint64_t chunk_bits);
#else
std::vector<FieldT> pack_bit_vector_into_field_element_vector(const bit_vector &v, const size_t chunk_bits);
#endif

template<typename FieldT>
std::vector<FieldT> pack_bit_vector_into_field_element_vector(const bit_vector &v);

template<typename FieldT>
std::vector<FieldT> convert_bit_vector_to_field_element_vector(const bit_vector &v);

template<typename FieldT>
bit_vector convert_field_element_vector_to_bit_vector(const std::vector<FieldT> &v);

template<typename FieldT>
bit_vector convert_field_element_to_bit_vector(const FieldT &el);

template<typename FieldT>
#ifdef _WIN32
bit_vector convert_field_element_to_bit_vector(const FieldT &el, const uint64_t bitcount);
#else
bit_vector convert_field_element_to_bit_vector(const FieldT &el, const size_t bitcount);
#endif

template<typename FieldT>
FieldT convert_bit_vector_to_field_element(const bit_vector &v);

template<typename FieldT>
void batch_invert(std::vector<FieldT> &vec);

} // libsnark
#include "algebra/fields/field_utils.tcc"

#endif // FIELD_UTILS_HPP_
