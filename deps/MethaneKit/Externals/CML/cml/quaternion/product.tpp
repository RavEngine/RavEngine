/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_QUATERNION_PRODUCT_TPP
#error "quaternion/product.tpp not included correctly"
#endif

#include <cml/vector/dot.h>
#include <cml/vector/cross.h>
#include <cml/vector/scalar_ops.h>
#include <cml/vector/binary_ops.h>

namespace cml {

template<class Sub1, class Sub2,
  enable_if_quaternion_t<Sub1>*, enable_if_quaternion_t<Sub2>*>
inline auto operator*(Sub1&& sub1, Sub2&& sub2)
-> quaternion_promote_t<
  actual_operand_type_of_t<decltype(sub1)>,
  actual_operand_type_of_t<decltype(sub2)>>
{
  typedef quaternion_promote_t<
    actual_operand_type_of_t<decltype(sub1)>,
    actual_operand_type_of_t<decltype(sub2)>>		result_type;
  typedef order_type_trait_of_t<result_type>		order_type;
  typedef cross_type_trait_of_t<result_type>		cross_type;

  /* Local enum for the order: */
  enum {
    W = order_type::W,
    X = order_type::X,
    Y = order_type::Y,
    Z = order_type::Z
  };

  /* If sub1 = (w1, v1) and sub2 = (w2, v2), where w is the real part and
   * v is the imaginary part of the quaternion, then
   *
   *   sub1 * sub2 = (w1*w2 - dot(v1,v2), w1*v2 + w2*v1 {+/-} cross(v1,v2))
   *
   * {+/-} is determined by cross_type:
   */
  auto real
    = sub1[W]*sub2[W] - cml::dot(sub1.imaginary(), sub2.imaginary());
  auto cross = std::is_same<cross_type, positive_cross>::value
    ? cml::cross(sub1.imaginary(), sub2.imaginary())
    : cml::cross(sub2.imaginary(), sub1.imaginary())
    ;
  auto imaginary
    = sub1[W]*sub2.imaginary() + sub2[W]*sub1.imaginary() + cross;

  /* Done: */
  return result_type(real, imaginary);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
