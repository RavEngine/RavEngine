/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_types_h
#define	cml_quaternion_types_h

#include <cml/storage/selectors.h>
#include <cml/quaternion/quaternion.h>

namespace cml {

/** @defgroup quaternion_types Predefined Quaternion Types */
/*@{*/

// Imaginary first, positive cross:
typedef quaternion<
  float, fixed<>, imaginary_first, positive_cross>	quaternionf_ip;
typedef quaternion<
  float, fixed<>, imaginary_first, positive_cross>	quaternionf_p;
typedef quaternion<float>				quaternionf;

typedef quaternion<
  double, fixed<>, imaginary_first, positive_cross>	quaterniond_ip;
typedef quaternion<
  double, fixed<>, imaginary_first, positive_cross>	quaterniond_p;
typedef quaternion<double>				quaterniond;


// Imaginary first, negative cross:
typedef quaternion<
  float, fixed<>, imaginary_first, negative_cross>	quaternionf_in;
typedef quaternion<
  float, fixed<>, imaginary_first, negative_cross>	quaternionf_n;

typedef quaternion<
  double, fixed<>, imaginary_first, negative_cross>	quaterniond_in;
typedef quaternion<
  double, fixed<>, imaginary_first, negative_cross>	quaterniond_n;


// Real first, positive cross:
typedef quaternion<
  float, fixed<>, real_first, positive_cross>		quaternionf_rp;
typedef quaternion<
  double, fixed<>, real_first, positive_cross>		quaterniond_rp;


// Real first, negative cross:
typedef quaternion<
  float, fixed<>, real_first, negative_cross>		quaternionf_rn;
typedef quaternion<
  double, fixed<>, real_first, negative_cross>		quaterniond_rn;

/*@}*/

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
