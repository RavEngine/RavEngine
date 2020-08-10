/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_common_promotion_h
#define	cml_common_promotion_h

#include <cml/common/mpl/if_t.h>
#include <cml/common/mpl/is_same_pair.h>
#include <cml/common/size_tags.h>
#include <cml/common/basis_tags.h>
#include <cml/common/layout_tags.h>

namespace cml {

/** Deduce the size tag needed to compare the size of two expressions
 * having size tags @c Tag1 and @c Tag2.  By default:
 *
 * - any_size_tag with any other tag: any_size_tag
 * - fixed_size_tag with fixed_size_tag: fixed_size_tag
 * - otherwise: dynamic_size_tag
 */
template<class Tag1, class Tag2> struct size_check_promote
{
  static_assert(is_size_tag<Tag1>::value, "invalid size tag");
  static_assert(is_size_tag<Tag2>::value, "invalid size tag");

  /* Promote to any_size_tag when combined with any other tag: */
  static const bool is_any
    = std::is_same<Tag1, any_size_tag>::value
    || std::is_same<Tag2, any_size_tag>::value;

  /* Promote to fixed_size_tag when combining two fixed-size expressions: */
  static const bool is_fixed
    = std::is_same<Tag1,fixed_size_tag>::value
    && std::is_same<Tag2,fixed_size_tag>::value;

  /* Promote to dynamic_size_tag by default if not promoting to
   * any_size_tag or fixed_size_tag:
   */
  typedef
    cml::if_t< is_any, any_size_tag,
    cml::if_t< is_fixed, fixed_size_tag,
    /* else */ dynamic_size_tag>>			type;
};

/** Convenience alias for size_check_promote. */
template<class Tag1, class Tag2> using size_check_promote_t
= typename size_check_promote<Tag1,Tag2>::type;


/** Deduce the default size tag needed to promote the result of combining
 * two expressions having size tags @c Tag1 and @c Tag2.  By default:
 *
 * - fixed_size_tag with any other tag: fixed_size_tag
 * - dynamic_size_tag with any other tag: dynamic_size_tag
 * - any_size_tag with any_size_tag: any_size_tag
 */
template<class Tag1, class Tag2> struct size_tag_promote
{
  static_assert(is_size_tag<Tag1>::value, "invalid size tag");
  static_assert(is_size_tag<Tag2>::value, "invalid size tag");

  /* Fixed-size with any other tag promotes to fixed-size: */
  static const bool is_fixed
    = std::is_same<Tag1,fixed_size_tag>::value
    || std::is_same<Tag2,fixed_size_tag>::value;

  /* Promote to dynamic if not promoting to fixed, and if at least one of
   * the size tags is dynamic_size_tag:
   */
  static const bool is_dynamic = !is_fixed &&
    (std::is_same<Tag1,dynamic_size_tag>::value
     || std::is_same<Tag2,dynamic_size_tag>::value);

  /* Promote to any_size_tag when both are any_size_tag: */
  static const bool is_any
    = std::is_same<Tag1, any_size_tag>::value
    && std::is_same<Tag2, any_size_tag>::value;

  /* Has to be one of the deduced categories: */
  static_assert(is_fixed || is_dynamic || is_any, "unexpected size tag type");

  /* Promote to the selected tag: */
  typedef cml::if_t< is_fixed, fixed_size_tag,
	  cml::if_t< is_dynamic, dynamic_size_tag,
	  /* else */ any_size_tag>>			type;
};

/** Convenience alias for size_tag_promote. */
template<class Tag1, class Tag2>
  using size_tag_promote_t = typename size_tag_promote<Tag1,Tag2>::type;

/** Deduce the default size tag needed to promote the result of combining
 * two expressions @c T1 and @c T2 with traits that define the size_tag
 * type.
 */
template<class T1, class T2> struct size_tag_trait_promote {
  typedef size_tag_promote_t<
    size_tag_trait_of_t<T1>, size_tag_trait_of_t<T2>>	type;
};

/** Convenience alias for size_tag_trait_promote. */
template<class T1, class T2> using size_tag_trait_promote_t
  = typename size_tag_trait_promote<T1,T2>::type;


/** Deduce the default basis tag needed to promote the result of combining
 * two expressions having basis tags @c Tag1 and @c Tag2.  By default:
 *
 * - both row_basis, or row_basis with any_basis: row_basis
 * - both col_basis, or col_basis with any_basis: col_basis
 * - otherwise: any_basis
 */
template<class Tag1, class Tag2> struct basis_tag_promote
{
  static_assert(is_basis_tag<Tag1>::value, "invalid basis tag");
  static_assert(is_basis_tag<Tag2>::value, "invalid basis tag");

  /* True if possible to promote to row_basis: */
  static const bool is_row_basis
    =  is_same_pair<Tag1, Tag2, row_basis>::value
    || is_same_pair<Tag1, Tag2, row_basis, any_basis>::value;

  /* True if possible to promote to col_basis: */
  static const bool is_col_basis
    =  is_same_pair<Tag1, Tag2, col_basis>::value
    || is_same_pair<Tag1, Tag2, col_basis, any_basis>::value;

  /* At least one has to be false: */
  static_assert(!is_row_basis || !is_col_basis, "invalid basis promotion");

  /* Promote to the selected basis, or any_basis otherwise: */
  typedef
    cml::if_t< is_row_basis, row_basis,
    cml::if_t< is_col_basis, col_basis,
    /* else */ any_basis>>				type;
};

/** Convenience alias for basis_tag_promote. */
template<class Tag1, class Tag2>
  using basis_tag_promote_t = typename basis_tag_promote<Tag1,Tag2>::type;

/** Deduce the default basis tag needed to promote the result of combining
 * two expressions @c T1 and @c T2 with traits that define the basis_tag
 * type.
 */
template<class T1, class T2> struct basis_tag_trait_promote {
  typedef basis_tag_promote_t<
    basis_tag_trait_of_t<T1>, basis_tag_trait_of_t<T2>> type;
};

/** Convenience alias for basis_tag_trait_promote. */
template<class T1, class T2> using basis_tag_trait_promote_t
  = typename basis_tag_trait_promote<T1,T2>::type;


/** Deduce the default layout tag needed to promote the result of combining
 * two expressions having layout tags @c Tag1 and @c Tag2.  By default:
 *
 * - both row_major, or row_major with any_major: row_major
 * - both col_major, or col_major with any_major: col_major
 * - otherwise: any_major
 */
template<class Tag1, class Tag2> struct layout_tag_promote
{
  static_assert(is_layout_tag<Tag1>::value, "invalid layout tag");
  static_assert(is_layout_tag<Tag2>::value, "invalid layout tag");

  /* True if possible to promote to row_major: */
  static const bool is_row_major
    =  is_same_pair<Tag1, Tag2, row_major>::value
    || is_same_pair<Tag1, Tag2, row_major, any_major>::value;

  /* True if possible to promote to col_major: */
  static const bool is_col_major
    =  is_same_pair<Tag1, Tag2, col_major>::value
    || is_same_pair<Tag1, Tag2, col_major, any_major>::value;

  /* At least one has to be false: */
  static_assert(!is_row_major || !is_col_major, "invalid layout promotion");

  /* Promote to the selected layout, or any_major otherwise: */
  typedef
    cml::if_t< is_row_major, row_major,
    cml::if_t< is_col_major, col_major,
    /* else */ any_major>>				type;
};

/** Convenience alias for layout_tag_promote. */
template<class Tag1, class Tag2>
  using layout_tag_promote_t = typename layout_tag_promote<Tag1,Tag2>::type;

/** Deduce the default layout tag needed to promote the result of combining
 * two expressions @c T1 and @c T2 with traits that define the layout_tag
 * type.
 */
template<class T1, class T2> struct layout_tag_trait_promote {
  typedef layout_tag_promote_t<
    layout_tag_trait_of_t<T1>, layout_tag_trait_of_t<T2>> type;
};

/** Convenience alias for layout_tag_trait_promote. */
template<class T1, class T2> using layout_tag_trait_promote_t
  = typename layout_tag_trait_promote<T1,T2>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
