/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_axis_order_h
#define	cml_mathlib_axis_order_h

#include <cml/mathlib/euler_order.h>

namespace cml {

/** Specify 3D axis ordering. */
enum axis_order {
  axis_order_xyz = euler_order_xyz, // 0x00 [0000]
  axis_order_xzy = euler_order_xzy, // 0x02 [0010]
  axis_order_yzx = euler_order_yzx, // 0x04 [0100]
  axis_order_yxz = euler_order_yxz, // 0x06 [0110]
  axis_order_zxy = euler_order_zxy, // 0x08 [1000]
  axis_order_zyx = euler_order_zyx  // 0x0A [1010]
};

/** Specify 2D axis ordering. */
enum axis_order2D {
  axis_order_xy = axis_order_xyz, // 0x00 [0000]
  axis_order_yx = axis_order_yxz, // 0x06 [0110]
};

/** For CML1 compatibility. */
typedef axis_order AxisOrder;
typedef axis_order2D AxisOrder2D;

inline void unpack_axis_order(
  axis_order order, int& i, int& j, int& k, bool& odd
  )
{
  enum { ODD = 0x02, AXIS = 0x0C };

  odd = ((order & ODD) == ODD);
  int offset = int(odd);
  i = (order & AXIS) % 3;
  j = (i + 1 + offset) % 3;
  k = (i + 2 - offset) % 3;
}

inline void unpack_axis_order2D(
  axis_order2D order, int& i, int& j, bool& odd
  )
{
  enum { ODD = 0x02, AXIS = 0x0C };

  odd = ((order & ODD) == ODD);
  int offset = int(odd);
  i = (order & AXIS) % 3;
  j = (i + 1 + offset) % 3;
}

inline axis_order pack_axis_order(int i, bool odd) {
  return axis_order((i << 2) | (int(odd) << 1));
}

inline axis_order swap_axis_order(axis_order order)
{
  int i, j, k;
  bool odd;
  unpack_axis_order(order, i, j, k, odd);
  return pack_axis_order(j, !odd);
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
