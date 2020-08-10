/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_hadamard_product_h
#define	cml_vector_hadamard_product_h

#include <cml/vector/binary_ops.h>

namespace cml {

/** Elementwise (Hadamard) product of two vectors. */
template<class Sub1, class Sub2,
  enable_if_vector_t<Sub1>* = nullptr, enable_if_vector_t<Sub2>* = nullptr>
inline auto hadamard(Sub1&& sub1, Sub2&& sub2)
-> decltype(make_vector_binary_node<binary_multiply_t<Sub1,Sub2>>(
    std::forward<Sub1>(sub1), std::forward<Sub2>(sub2)))
{
  return make_vector_binary_node<binary_multiply_t<Sub1,Sub2>>
    (std::forward<Sub1>(sub1), std::forward<Sub2>(sub2));
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
