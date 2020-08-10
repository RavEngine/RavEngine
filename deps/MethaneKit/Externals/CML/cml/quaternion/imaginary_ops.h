/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_imaginary_ops_h
#define	cml_quaternion_imaginary_ops_h

#include <cml/quaternion/imaginary_node.h>

namespace cml {

/** Return an expression node for the imaginary part of @c q.  @c q is
 * stored by const reference in the node.
 */
template<class Sub> auto imaginary(const readable_quaternion<Sub>& q)
-> imaginary_node<const Sub&>;

/** Return an expression node for imaginary part of the temporary
 * subexpression @c q. @c q is stored by value in the node (via std::move).
 */
template<class Sub> auto imaginary(readable_quaternion<Sub>&& q)
-> imaginary_node<Sub&&>;

} // namespace cml

#define __CML_QUATERNION_IMAGINARY_OPS_TPP
#include <cml/quaternion/imaginary_ops.tpp>
#undef __CML_QUATERNION_IMAGINARY_OPS_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
