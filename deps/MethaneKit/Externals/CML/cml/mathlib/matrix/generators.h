/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
 /** @file
   */

#pragma once

#ifndef	cml_mathlib_matrix_generators_h
#define	cml_mathlib_matrix_generators_h

#include <cml/matrix/fixed_compiled.h>

/** @defgroup mathlib_matrix_generators Matrix Generator Functions */

namespace cml {

/** @addtogroup mathlib_matrix_generators */
/*@{*/

/** @defgroup mathlib_matrix_generators_zero Zero Matrix Generators */
/*@{*/

/** Return a fixed-size double-precision zero matrix. */
template<int Rows, int Cols> inline auto zero()
-> matrix<double, compiled<Rows, Cols>>
{
  return matrix<double, compiled<Rows, Cols>>().zero();
}

/** Return the 2x2 zero matrix */
inline auto zero_2x2() -> decltype(zero<2, 2>()) { return zero<2, 2>(); }

/** Return the 3x3 zero matrix */
inline auto zero_3x3() -> decltype(zero<3, 3>()) { return zero<3, 3>(); }

/** Return the 4x4 zero matrix */
inline auto zero_4x4() -> decltype(zero<4, 4>()) { return zero<4, 4>(); }

/*@}*/

/** @defgroup mathlib_matrix_generators_identity Identity Matrix Generators */
/*@{*/

/** Return a fixed-size double-precision identity matrix. */
template<int Rows, int Cols> inline auto identity()
-> matrix<double, compiled<Rows, Cols>>
{
  return matrix<double, compiled<Rows, Cols>>().identity();
}


/** Return the 2x2 identity matrix */
inline auto identity_2x2()
-> decltype(identity<2,2>()) { return identity<2, 2>(); }

/** Return the 3x3 identity matrix */
inline auto identity_3x3()
-> decltype(identity<3, 3>()) { return identity<3, 3>(); }

/** Return the 4x4 identity matrix */
inline auto identity_4x4()
-> decltype(identity<4, 4>()) { return identity<4, 4>(); }

/*@}*/

/*@}*/

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2


