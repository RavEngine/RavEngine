/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_quaternion_size_checking_h
#define	cml_quaternion_size_checking_h

#include <cml/common/mpl/int_c.h>
#include <cml/common/mpl/enable_if_array.h>
#include <cml/common/exception.h>
#include <cml/quaternion/fwd.h>

namespace cml {

/** Exception thrown when run-time size checking is enabled, and the
 * operand of a quaternion expression does not have 4 elements.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_QUATERNION_SIZE_CHECKS at compile time.
 */
struct quaternion_size_error : std::runtime_error {
  quaternion_size_error()
    : std::runtime_error("incorrect quaternion expression size") {}
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
