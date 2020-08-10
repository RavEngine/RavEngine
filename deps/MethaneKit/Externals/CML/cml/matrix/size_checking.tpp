/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_SIZE_CHECKING_TPP
#error "matrix/size_checking.tpp not included correctly"
#endif

#include <cml/common/promotion.h>
#include <cml/vector/readable_vector.h>
#include <cml/matrix/readable_matrix.h>
#include <cml/matrix/array_size_of.h>

namespace cml {
namespace detail {

/* For checking inner product sizes of vectors and/or matrices: */
template<class Sub, class Enable = void> struct inner_rows_of_c;
template<class Sub, class Enable = void> struct inner_cols_of_c;

/* Use the matrix row size for inner products: */
template<class Sub> struct inner_rows_of_c<Sub,
  typename std::enable_if<array_rows_of_c<Sub>::value != 0>::type>
{
  static const int value = array_rows_of_c<Sub>::value;
};

/* Use the matrix column size for inner products: */
template<class Sub> struct inner_cols_of_c<Sub,
  typename std::enable_if<array_cols_of_c<Sub>::value != 0>::type>
{
  static const int value = array_cols_of_c<Sub>::value;
};

/* The inner product row size for vectors is just the vector size: */
template<class Sub>
struct inner_rows_of_c<Sub,
  typename std::enable_if<array_size_of_c<Sub>::value != 0>::type>
{
  static const int value = array_size_of_c<Sub>::value;
};

/* The inner product column size for vectors is just the vector size: */
template<class Sub> struct inner_cols_of_c<Sub,
  typename std::enable_if<array_size_of_c<Sub>::value != 0>::type>
{
  static const int value = array_size_of_c<Sub>::value;
};

/* Run-time matrix row size for inner products: */
template<class Sub> inline
int inner_rows_of(const readable_matrix<Sub>& sub) { return sub.rows(); }

/* Run-time matrix column size for inner products: */
template<class Sub> inline
int inner_cols_of(const readable_matrix<Sub>& sub) { return sub.cols(); }

/* Run-time row size for vectors is the vector size: */
template<class Sub> inline
int inner_rows_of(const readable_vector<Sub>& sub) { return sub.size(); }

/* Run-time column size for vectors is the vector size: */
template<class Sub> inline
int inner_cols_of(const readable_vector<Sub>& sub) { return sub.size(); }


/* No-op binary matrix expression linear size checking: */
template<class Sub1, class Sub2> inline void check_same_linear_size(
  const readable_matrix<Sub1>&, const Sub2&, any_size_tag) {}

/* Compile-time binary matrix expression linear size checking: */
template<class Sub1, class Sub2> inline void
check_same_linear_size(
  const readable_matrix<Sub1>&, const Sub2&, fixed_size_tag
  )
{
  static_assert(
    array_rows_of_c<Sub1>::value*array_cols_of_c<Sub1>::value
    == array_size_of_c<Sub2>::value,
    "incompatible matrix expression sizes");
}

/* Run-time binary matrix expression linear size checking: */
template<class Sub1, class Sub2> inline void
check_same_linear_size(
  const readable_matrix<Sub1>& left, const Sub2& right, dynamic_size_tag
  )
{
#ifndef CML_NO_RUNTIME_MATRIX_SIZE_CHECKS
  cml_require(
    array_rows_of(left)*array_cols_of(left) == array_size_of(right),
    incompatible_matrix_size_error, /**/);
#endif
}


/* No-op matrix linear size checking. */
template<class Sub> inline void
check_linear_size(const readable_matrix<Sub>&, int, any_size_tag) {}

/* Compile-time matrix linear size checking. */
template<class Sub, int N> inline void
check_linear_size(const readable_matrix<Sub>&, cml::int_c<N>, fixed_size_tag)
{
  static_assert(array_rows_of_c<Sub>::value*array_cols_of_c<Sub>::value == N,
    "incorrect matrix expression size");
}

/* Run-time matrix linear size checking. */
template<class Sub, class SizeTag> inline void
check_linear_size(const readable_matrix<Sub>& sub, int N, SizeTag)
{
#ifndef CML_NO_RUNTIME_MATRIX_SIZE_CHECKS
  cml_require(
    array_rows_of(sub)*array_cols_of(sub) == N, matrix_size_error, /**/);
#endif
}


/* No-op binary matrix expression size checking: */
template<class Sub1, class Sub2> inline void
check_same_size(const readable_matrix<Sub1>&, const Sub2&, any_size_tag) {}

/* Compile-time binary matrix expression size checking: */
template<class Sub1, class Sub2> inline void
check_same_size(const readable_matrix<Sub1>&, const Sub2&, fixed_size_tag)
{
  static_assert(
    (array_rows_of_c<Sub1>::value == array_rows_of_c<Sub2>::value)
    && (array_cols_of_c<Sub1>::value == array_cols_of_c<Sub2>::value),
    "incompatible matrix expression sizes");
}

/* Run-time binary matrix expression size checking: */
template<class Sub1, class Sub2> inline void
check_same_size(
  const readable_matrix<Sub1>& left, const Sub2& right, dynamic_size_tag
  )
{
#ifndef CML_NO_RUNTIME_MATRIX_SIZE_CHECKS
  cml_require(array_size_of(left) == array_size_of(right),
    incompatible_matrix_size_error, /**/);
#endif
}

/* Compile-time binary matrix expression size checking against a C-array: */
template<class Sub, class Other, int R, int C> inline void
check_same_size(
  const readable_matrix<Sub>&, Other const (&)[R][C], fixed_size_tag
  )
{
  static_assert(
    (array_rows_of_c<Sub>::value == R) && (array_cols_of_c<Sub>::value == C),
    "incompatible matrix expression sizes");
}

/* Run-time binary matrix expression size checking against a C-array: */
template<class Sub, class Other, int R, int C> inline void
check_same_size(
  const readable_matrix<Sub>& left, Other const (&)[R][C], dynamic_size_tag
  )
{
#ifndef CML_NO_RUNTIME_MATRIX_SIZE_CHECKS
  cml_require((array_rows_of(left) == R && array_cols_of(left) == C),
    incompatible_matrix_size_error, /**/);
#endif
}


/* No-op binary matrix expression row size checking: */
template<class Sub1, class Sub2> inline void
check_same_row_size(const readable_matrix<Sub1>&, const Sub2&, any_size_tag) {}

/* Compile-time binary matrix expression row size checking against a
 * fixed-size readable_vector:
 */
template<class Sub1, class Sub2> inline void
check_same_row_size(
  const readable_matrix<Sub1>&, const readable_vector<Sub2>&, fixed_size_tag
  )
{
  static_assert(array_rows_of_c<Sub1>::value == array_size_of_c<Sub2>::value,
    "incompatible matrix row sizes");
}

/* Run-time binary matrix expression row size checking against a
 * dynamic-size readable_vector:
 */
template<class Sub1, class Sub2> inline void
check_same_row_size(const readable_matrix<Sub1>& left,
  const readable_vector<Sub2>& right, dynamic_size_tag
  )
{
#ifndef CML_NO_RUNTIME_MATRIX_SIZE_CHECKS
  cml_require(array_rows_of(left) == array_size_of(right),
    incompatible_matrix_row_size_error, /**/);
#endif
}


/* No-op binary matrix expression column size checking: */
template<class Sub1, class Sub2> inline void
check_same_col_size(const readable_matrix<Sub1>&, const Sub2&, any_size_tag) {}

/* Compile-time binary matrix expression column size checking against a
 * fixed-size readable_vector:
 */
template<class Sub1, class Sub2> inline void
check_same_col_size(
  const readable_matrix<Sub1>&, const readable_vector<Sub2>&, fixed_size_tag
  )
{
  static_assert(array_cols_of_c<Sub1>::value == array_size_of_c<Sub2>::value,
    "incompatible matrix row sizes");
}

/* Run-time binary matrix expression column size checking against a
 * dynamic-size readable_vector:
 */
template<class Sub1, class Sub2> inline void
check_same_col_size(const readable_matrix<Sub1>& left,
  const readable_vector<Sub2>& right, dynamic_size_tag
  )
{
#ifndef CML_NO_RUNTIME_MATRIX_SIZE_CHECKS
  cml_require(array_cols_of(left) == array_size_of(right),
    incompatible_matrix_col_size_error, /**/);
#endif
}


/* No-op matrix inner product size checking: */
template<class Sub1, class Sub2> inline void
check_same_inner_size(const Sub1&, const Sub2&, any_size_tag) {}

/* Compile-time matrix inner product size checking: */
template<class Sub1, class Sub2> inline void
check_same_inner_size(const Sub1&, const Sub2&, fixed_size_tag)
{
  typedef traits_of_t<Sub1> left_traits;
  typedef traits_of_t<Sub2> right_traits;
  static_assert(
    (inner_cols_of_c<left_traits>::value
     == inner_rows_of_c<right_traits>::value),
    "incompatible matrix inner product size");
}

/* Run-time matrix inner product size checking: */
template<class Sub1, class Sub2> inline void
check_same_inner_size(const Sub1& left, const Sub2& right, dynamic_size_tag)
{
#ifndef CML_NO_RUNTIME_MATRIX_SIZE_CHECKS
  cml_require(inner_cols_of(left) == inner_rows_of(right),
    incompatible_matrix_inner_size_error, /**/);
#endif
}


/* No-op matrix size checking. */
template<class Sub> inline void
check_size(const readable_matrix<Sub>&, int, int, any_size_tag) {}

/* Compile-time checking against constant row and column sizes. */
template<class Sub, int R, int C> inline void
check_size(
  const readable_matrix<Sub>&, cml::int_c<R>, cml::int_c<C>, fixed_size_tag
  )
{
  static_assert(
    (array_rows_of_c<Sub>::value == R) && (array_cols_of_c<Sub>::value == C),
    "incorrect matrix expression size");
}

/* Run-time matrix size checking. */
template<class Sub, class SizeTag> inline void
check_size(const readable_matrix<Sub>& sub, int R, int C, SizeTag)
{
#ifndef CML_NO_RUNTIME_MATRIX_SIZE_CHECKS
  cml_require(
    (array_rows_of(sub) == R) && (array_cols_of(sub) == C),
    matrix_size_error, /**/);
#endif
}


/* No-op minimum matrix size checking. */
template<class Sub> inline void
check_minimum_size(const readable_matrix<Sub>&, int, int, any_size_tag) {}

/* Compile-time minimum size checking against constant row and column
 * sizes.
 */
template<class Sub, int R, int C> inline void
check_minimum_size(
  const readable_matrix<Sub>&, cml::int_c<R>, cml::int_c<C>, fixed_size_tag
  )
{
  static_assert(
    (array_rows_of_c<Sub>::value >= R) && (array_cols_of_c<Sub>::value >= C),
    "matrix expression too small");
}

/* Run-time minimum matrix size checking. */
template<class Sub, class SizeTag> inline void
check_minimum_size(const readable_matrix<Sub>& sub, int R, int C, SizeTag)
{
#ifndef CML_NO_RUNTIME_MATRIX_SIZE_CHECKS
  cml_require(
    (array_rows_of(sub) >= R) && (array_cols_of(sub) >= C),
    minimum_matrix_size_error, /**/);
#endif
}


/* No-op square matrix checking. */
template<class Sub> inline void
check_square(const readable_matrix<Sub>&, any_size_tag) {}

/* Compile-time square matrix checking. */
template<class Sub> inline void
check_square(const readable_matrix<Sub>&, fixed_size_tag)
{
  static_assert(
    (array_rows_of_c<Sub>::value == array_cols_of_c<Sub>::value),
    "non-square matrix");
}

/* Run-time square matrix checking. */
template<class Sub, class SizeTag> inline void
check_square(const readable_matrix<Sub>& sub, SizeTag)
{
#ifndef CML_NO_RUNTIME_MATRIX_SIZE_CHECKS
  cml_require(
    (array_rows_of(sub) == array_cols_of(sub)),
    non_square_matrix_error, /**/);
#endif
}

} // namespace detail


/* check_same_linear_size: */

template<class Sub1, class Sub2> inline void
check_same_linear_size(
  const readable_matrix<Sub1>& left, const readable_matrix<Sub1>& right
  )
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef size_tag_of_t<Sub2> tag2;
  detail::check_same_linear_size(
    left, right.actual(), size_check_promote_t<tag1,tag2>());
}

template<class Sub1, class Sub2> inline void
check_same_linear_size(
  const readable_matrix<Sub1>& left, const Sub2& right,
  enable_if_array_t<Sub2>*
  )
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef tag1 tag2;			// dynamic/dynamic or fixed/fixed.
  detail::check_same_linear_size(
    left, right, size_check_promote_t<tag1,tag2>());
}

template<class Sub1, class Sub2> inline auto
check_same_linear_size(const readable_matrix<Sub1>& left, const Sub2& right)
-> decltype(right.size(), void())
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef dynamic_size_tag tag2;	// dynamic/dynamic or fixed/dynamic.
  detail::check_same_linear_size(
    left, right, size_check_promote_t<tag1,tag2>());
}


/* check_linear_size: */

template<class Sub> inline void
check_linear_size(const readable_matrix<Sub>& left, int n)
{
  typedef size_tag_of_t<Sub> tag;
  detail::check_linear_size(left, n, tag());
}

template<class Sub, int N> inline void
check_linear_size(const readable_matrix<Sub>& left, cml::int_c<N>)
{
  typedef size_tag_of_t<Sub> tag;
  detail::check_linear_size(left, cml::int_c<N>(), tag());
}


/* check_same_size: */

template<class Sub1, class Sub2> inline void
check_same_size(
  const readable_matrix<Sub1>& left, const readable_matrix<Sub2>& right
  )
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef size_tag_of_t<Sub2> tag2;
  detail::check_same_size(
    left, right.actual(), size_check_promote_t<tag1,tag2>());
}

template<class Sub, class Other, int R, int C> inline void
check_same_size(
  const readable_matrix<Sub>& left, Other const (&array)[R][C]
  )
{
  typedef size_tag_of_t<Sub> tag1;
  typedef fixed_size_tag tag2;
  detail::check_same_size(left, array, size_check_promote_t<tag1,tag2>());
}


/* check_same_row_size: */

template<class Sub1, class Sub2> inline void
check_same_row_size(
  const readable_matrix<Sub1>& left, const readable_vector<Sub2>& right
  )
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef size_tag_of_t<Sub2> tag2;
  detail::check_same_row_size(
    left, right.actual(), size_check_promote_t<tag1,tag2>());
}


/* check_same_col_size: */

template<class Sub1, class Sub2> inline void
check_same_col_size(
  const readable_matrix<Sub1>& left, const readable_vector<Sub2>& right
  )
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef size_tag_of_t<Sub2> tag2;
  detail::check_same_col_size(
    left, right.actual(), size_check_promote_t<tag1,tag2>());
}


/* check_same_inner_size: */

template<class Sub1, class Sub2> inline void
check_same_inner_size(
  const readable_matrix<Sub1>& left, const readable_matrix<Sub2>& right
  )
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef size_tag_of_t<Sub2> tag2;
  detail::check_same_inner_size(
    left.actual(), right.actual(), size_check_promote_t<tag1,tag2>());
}

template<class Sub1, class Sub2> inline void
check_same_inner_size(
  const readable_matrix<Sub1>& left, const readable_vector<Sub2>& right
  )
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef size_tag_of_t<Sub2> tag2;
  detail::check_same_inner_size(
    left.actual(), right.actual(), size_check_promote_t<tag1,tag2>());
}

template<class Sub1, class Sub2> inline void
check_same_inner_size(
  const readable_vector<Sub1>& left, const readable_matrix<Sub2>& right
  )
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef size_tag_of_t<Sub2> tag2;
  detail::check_same_inner_size(
    left.actual(), right.actual(), size_check_promote_t<tag1,tag2>());
}


/* check_size: */

template<class Sub> inline void
check_size(const readable_matrix<Sub>& left, int R, int C)
{
  typedef size_tag_of_t<Sub> tag;
  detail::check_size(left, R, C, tag());
}

template<class Sub, int R, int C> inline void
check_size(const readable_matrix<Sub>& left, cml::int_c<R>, cml::int_c<C>)
{
  typedef size_tag_of_t<Sub> tag;
  detail::check_size(left, cml::int_c<R>(), cml::int_c<C>(), tag());
}


/* check_minimum_size: */

template<class Sub> inline void
check_minimum_size(const readable_matrix<Sub>& left, int R, int C)
{
  typedef size_tag_of_t<Sub> tag;
  detail::check_minimum_size(left, R, C, tag());
}

template<class Sub, int R, int C> inline void
check_minimum_size(
  const readable_matrix<Sub>& left, cml::int_c<R>, cml::int_c<C>
  )
{
  typedef size_tag_of_t<Sub> tag;
  detail::check_minimum_size(left, cml::int_c<R>(), cml::int_c<C>(), tag());
}


/* check_square: */

template<class Sub> inline void
check_square(const readable_matrix<Sub>& left)
{
  typedef size_tag_of_t<Sub> tag;
  detail::check_square(left, tag());
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
