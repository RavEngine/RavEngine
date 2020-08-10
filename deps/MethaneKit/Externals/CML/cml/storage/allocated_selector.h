/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_storage_allocated_selector_h
#define	cml_storage_allocated_selector_h

#include <memory>
#include <cml/common/mpl/if_t.h>
#include <cml/common/size_tags.h>
#include <cml/common/storage_tags.h>
#include <cml/common/memory_tags.h>
#include <cml/storage/type_util.h>

// XXX Temporary, for fixed-size allocated proxies:
#include <cml/storage/compiled_selector.h>

namespace cml {

/* Forward declarations: */
template<class Allocator = std::allocator<void*>,
  int Size1 = -1, int Size2 = -1, class Tag = void> struct allocated;

/** Base selector to choose dynamically-allocated types (types with runtime
 * storage and either fixed or dynamic size).
 *
 * @tparam Size1 First dimension size.
 *
 * @tparam Size2 Second dimension size.
 *
 * @tparam Allocator Optional allocator type that must be compatible with
 * std::allocator. The default is std::allocator<void>.
 *
 * @tparam Tag Tag specifying the type of storage (e.g.
 * vector_storage_tag).  This is set by instantiating @c rebind with the
 * required tag.
 */
template<class Allocator, int Size1, int Size2>
struct allocated<Allocator, Size1, Size2>
{
  /** Rebind the base selector to the required type. */
  template<class Rebind> struct rebind {
    typedef allocated<Allocator, Size1, Size2, Rebind> other;
  };

  /** Make a partially bound selector with size @c N. */
  template<int N> struct resize {
    typedef allocated<Allocator, N>			type;
  };

  /** Make a partially bound selector with size @c R x @c C. */
  template<int R, int C> struct reshape {
    typedef allocated<Allocator, R, C>			type;
  };
};

/** Specialized selector for dynamically-allocated vectors. */
template<class Allocator>
struct allocated<Allocator, -1, -1, vector_storage_tag>
{
  typedef allocated<>					selector_type;
  typedef allocated<Allocator>				unbound_type;
  typedef allocated<Allocator>				proxy_type;
  typedef vector_storage_tag				storage_tag;
  typedef dynamic_size_tag				size_tag;
  typedef allocated_memory_tag				memory_tag;

  /** Unspecified array size. */
  static const int array_size = -1;

  /** Make a partially bound selector with size @c N. */
  template<int N> struct resize {
    typedef allocated<Allocator, N>			type;
  };
};

/** Specialized selector for dynamically-allocated, fixed-size vectors.
 *
 * @todo Fixed-size allocated types are not actually implemented by CML, so
 * the proxy_type is set to compiled<Size>.
 */
template<int Size, class Allocator>
struct allocated<Allocator, Size, -1, vector_storage_tag>
{
  typedef allocated<>					selector_type;
  typedef allocated<Allocator>				unbound_type;
  //XXX typedef allocated<Allocator, Size>		proxy_type;
  typedef compiled<Size>				proxy_type;
  typedef vector_storage_tag				storage_tag;
  typedef fixed_size_tag				size_tag;
  typedef allocated_memory_tag				memory_tag;

  /** Constant for the array size. */
  static const int array_size = Size;

  /** Make a partially bound selector with size @c N. */
  template<int N> struct resize {
    typedef allocated<Allocator, N>			type;
  };
};

/** Specialized selector for dynamically-allocated matrices. */
template<class Allocator>
struct allocated<Allocator, -1, -1, matrix_storage_tag>
{
  typedef allocated<>					selector_type;
  typedef allocated<Allocator>				unbound_type;
  typedef allocated<Allocator>				proxy_type;
  typedef matrix_storage_tag				storage_tag;
  typedef dynamic_size_tag				size_tag;
  typedef allocated_memory_tag				memory_tag;

  /** Unspecified array rows. */
  static const int array_rows = -1;

  /** Unspecified array columns. */
  static const int array_cols = -1;

  /** Make a partially bound selector with size @c R x @c C. */
  template<int R, int C> struct reshape {
    typedef allocated<Allocator, R, C>			type;
  };
};

/** Specialized selector for dynamically-allocated, fixed-size matrices.
 *
 * @todo Fixed-size allocated types are not actually implemented by CML, so
 * the proxy_type is set to compiled<Size1,Size2>.
 */
template<class Allocator, int Size1, int Size2>
struct allocated<Allocator, Size1, Size2, matrix_storage_tag>
{
  typedef allocated<>					selector_type;
  typedef allocated<Allocator>				unbound_type;
  //XXX typedef allocated<Allocator, Size1, Size2>	proxy_type;
  typedef compiled<Size1, Size2>			proxy_type;
  typedef matrix_storage_tag				storage_tag;
  typedef fixed_size_tag				size_tag;
  typedef allocated_memory_tag				memory_tag;

  /** Constant for the number of array rows. */
  static const int array_rows = Size1;

  /** Constant for the number of array columns. */
  static const int array_cols = Size2;

  /** Make a partially bound selector with size @c R x @c C. */
  template<int R, int C> struct resize {
    typedef allocated<Allocator, R, C>			type;
  };
};

/** Specialized selector for dynamically-allocated quaternions.
 *
 * @todo Fixed-size allocated types are not actually implemented by CML, so
 * the proxy_type is set to compiled<Size>.
 */
template<class Allocator>
struct allocated<Allocator, 4, -1, quaternion_storage_tag>
{
  typedef allocated<>					selector_type;
  typedef allocated<Allocator>				unbound_type;
  //XXX typedef allocated<Allocator,4>			proxy_type;
  typedef compiled<4>					proxy_type;
  typedef quaternion_storage_tag			storage_tag;
  typedef fixed_size_tag				size_tag;
  typedef allocated_memory_tag				memory_tag;

  /** Constant for the array size. */
  static const int array_size = 4;

  /** Make a partially bound selector with size @c N. */
  template<int N> struct resize {
    static_assert(N == 4, "invalid quaternion storage size");
    typedef allocated<Allocator, 4>			type;
  };
};

/** is_storage_selector for allocated<>. */
template<class Allocator, int Size1, int Size2, class Tag>
struct is_storage_selector<allocated<Allocator, Size1, Size2, Tag>> {
  static const bool value = true;
};

/** Determine an unbound allocated<> storage type using an allocator that
 * best represents the combination of the passed-in allocators.  If the
 * best allocator cannot be determined, std::allocator<> is used.
 *
 * @note This can be specialized for user-defined allocators if the default
 * disambiguation strategy fails to yield the proper type.
 */
template<
  class Allocator1, int R1, int C1, class Tag1,
  class Allocator2, int R2, int C2, class Tag2>
struct storage_disambiguate<
  allocated<Allocator1, R1, C1, Tag1>,
  allocated<Allocator2, R2, C2, Tag2>>
{
  /* Rebind the allocators to void* to compare them: */
  typedef std::allocator_traits<Allocator1>                     left_traits;
  typedef std::allocator_traits<Allocator2>                     right_traits;
  typedef typename left_traits::template rebind_alloc<void*>    left_type;
  typedef typename right_traits::template rebind_alloc<void*>   right_type;

  /* True if the unbound allocators are the same: */
  static const bool is_same
    = std::is_same<left_type, right_type>::value;

  /* True if the left and/or right allocators are the default: */
  static const bool left_is_default
    = std::is_same<left_type, std::allocator<void*>>::value;
  static const bool right_is_default
    = std::is_same<right_type, std::allocator<void*>>::value;

  /* Prefer the left allocator if it is not the default allocator: */
  static const bool prefer_left
    = !left_is_default && right_is_default;

  /* Prefer the right allocator if it is not the default allocator: */
  static const bool prefer_right
    = !right_is_default && left_is_default;

  /* Use std::allocator<> if the best cannot be determined: */
  static const bool prefer_default
    = !(is_same || prefer_left || prefer_right);
  static_assert(is_same || prefer_left || prefer_right || prefer_default,
    "unexpected storage_disambiguate result");

  /* Determine the preferred allocator type: */
  typedef
    cml::if_t<is_same,		left_type,
    cml::if_t<prefer_left,	left_type,
    cml::if_t<prefer_right,	right_type,
    /*else*/			std::allocator<void*>
      >>>						allocator_type;

  /* Build the disambiguated unbound storage type: */
  typedef allocated<allocator_type>			type;
};


/** For compatibility with CML1. */
template<class Allocator = std::allocator<void*>>
  using dynamic = allocated<Allocator>;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
