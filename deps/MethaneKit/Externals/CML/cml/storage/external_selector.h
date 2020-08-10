/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_storage_external_selector_h
#define	cml_storage_external_selector_h

#include <cml/storage/compiled_selector.h>
#include <cml/storage/allocated_selector.h>

namespace cml {

/* Forward declarations: */
template<int Size1 = -1, int Size2 = -1, class Tag = void> struct external;

/** Base selector to choose storage types that wrap a pointer or reference
 * to array data.
 *
 * @tparam Size1 First dimension size.
 *
 * @tparam Size2 Second dimension size.
 *
 * @tparam Tag Tag specifying the type of storage (e.g.
 * vector_storage_tag).  This is set by instantiating @c rebind with the
 * required tag.
 */
template<int Size1, int Size2> struct external<Size1, Size2, void>
{
  /** Rebind the base selector to the required type. */
  template<class Rebind> struct rebind {
    typedef external<Size1, Size2, Rebind>		other;
  };

  /** Make a partially bound selector with size @c N. */
  template<int N> struct resize {
    typedef external<N>					type;
  };

  /** Make a partially bound selector with size @c R x @c C. */
  template<int R, int C> struct reshape {
    typedef external<R, C>				type;
  };
};

/** Specialized selector for dynamic-size external vectors. */
template<>
struct external<-1, -1, vector_storage_tag>
{
  typedef external<>					selector_type;
  typedef external<>					unbound_type;
  typedef allocated<>					proxy_type;
  typedef vector_storage_tag				storage_tag;
  typedef dynamic_size_tag				size_tag;
  typedef external_memory_tag				memory_tag;

  /** Unspecified array size. */
  static const int array_size = -1;

  /** Make a partially bound selector with size @c N. */
  template<int N> struct resize {
    typedef external<N>					type;
  };
};

/** Specialized selector for fixed-size external vectors. */
template<int Size>
struct external<Size, -1, vector_storage_tag>
{
  typedef external<>					selector_type;
  typedef external<>					unbound_type;
  typedef compiled<Size>				proxy_type;
  typedef vector_storage_tag				storage_tag;
  typedef fixed_size_tag				size_tag;
  typedef external_memory_tag				memory_tag;

  /** Constant for the array size. */
  static const int array_size = Size;

  /** Make a partially bound selector with size @c N. */
  template<int N> struct resize {
    typedef external<N>					type;
  };
};

/** Specialized selector for dynamic-size external matrices. */
template<> struct external<-1, -1, matrix_storage_tag>
{
  typedef external<>					selector_type;
  typedef external<>					unbound_type;
  typedef allocated<>					proxy_type;
  typedef matrix_storage_tag				storage_tag;
  typedef dynamic_size_tag				size_tag;
  typedef external_memory_tag				memory_tag;

  /** Unspecified array rows. */
  static const int array_rows = -1;

  /** Unspecified array columns. */
  static const int array_cols = -1;

  /** Make a partially bound selector with size @c R x @c C. */
  template<int R, int C> struct reshape {
    typedef external<R, C>				type;
  };
};

/** Specialized selector for fixed-size external matrices. */
template<int Size1, int Size2>
struct external<Size1, Size2, matrix_storage_tag>
{
  typedef external<>					selector_type;
  typedef external<>					unbound_type;
  typedef compiled<Size1, Size2>			proxy_type;
  typedef matrix_storage_tag				storage_tag;
  typedef fixed_size_tag				size_tag;
  typedef external_memory_tag				memory_tag;

  /** Constant for the number of array rows. */
  static const int array_rows = Size1;

  /** Constant for the number of array columns. */
  static const int array_cols = Size2;

  /** Make a partially bound selector with size @c R x @c C. */
  template<int R, int C> struct reshape {
    typedef external<R, C>				type;
  };
};

/** Specialized selector for external quaternions. */
template<>
struct external<4, -1, quaternion_storage_tag>
{
  typedef external<>					selector_type;
  typedef external<>					unbound_type;
  typedef compiled<4>					proxy_type;
  typedef quaternion_storage_tag			storage_tag;
  typedef fixed_size_tag				size_tag;
  typedef external_memory_tag				memory_tag;

  /** Constant for the array size. */
  static const int array_size = 4;

  /** Make a partially bound selector with size @c N. */
  template<int N> struct resize {
    static_assert(N == 4, "invalid quaternion storage size");
    typedef external<4>					type;
  };
};

/** is_storage_selector for external<>. */
template<int Size1, int Size2, class Tag>
struct is_storage_selector<external<Size1, Size2, Tag>> {
  static const bool value = true;
};

/** Helper to disambiguate external<> types. */
template<int R1, int C1, class Tag1, int R2, int C2, class Tag2>
struct storage_disambiguate<external<R1, C1, Tag1>, external<R2, C2, Tag2>>
{
  typedef external<>					type;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
