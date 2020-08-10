/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_random_unit_h
#define	cml_mathlib_random_unit_h

#include <cml/vector/fwd.h>

namespace cml {

/** @addtogroup mathlib_vector_misc */
/*@{*/

/** Replace @c n with a uniformly random unit vector, using the specified
 * random number generator @c gen.
 *
 * This generates the coordinates of the vector from a Gaussian
 * distribution having mean 0 and standard deviation 1.  The vector must
 * have a floating point coordinate type, otherwise a compile-time error
 * will be raised.
 *
 * @note @c n must have non-zero size on entry.  The coordinates of @c n
 * are overwritten on exit.
 *
 * @note @c gen must be compatible with the standard (STL) random number
 * engines.
 *
 * @note Use std::srand() to seed the random number generator.
 *
 * @throws minimum_vector_size_error if @c n has zero size.
 */
template<class Sub, class RNG> void
  random_unit(writable_vector<Sub>& n, RNG& gen);

/** Replace @c n with a uniformly random unit vector.
 *
 * This generates the coordinates of the vector from a Gaussian
 * distribution having mean 0 and standard deviation 1.  The vector must
 * have a floating point coordinate type, otherwise a compile-time error
 * will be raised.
 *
 * @note @c n must have non-zero size on entry.  The coordinates of @c n
 * are overwritten on exit.
 *
 * @note Use std::srand() to seed the random number generator.
 *
 * @throws minimum_vector_size_error if @c n has zero size.
 */
template<class Sub> void random_unit(writable_vector<Sub>& n);

/** Generate a random unit vector @c n within a cone having unit direction
 * @c d and non-zero half-angle @c a no greater than 90 deg, specified in
 * radians.  This function works for any vector dimension
 *
 * @c n must have a floating point coordinate type, otherwise a
 * compile-time error will be raised.
 *
 * @note @c d is assumed to be normalized.
 *
 * @note Use std::srand() to seed the random number generator.
 *
 * @warning The algorithm was independently developed by the authors, and
 * has not yet been proven to generate vectors uniformly distributed over
 * the cone.
 *
 * @throws std::invalid_argument if @c a < 0 or @c a > 90 deg.
 *
 * @throws incompatible_vector_size_error if @c n.size() != @c d.size, and
 * @c d is dynamically-sized and @c n is fixed-size.  If @c n is
 * dynamically-sized, it will be resized to @c d.size().  If both @c n and
 * @c d are fixed-size, then the size is checked at compile-time.
 */
template<class Sub1, class Sub2, class Scalar> void random_unit(
  writable_vector<Sub1>& n, const readable_vector<Sub2>& d, const Scalar& a);

/*@}*/

} // namespace cml

#define __CML_MATHLIB_RANDOM_UNIT_TPP
#include <cml/mathlib/random_unit.tpp>
#undef __CML_MATHLIB_RANDOM_UNIT_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
