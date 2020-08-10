/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_temporary_h
#define	cml_common_temporary_h

namespace cml {

/** Specializable struct to obtain a temporary for a specified expression
 * type, possibly using SFINAE.  Specializations should typedef "type" as a
 * temporary for @c T.
 */
template<class T, class Enable = void> struct temporary_of;

/** Convenience alias for temporary_of. */
template<class T> using temporary_of_t = typename temporary_of<T>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
