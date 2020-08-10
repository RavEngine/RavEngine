/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_conjugate_ops_h
#define	cml_quaternion_conjugate_ops_h

#include <cml/quaternion/conjugate_node.h>

namespace cml {

/** Return an expression node for the conjugate part of @c q.  @c q is
 * stored by const reference in the node.
 */
template<class Sub> auto conjugate(const readable_quaternion<Sub>& q)
-> conjugate_node<const Sub&>;

/** Return an expression node for conjugate part of the temporary
 * subexpression @c q. @c q is stored by value in the node (via std::move).
 */
template<class Sub> auto conjugate(readable_quaternion<Sub>&& q)
-> conjugate_node<Sub&&>;

} // namespace cml

#define __CML_QUATERNION_CONJUGATE_OPS_TPP
#include <cml/quaternion/conjugate_ops.tpp>
#undef __CML_QUATERNION_CONJUGATE_OPS_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
