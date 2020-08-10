/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_constants_h
#define	cml_mathlib_constants_h

namespace cml {

/** Coordinate system handedness. */
enum AxisOrientation { left_handed, right_handed };

/** For CML1 compatibility. */
typedef AxisOrientation Handedness;

/** Perspective clipping type. */
enum ZClip { z_clip_neg_one, z_clip_zero };

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
