/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_subvector_ops_h
#define	cml_vector_subvector_ops_h

#include <cml/vector/subvector_node.h>

namespace cml {

/** Return an expression node for the subvector @c i of @c sub. @c sub is
 * stored by const reference in the node.
 */
template<class Sub> auto subvector(const readable_vector<Sub>& sub, int i)
-> subvector_node<const Sub&>;

/** Return an expression node for subvector @c i of the temporary
 * subexpression @c sub. @c sub is stored by value in the node (via
 * std::move).
 */
template<class Sub> auto subvector(readable_vector<Sub>&& sub)
-> subvector_node<Sub&&>;

} // namespace cml

#define __CML_VECTOR_SUBVECTOR_OPS_TPP
#include <cml/vector/subvector_ops.tpp>
#undef __CML_VECTOR_SUBVECTOR_OPS_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
