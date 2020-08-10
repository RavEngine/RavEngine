/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_matrix_h
#define	cml_matrix_matrix_h

#include <cml/common/basis_tags.h>
#include <cml/common/layout_tags.h>

namespace cml {

/** Specializable class for building vector types.
 *
 * This class encapsulates the notion of a matrix.
 *
 * @tparam Element The scalar type for matrix elements, with the following
 * operators defined: +, -, *, /, <, >, ==, = (assign).
 *
 * @tparam StorageType Storage type to use for holding the 2D array of
 * matrix elements.
 *
 * @tparam BasisOrient Encodes whether basis vectors for the matrix type
 * lie along rows or columns.  The default is col_basis, which is typical
 * in mathematics expositions, and implies that transformation matrices
 * pre-multiply vectors.  The opposite, row_basis, appears most frequently
 * in graphics, and implies that transformation matrices post-multiple
 * vectors.
 *
 * @tparam Layout Encodes whether rows or columns are physically contiguous
 * in memory.  The default is row_major, which is equivalent to the layout
 * of 2D arrays in C. The opposite, col_major, is equivalent to the layout
 * of 2D arrays in Fortran.
 *
 * @sa scalar_traits
 */
template<typename Element, class StorageType,
  typename BasisOrient = col_basis, typename Layout = row_major> class matrix;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
