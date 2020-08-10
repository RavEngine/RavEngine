/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */


#ifndef __CML_QUATERNION_CONJUGATE_OPS_TPP
#error "quaternion/conjugate_ops.tpp not included correctly"
#endif

namespace cml {

template<class Sub> inline auto
conjugate(const readable_quaternion<Sub>& q)
-> conjugate_node<const Sub&>
{
  return conjugate_node<const Sub&>(q.actual());
}

template<class Sub> inline auto
conjugate(readable_quaternion<Sub>&& q)
-> conjugate_node<Sub&&>
{
  return conjugate_node<Sub&&>((Sub&&) q);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
