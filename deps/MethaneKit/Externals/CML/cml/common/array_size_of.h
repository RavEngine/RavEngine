/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_array_size_of_h
#define	cml_common_array_size_of_h

#include <type_traits>

namespace cml {
namespace detail {

/** Helper defining a typedef @c type that is int if @c T is an integral
 * type, or @c T otherwise.
 */
template<class T> struct int_if_integral {
  typedef typename std::conditional<
    std::is_integral<T>::value, int, T>::type type;
};

} // namespace detail


/** Specializable compile-time array size. */
template<class Array, class Enable = void> struct array_size_of_c;

/** Specializable compile-time array row count. */
template<class Array, class Enable = void> struct array_rows_of_c;

/** Specializable compile-time array row count. */
template<class Array, class Enable = void> struct array_cols_of_c;

/** Compile-time size of an array. */
template<class Array> struct array_size_of_c<
  Array, typename std::enable_if<std::is_array<Array>::value>::type>
{
  static const int value = int(std::extent<Array>::value);
};

/** Compile-time size of an object implementing an array_size integral or
 * enum member.
 */
template<class Array> struct array_size_of_c<Array
, typename std::enable_if<Array::array_size == Array::array_size>::type>
{
  static const int value = Array::array_size;
};

/** Return the size of @c array if it implements the size() method. */
template<class Array> inline auto
array_size_of(const Array& array)
-> typename detail::int_if_integral<decltype(array.size())>::type
{
  typedef typename detail::int_if_integral<
    decltype(array.size())>::type result_type;
  return result_type(array.size());
}

/** Return the size of a fixed-length array. */
template<class Array> inline int array_size_of(const Array&,
  typename std::enable_if<std::is_array<Array>::value>::type* = 0)
{
  return int(array_size_of_c<Array>::value);
}


/** Compile-time size of an object implementing an array_rows integral or
 * enum member.
 */
template<class Array> struct array_rows_of_c<Array,
  typename std::enable_if<Array::array_rows == Array::array_rows>::type>
{
  static const int value = int(Array::array_rows);
};

/** Compile-time size of an object implementing an array_cols integral or
 * enum member.
 */
template<class Array> struct array_cols_of_c<Array,
  typename std::enable_if<Array::array_cols == Array::array_cols>::type>
{
  static const int value = int(Array::array_cols);
};

/** Return the number of rows of @c array if it implements the rows()
 * method.
 */
template<class Array> inline auto
array_rows_of(const Array& array) -> decltype(array.rows()) {
  return array.rows();
}

/** Return the number of columns @c array if it implements the cols()
 * method.
 */
template<class Array> inline auto
array_cols_of(const Array& array) -> decltype(array.cols()) {
  return array.cols();
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
