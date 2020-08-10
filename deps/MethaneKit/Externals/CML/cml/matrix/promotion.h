/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_promotion_h
#define	cml_matrix_promotion_h

#include <cml/common/promotion.h>
#include <cml/storage/resize.h>
#include <cml/storage/promotion.h>
#include <cml/scalar/promotion.h>
#include <cml/vector/vector.h>
#include <cml/vector/traits.h>
#include <cml/vector/type_util.h>
#include <cml/matrix/type_util.h>
#include <cml/matrix/traits.h>
#include <cml/matrix/matrix.h>

namespace cml {

/** Determine an appropriate storage type to use when combining matrix
 * expressions via a pairwise binary operator.
 *
 * @note This can be specialized to accomodate user-defined storage types.
 */
template<class Storage1, class Storage2>
struct matrix_binary_storage_promote
{
  static_assert(
    is_matrix_storage<Storage1>::value &&
    is_matrix_storage<Storage2>::value,
    "expected matrix storage types for binary promotion");

  /* Determine the common unbound storage type: */
  typedef storage_promote_t<Storage1, Storage2>		unbound_type;

  /* Determine the new row size: */
  static const int array_rows
    = Storage1::array_rows > Storage2::array_rows
    ?  Storage1::array_rows : Storage2::array_rows;

  /* Determine the new column size: */
  static const int array_cols
    = Storage1::array_cols > Storage2::array_cols
    ?  Storage1::array_cols : Storage2::array_cols;

  /* Resize: */
  typedef reshape_storage_t<
    unbound_type, array_rows, array_cols>		resized_type;

  /* Rebind to a matrix storage type: */
  typedef rebind_matrix_storage_t<resized_type>		type;
};

/** Convenience alias for matrix_binary_storage_promote. */
template<class Storage1, class Storage2>
  using matrix_binary_storage_promote_t =
    typename matrix_binary_storage_promote<Storage1,Storage2>::type;


/** Specializable class to determine a temporary type that can store the
 * result of matrix/matrix or matrix/vector products.
 */
template<class Sub1, class Sub2, class Enable = void>
  struct matrix_inner_product_promote;

/** Determine a matrix temporary type that can hold the result of
 * multiplying two matrix expressions @c Sub1 and @c Sub2.
 */
template<class Sub1, class Sub2>
struct matrix_inner_product_promote<Sub1, Sub2,
  typename std::enable_if<
    is_matrix<Sub1>::value && is_matrix<Sub2>::value>::type
    >
{
  typedef cml::unqualified_type_t<Sub1>			left_type;
  typedef cml::unqualified_type_t<Sub2>			right_type;
  typedef matrix_traits<left_type>			left_traits;
  typedef matrix_traits<right_type>			right_traits;
  typedef value_type_of_t<left_traits>			left_value_type;
  typedef value_type_of_t<right_traits>			right_value_type;
  typedef storage_type_of_t<left_traits>		left_storage_type;
  typedef storage_type_of_t<right_traits>		right_storage_type;

  /* Deduce the element type: */
  typedef value_type_trait_promote_t<
    left_value_type, right_value_type>			value_type;

  /* Determine the common unbound storage type from the bound proxy types,
   * preferring a dynamic-size type unless the resulting matrix can be
   * fixed-size:
   */
  typedef rebind_matrix_storage_t<
    proxy_type_of_t<left_storage_type>>			left_proxy_type;
  typedef rebind_matrix_storage_t<
    proxy_type_of_t<right_storage_type>>		right_proxy_type;
  static const int left_rows = left_proxy_type::array_rows;
  static const int right_cols = right_proxy_type::array_cols;
  static const bool is_fixed = left_rows > 0 && right_cols > 0;
  typedef storage_promote_t<
    left_proxy_type, right_proxy_type, !is_fixed>	unbound_type;

  /* Determine the new matrix size: */
  static const int array_rows = is_fixed ? left_rows : -1;
  static const int array_cols = is_fixed ? right_cols : -1;

  /* Determine the unbound proxy type: */
  typedef reshape_storage_t<
    unbound_type, array_rows, array_cols>		resized_type;
  typedef proxy_type_of_t<
    rebind_matrix_storage_t<resized_type>>		proxy_type;

  /* Determine the common basis type: */
  typedef basis_tag_promote_t<
    basis_tag_of_t<left_traits>,
    basis_tag_of_t<right_traits>>			basis_tag;

  /* Determine the common layout type: */
  typedef layout_tag_promote_t<
    layout_tag_of_t<left_traits>,
    layout_tag_of_t<right_traits>>			layout_tag;

  /* Build the temporary: */
  typedef matrix<value_type,
	  proxy_type, basis_tag, layout_tag>		type;
};

/** Determine a matrix temporary type that can hold the result of
 * multiplying two matrix expressions @c Sub1 and @c Sub2.
 */
template<class Sub1, class Sub2>
struct matrix_inner_product_promote<Sub1, Sub2,
  typename std::enable_if<
     (is_vector<Sub1>::value && is_matrix<Sub2>::value)
  || (is_matrix<Sub1>::value && is_vector<Sub2>::value)>::type
  >
{
  typedef cml::unqualified_type_t<Sub1>			left_type;
  typedef cml::unqualified_type_t<Sub2>			right_type;

  /* Figure out if pre- or post-multiplying: */
  static const bool left_is_vector = is_vector<left_type>::value;
  static const bool right_is_vector = is_vector<right_type>::value;
  static_assert(left_is_vector ^ right_is_vector, "unexpected expression");

  /* Vector traits and types: */
  typedef cml::if_t<
    left_is_vector, left_type, right_type>		vector_type;
  typedef traits_of_t<vector_type>			vector_traits;
  typedef value_type_of_t<vector_traits>		vector_value_type;
  typedef storage_type_of_t<vector_traits>		vector_storage_type;

  /* Matrix traits and types: */
  typedef cml::if_t<
    right_is_vector, left_type, right_type>		matrix_type;
  typedef traits_of_t<matrix_type>			matrix_traits;
  typedef value_type_of_t<matrix_traits>		matrix_value_type;
  typedef storage_type_of_t<matrix_traits>		matrix_storage_type;

  /* Deduce the element type: */
  typedef value_type_trait_promote_t<
    vector_value_type, matrix_value_type>		value_type;

  /* The resulting vector size comes from the bound matrix proxy rows or
   * columns, depending on pre- or post-multiplication:
   */
  typedef proxy_type_of_t<matrix_storage_type>		matrix_unbound_type;
  typedef rebind_matrix_storage_t<matrix_unbound_type>	matrix_proxy_type;
  static const int array_size = left_is_vector
    ? matrix_proxy_type::array_cols : matrix_proxy_type::array_rows;

  /* Determine the unbound and bound vector proxy types by resizing the
   * original vector storage type:
   */
  typedef rebind_vector_storage_t<
    resize_storage_t<vector_storage_type, array_size>>	vector_resized_type;
  typedef proxy_type_of_t<vector_resized_type>		vector_unbound_type;
  typedef rebind_vector_storage_t<vector_unbound_type>	vector_proxy_type;
  /* Note: two rebinds are needed here, first to get the unbound proxy
   * type, then to get the bound proxy type for sizing.
   */

  /* Determine the common unbound storage type from the bound proxy types,
   * preferring a dynamic-size type unless the resulting vector can be
   * fixed-size:
   */
  static const bool is_fixed = array_size > 0;
  typedef storage_promote_t<
    vector_proxy_type, matrix_proxy_type, !is_fixed>	unbound_type;
  typedef resize_storage_t<unbound_type, array_size>	resized_type;

  /* Prefer the unbound vector proxy type when possible: */
  typedef cml::if_t<
    /**/ (is_fixed && is_fixed_size<vector_proxy_type>::value)
    ||   (!is_fixed && is_dynamic_size<vector_proxy_type>::value)
    , vector_unbound_type, resized_type>		proxy_type;

  /* Build the temporary: */
  typedef vector<value_type, proxy_type>		type;
};

/** Convenience alias for matrix_inner_product_promote. */
template<class Sub1, class Sub2> using matrix_inner_product_promote_t
  = typename matrix_inner_product_promote<Sub1,Sub2>::type;


template<class Storage1, class Storage2>
struct matrix_outer_product_storage_promote
{
  static_assert(
    is_vector_storage<Storage1>::value &&
    is_vector_storage<Storage2>::value,
    "expected vector storage types for outer product promotion");

  /* Deduce the left matrix storage type from the vector storage: */
  static const int left_size = Storage1::array_size;
  typedef reshape_storage_t<typename Storage1::unbound_type,
	  left_size, -1>				left_unbound_type;
  typedef rebind_matrix_storage_t<left_unbound_type>	left_storage_type;

  /* Deduce the right matrix storage type from the vector storage: */
  static const int right_size = Storage2::array_size;
  typedef reshape_storage_t<typename Storage2::unbound_type,
	  -1, right_size>				right_unbound_type;
  typedef rebind_matrix_storage_t<right_unbound_type>	right_storage_type;

  /* Determine the common storage type, based on the storage types of its
   * subexpressions:
   */
  typedef matrix_binary_storage_promote_t<
    left_storage_type, right_storage_type>		type;
};

/** Convenience alias for matrix_outer_product_promote. */
template<class Storage1, class Storage2>
using matrix_outer_product_storage_promote_t
  = typename matrix_outer_product_storage_promote<Storage1,Storage2>::type;


/** Specializable class to determine a temporary type that can store the
 * result of vector outer products.
 */
template<class Sub1, class Sub2, class Basis, class Layout,
  class Enable = void> struct matrix_outer_product_promote;

/** Determine a matrix temporary type that can hold the result of the outer
 * product of two vector expressions @c Sub1 and @c Sub2.
 *
 * @note The temporary will
 */
template<class Sub1, class Sub2, class Basis, class Layout>
struct matrix_outer_product_promote<Sub1, Sub2, Basis, Layout,
  typename std::enable_if<
     (is_vector<Sub1>::value && is_vector<Sub2>::value)>::type
  >
{
  typedef cml::unqualified_type_t<Sub1>			left_type;
  typedef cml::unqualified_type_t<Sub2>			right_type;
  typedef vector_traits<left_type>			left_traits;
  typedef vector_traits<right_type>			right_traits;

  /* Deduce the element type: */
  typedef value_type_promote_t<left_traits,right_traits> value_type;

  /* Determine the common storage type for the temporary, based on the
   * storage types of its subexpressions:
   */
  typedef storage_type_of_t<left_traits>		left_storage_type;
  typedef storage_type_of_t<right_traits>		right_storage_type;
  typedef matrix_outer_product_storage_promote_t<
    left_storage_type, right_storage_type>		storage_type;
  typedef proxy_type_of_t<storage_type>			proxy_type;

  /* Build the temporary: */
  typedef matrix<value_type, proxy_type, Basis, Layout>	type;
};

/** Convenience alias for matrix_outer_product_promote. */
template<class Sub1, class Sub2, class Basis, class Layout>
  using matrix_outer_product_promote_t
    = typename matrix_outer_product_promote<Sub1,Sub2,Basis,Layout>::type;


/** Determine the row vector temporary type for matrix type @c Sub. */
template<class Sub, class Enable = void> struct row_type_of;

template<class Sub>
struct row_type_of<Sub,
  typename std::enable_if<cml::is_matrix<Sub>::value>::type>
{
  /* Matrix traits and types: */
  using matrix_type = cml::unqualified_type_t<Sub>;
  using matrix_traits = cml::traits_of_t<matrix_type>;
  using matrix_storage_type = cml::storage_type_of_t<matrix_traits>;
  using value_type = cml::value_type_of_t<matrix_traits>;

  /* The vector proxy type and size comes from the matrix proxy type: */
  using unbound_type = cml::proxy_type_of_t<matrix_storage_type>;
  using proxy_type = cml::rebind_matrix_storage_t<unbound_type>;
  static const int array_size = proxy_type::array_cols;
  using vector_type = cml::resize_storage_t<unbound_type, array_size>;

  /* Build the temporary: */
  using type = cml::vector<value_type, vector_type>;
};

/** Convenience alias for row_type_of. */
template<class Sub> using row_type_of_t = typename row_type_of<Sub>::type;


/** Determine the column vector temporary type for matrix type @c Sub. */
template<class Sub, class Enable = void> struct col_type_of;

template<class Sub>
struct col_type_of<Sub,
  typename std::enable_if<cml::is_matrix<Sub>::value>::type>
{
  /* Matrix traits and types: */
  using matrix_type = cml::unqualified_type_t<Sub>;
  using matrix_traits = cml::traits_of_t<matrix_type>;
  using matrix_storage_type = cml::storage_type_of_t<matrix_traits>;
  using value_type = cml::value_type_of_t<matrix_traits>;

  /* The vector proxy type and size comes from the matrix proxy type: */
  using unbound_type = cml::proxy_type_of_t<matrix_storage_type>;
  using proxy_type = cml::rebind_matrix_storage_t<unbound_type>;
  static const int array_size = proxy_type::array_rows;
  using vector_type = cml::resize_storage_t<unbound_type, array_size>;

  /* Build the temporary: */
  using type = cml::vector<value_type, vector_type>;
};

/** Convenience alias for col_type_of. */
template<class Sub> using col_type_of_t = typename col_type_of<Sub>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
