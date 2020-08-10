/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_product_h
#define	cml_quaternion_product_h

#include <cml/quaternion/readable_quaternion.h>
#include <cml/quaternion/promotion.h>

namespace cml {

/** Multiply two quaternions, and return the result as a temporary. */
template<class Sub1, class Sub2,
  enable_if_quaternion_t<Sub1>* = nullptr,
  enable_if_quaternion_t<Sub2>* = nullptr>
auto operator*(Sub1&& sub1, Sub2&& sub2)
-> quaternion_promote_t<
  actual_operand_type_of_t<decltype(sub1)>,
  actual_operand_type_of_t<decltype(sub2)>>;

} // namespace cml

#define __CML_QUATERNION_PRODUCT_TPP
#include <cml/quaternion/product.tpp>
#undef __CML_QUATERNION_PRODUCT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
