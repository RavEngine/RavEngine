/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_SUBVECTOR_OPS_TPP
#error "vector/subvector_ops.tpp not included correctly"
#endif

namespace cml {

template<class Sub> inline auto
subvector(const readable_vector<Sub>& sub, int i)
-> subvector_node<const Sub&>
{
  return subvector_node<const Sub&>(sub.actual(), i);
}

template<class Sub> inline auto
subvector(readable_vector<Sub>&& sub, int i)
-> subvector_node<Sub&&>
{
  return subvector_node<Sub&&>((Sub&&) sub, i);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
