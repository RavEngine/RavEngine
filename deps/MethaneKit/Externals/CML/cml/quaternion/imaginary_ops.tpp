/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */


#ifndef __CML_QUATERNION_IMAGINARY_OPS_TPP
#error "quaternion/imaginary_ops.tpp not included correctly"
#endif

namespace cml {

template<class Sub> inline auto
imaginary(const readable_quaternion<Sub>& q)
-> imaginary_node<const Sub&>
{
  return imaginary_node<const Sub&>(q.actual());
}

template<class Sub> inline auto
imaginary(readable_quaternion<Sub>&& q)
-> imaginary_node<Sub&&>
{
  return imaginary_node<Sub&&>((Sub&&) q);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
