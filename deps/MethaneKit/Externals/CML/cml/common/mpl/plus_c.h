/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_mpl_plus_c_h
#define	cml_common_mpl_plus_c_h

namespace cml {

/** Helper to add two integral constants.
 *
 * @note This also avoids spurious VC14 "integral constant overflow"
 * warnings.
 */
template<int a, int b> struct plus_c {
  static const int value = a + b;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
