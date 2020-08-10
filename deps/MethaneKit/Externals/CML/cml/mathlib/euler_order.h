/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_mathlib_euler_order_h
#define	cml_mathlib_euler_order_h

namespace cml {

/** Constants for specifying the order of Euler angle computations. */
enum euler_order
{
  euler_order_xyz, // 0x00 [0000]
  euler_order_xyx, // 0x01 [0001]
  euler_order_xzy, // 0x02 [0010]
  euler_order_xzx, // 0x03 [0011]
  euler_order_yzx, // 0x04 [0100]
  euler_order_yzy, // 0x05 [0101]
  euler_order_yxz, // 0x06 [0110]
  euler_order_yxy, // 0x07 [0111]
  euler_order_zxy, // 0x08 [1000]
  euler_order_zxz, // 0x09 [1001]
  euler_order_zyx, // 0x0A [1010]
  euler_order_zyz  // 0x0B [1011]
};

/** For CML1 compatibility. */
typedef euler_order EulerOrder;

/** Unpack Euler ordering @c order as three integers in {0, 1, 2}.  If @c
 * odd is true, the ordering is swapped.  If @c repeat is true, one axis
 * appears twice in the order.
 */
inline void unpack_euler_order(
  euler_order order, int& i, int& j, int& k, bool& odd, bool& repeat
  )
{
  enum { REPEAT = 0x01, ODD = 0x02, AXIS = 0x0C };

  repeat = order & REPEAT;
  odd = ((order & ODD) == ODD);
  int offset = odd;
  i = (order & AXIS) % 3;
  j = (i + 1 + offset) % 3;
  k = (i + 2 - offset) % 3;
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
