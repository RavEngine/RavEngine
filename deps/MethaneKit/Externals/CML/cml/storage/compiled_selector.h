/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_storage_compiled_selector_h
#define	cml_storage_compiled_selector_h

#include <cml/common/size_tags.h>
#include <cml/common/storage_tags.h>
#include <cml/common/memory_tags.h>
#include <cml/storage/type_util.h>

namespace cml {

/* Forward declarations: */
template<int Size1 = -1, int Size2 = -1, class Tag = void> struct compiled;

/** Base selector to choose compiled storage types.
 *
 * @tparam Size1 First dimension size.
 *
 * @tparam Size2 Second dimension size.
 *
 * @tparam Tag Tag specifying the type of storage (e.g.
 * vector_storage_tag).  This is set by instantiating @c rebind with the
 * required tag.
 */
template<int Size1, int Size2>
struct compiled<Size1, Size2, void>
{
  /** Rebind the base selector to the required type. */
  template<class Rebind> struct rebind {
    typedef compiled<Size1, Size2, Rebind>		other;
  };

  /** Make a partially bound selector with size @c N. */
  template<int N> struct resize {
    typedef compiled<N>					type;
  };

  /** Make a partially bound selector with size @c R x @c C. */
  template<int R, int C> struct reshape {
    typedef compiled<R, C>				type;
  };
};

/** Specialized selector for fixed-size compiled vectors. */
template<int Size>
struct compiled<Size, -1, vector_storage_tag>
{
  typedef compiled<>					selector_type;
  typedef compiled<>					unbound_type;
  typedef compiled<Size>				proxy_type;
  typedef vector_storage_tag				storage_tag;
  typedef fixed_size_tag				size_tag;
  typedef compiled_memory_tag				memory_tag;

  /** Constant for the array size. */
  static const int array_size = Size;

  /** Make a partially bound selector with size @c N. */
  template<int N> struct resize {
    typedef compiled<N>					type;
  };
};

/** Specialized selector for fixed-size compiled matrices. */
template<int Size1, int Size2>
struct compiled<Size1, Size2, matrix_storage_tag>
{
  typedef compiled<>					selector_type;
  typedef compiled<>					unbound_type;
  typedef compiled<Size1, Size2>			proxy_type;
  typedef matrix_storage_tag				storage_tag;
  typedef fixed_size_tag				size_tag;
  typedef compiled_memory_tag				memory_tag;

  /** Constant for the number of array rows. */
  static const int array_rows = Size1;

  /** Constant for the number of array columns. */
  static const int array_cols = Size2;

  /** Make a partially bound selector with size @c R x @c C. */
  template<int R, int C> struct reshape {
    typedef compiled<R, C>				type;
  };
};

/** Specialized selector for quaternions. */
template<>
struct compiled<4, -1, quaternion_storage_tag>
{
  typedef compiled<>					selector_type;
  typedef compiled<>					unbound_type;
  typedef compiled<>					proxy_type;
  typedef quaternion_storage_tag			storage_tag;
  typedef fixed_size_tag				size_tag;
  typedef compiled_memory_tag				memory_tag;

  /** Constant for the array size. */
  static const int array_size = 4;

  /** Make a partially bound selector with size @c N. */
  template<int N> struct resize {
    static_assert(N == 4, "invalid quaternion storage size");
    typedef compiled<4>					type;
  };
};

/** is_storage_selector for compiled<>. */
template<int Size1, int Size2, class Tag>
struct is_storage_selector<compiled<Size1, Size2, Tag>> {
  static const bool value = true;
};

/** Helper to disambiguate compiled<> types. */
template<int R1, int C1, class Tag1, int R2, int C2, class Tag2>
struct storage_disambiguate<compiled<R1, C1, Tag1>, compiled<R2, C2, Tag2>>
{
  typedef compiled<>					type;
};


/** For compatibility with CML1. */
template<int Size1 = -1, int Size2 = -1> using fixed = compiled<Size1, Size2>;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
