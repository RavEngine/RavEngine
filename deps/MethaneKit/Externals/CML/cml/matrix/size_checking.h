/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_matrix_size_checking_h
#define	cml_matrix_size_checking_h

#include <cml/common/mpl/int_c.h>
#include <cml/common/mpl/enable_if_array.h>
#include <cml/common/exception.h>
#include <cml/vector/fwd.h>
#include <cml/matrix/fwd.h>

namespace cml {

/** Exception thrown when run-time size checking is enabled, and the
 * operands of a matrix expression have incompatible sizes.
 */
struct incompatible_matrix_size_error : std::runtime_error {
  incompatible_matrix_size_error()
    : std::runtime_error("incompatible matrix expression sizes") {}
};

/** Exception thrown when run-time size checking is enabled, and the
 * operand of a matrix expression does not meet a minimum size.
 */
struct minimum_matrix_size_error : std::runtime_error {
  minimum_matrix_size_error()
    : std::runtime_error("matrix expression too small") {}
};

/** Exception thrown when run-time size checking is enabled, and the
 * operand of a matrix expression does not have the required size.
 */
struct matrix_size_error : std::runtime_error {
  matrix_size_error()
    : std::runtime_error("incorrect matrix expression size") {}
};

/** Exception thrown when run-time size checking is enabled, and the row
 * size of a matrix operand does not match the (row) size of a second
 * operand.
 */
struct incompatible_matrix_row_size_error : std::runtime_error {
  incompatible_matrix_row_size_error()
    : std::runtime_error("incompatible matrix row sizes") {}
};

/** Exception thrown when run-time size checking is enabled, and the column
 * size of a matrix operand does not match the (row) size of a second
 * operand.
 */
struct incompatible_matrix_col_size_error : std::runtime_error {
  incompatible_matrix_col_size_error()
    : std::runtime_error("incompatible matrix column sizes") {}
};

/** Exception thrown when run-time size checking is enabled, and the
 * column size of a matrix operand does not match the row size of a second
 * operand.
 */
struct incompatible_matrix_inner_size_error : std::runtime_error {
  incompatible_matrix_inner_size_error()
    : std::runtime_error("incompatible matrix inner product size") {}
};

/** Exception thrown when run-time size checking is enabled, and a matrix
 * operand is not square.
 */
struct non_square_matrix_error : std::runtime_error {
  non_square_matrix_error()
    : std::runtime_error("non-square matrix") {}
};



/** Front-end for both compile-time and run-time matrix binary expression
 * linear size checking.  Both expressions must derive from
 * readable_matrix.
 *
 * @tparam Sub1 the actual type of the first expression.
 * @tparam Sub2 the actual type of the second expression.
 *
 * @param left First matrix expression.
 * @param right Second matrix expression.
 *
 * @throws incompatible_matrix_size_error at run-time if either @c left or
 * @c right is a dynamically-sized expression, and @c left.rows() * @c
 * left.cols() != @c right.rows() * @c right.cols().  If @c left and @c
 * right are fixed-size expressions, then the sizes are checked at compile
 * time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub1, class Sub2> void check_same_linear_size(
  const readable_matrix<Sub1>& left, const readable_matrix<Sub2>& right);

/** Front-end for both compile-time and run-time matrix binary expression
 * size checking against a fixed-size array.  The first expression must
 * derive from readable_matrix, and std::is_array<>::value must be true for
 * the second expression (e.g. double v[16]).
 *
 * @tparam Sub1 the actual type of the first expression.
 * @tparam Sub2 the actual type of the second expression.
 *
 * @param left Matrix expression.
 * @param right Fixed-length array.
 *
 * @throws incompatible_matrix_size_error at run-time if @c left is a
 * dynamically-sized expression, and @c left.rows() * @c left.cols() !=
 * array_size_of(@c right).  If @c left is a fixed-size expression, then the
 * sizes are checked at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub1, class Sub2> void
check_same_linear_size(const readable_matrix<Sub1>& left, const Sub2& right,
  enable_if_array_t<Sub2>* = 0);

/** Front-end for run-time matrix binary expression length checking.  The
 * first expression must derive from readable_matrix, and the second must
 * implement a size() method for this overload to be enabled.
 *
 * @tparam Sub1 the actual type of the first expression.
 * @tparam Sub2 the actual type of the second expression.
 *
 * @param left First matrix expression.
 * @param right Second expression.
 *
 * @throws incompatible_matrix_size_error if @c left.rows() * @c
 * left.cols() != @c right.size().
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub1, class Sub2> auto
check_same_linear_size(const readable_matrix<Sub1>& left, const Sub2& right)
-> decltype(right.size(), void());


/** Front-end for matrix expression size checking against a run-time linear
 * size.  The expression must derive from readable_matrix.
 *
 * @param left Matrix expression.
 * @param N The linear size to check.
 *
 * @throws matrix_size_error if @c left.rows() * @c left.cols() != @c N.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub> void check_linear_size(
  const readable_matrix<Sub>& left, int N);

/** Front-end for compile-time and run-time matrix expression linear size
 * checking against an integer constant via int_c<N>.  The expression
 * must derive from readable_matrix.
 *
 * @param left Matrix expression.
 *
 * @throws matrix_size_error at run-time if @c left is a dynamically-sized
 * expression and @c left.rows() * @c left.cols() != @c N. If @c left is a
 * fixed-size expression, then the size is checked at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub, int N> void check_linear_size(
  const readable_matrix<Sub>& left, int_c<N>);

/** Front-end for both compile-time and run-time matrix binary expression
 * size checking.  Both expressions must derive from readable_matrix.
 *
 * @tparam Sub1 the actual type of the first expression.
 * @tparam Sub2 the actual type of the second expression.
 *
 * @param left First matrix expression.
 * @param right Second matrix expression.
 *
 * @throws incompatible_matrix_size_error at run-time if either @c left or
 * @c right is a dynamically-sized expression, and @c left.size() !=
 * @c right.size().  If both are fixed-size expressions, then the sizes are
 * checked at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub1, class Sub2> void check_same_size(
  const readable_matrix<Sub1>& left, const readable_matrix<Sub2>& right);

/** Front-end for both compile-time and run-time matrix expression size
 * checking against a 2D C-array.  @c left must derive from
 * readable_matrix.
 *
 * @tparam Sub the actual type of the first expression.
 * @tparam Other the element type of the array.
 * @tparam Rows the number of array rows.
 * @tparam Cols the number of array cols.
 *
 * @param left First matrix expression.
 * @param right The 2D C-array.
 *
 * @throws incompatible_matrix_size_error at run-time if @c left is a
 * dynamically-sized expression and does not have the same number of rows
 * and columns as the array.  If @c left is a fixed-size expression, then the
 * sizes are checked at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub, class Other, int Rows, int Cols> void check_same_size(
  const readable_matrix<Sub>& left, Other const (&array)[Rows][Cols]);

/** Front-end for both compile-time and run-time matrix row size
 * checking against a vector expression.  @c left must derive from
 * readable_matrix, and @c right must derived from readable_vector.
 *
 * @tparam Sub1 the actual type of the matrix expression.
 * @tparam Sub2 the actual type of the vector expression.
 *
 * @param left Matrix expression.
 * @param right Vector expression.
 *
 * @throws incompatible_matrix_row_size_error at run-time if @c left or @c
 * right is dynamically-sized, and @c left does not have the same number of
 * rows as @c right has elements.  If both are fixed-size expressions, then
 * the sizes are checked at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub1, class Sub2> void check_same_row_size(
  const readable_matrix<Sub1>& left, const readable_vector<Sub2>& right);

/** Front-end for both compile-time and run-time matrix column size
 * checking against a vector expression.  @c left must derive from
 * readable_matrix, and @c right must derived from readable_vector.
 *
 * @tparam Sub1 the actual type of the matrix expression.
 * @tparam Sub2 the actual type of the vector expression.
 *
 * @param left Matrix expression.
 * @param right Vector expression.
 *
 * @throws incompatible_matrix_col_size_error at run-time if @c left or @c
 * right is dynamically-sized, and @c left does not have the same number
 * of columns as @c right has elements.  If both are fixed-size expressions,
 * then the sizes are checked at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub1, class Sub2> void check_same_col_size(
  const readable_matrix<Sub1>& left, const readable_vector<Sub2>& right);


/** Front-end for both compile-time and run-time compatible inner product
 * size checking.  @c left and @c right must derive from readable_matrix.
 *
 * @tparam Sub1 the actual type of the first matrix expression.
 * @tparam Sub2 the actual type of the second matrix expression.
 *
 * @param left First matrix expression.
 * @param right Second matrix expression.
 *
 * @throws incompatible_matrix_inner_size_error at run-time if @c left or @c
 * right is dynamically-sized, and @c left does not have the same number
 * of columns as @c right has rows.  If both are fixed-size expressions,
 * then the sizes are checked at compile time.
 */
template<class Sub1, class Sub2> void check_same_inner_size(
  const readable_matrix<Sub1>& left, const readable_matrix<Sub2>& right);

/** Front-end for both compile-time and run-time compatible inner product
 * size checking.  @c left must derive from readable_matrix, and @c right
 * must derive from readable_vector.
 *
 * @tparam Sub1 the actual type of the matrix expression.
 * @tparam Sub2 the actual type of the vector expression.
 *
 * @param left Matrix expression.
 * @param right Vector expression.
 *
 * @throws incompatible_matrix_inner_size_error at run-time if @c left or @c
 * right is dynamically-sized, and @c left does not have the same number
 * of columns as @c right has elements.  If both are fixed-size expressions,
 * then the sizes are checked at compile time.
 */
template<class Sub1, class Sub2> void check_same_inner_size(
  const readable_matrix<Sub1>& left, const readable_vector<Sub2>& right);

/** Front-end for both compile-time and run-time compatible inner product
 * size checking.  @c left must derive from readable_vector, and @c right
 * must derive from readable_vector.
 *
 * @tparam Sub1 the actual type of the vector expression.
 * @tparam Sub2 the actual type of the matrix expression.
 *
 * @param left Vector expression.
 * @param right Matrix expression.
 *
 * @throws incompatible_matrix_inner_size_error at run-time if @c left or
 * @c right is dynamically-sized, and @c left does not have the same number
 * of elements as @c right has rows.  If both are fixed-size expressions,
 * then the sizes are checked at compile time.
 */
template<class Sub1, class Sub2> void check_same_inner_size(
  const readable_vector<Sub1>& left, const readable_matrix<Sub2>& right);


/** Front-end for matrix expression size checking against a run-time
 * size.  The expression must derive from readable_matrix.
 *
 * @param left Matrix expression.
 * @param R Row count to check.
 * @param C Column count to check.
 *
 * @throws matrix_size_error at run-time if @c left is dynamically-sized,
 * and if @c left.rows() != @c R or @c left.cols() != @c C.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub> void check_size(
  const readable_matrix<Sub>& left, int R, int C);

/** Front-end for compile-time and run-time matrix expression size checking
 * against integer constants via int_c<R> and int_c<C>.  The expression
 * must derive from readable_matrix.
 *
 * @param left Matrix expression.
 *
 * @throws matrix_size_error at run-time if @c left is dynamically-sized,
 * and if @c left.rows() != @c R or @c left.cols() != @c C.  If @c left is a
 * fixed-size expression, then the size is checked at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub, int R, int C> void check_size(
  const readable_matrix<Sub>& left, cml::int_c<R>, cml::int_c<C>);


/** Front-end for matrix expression minimum size checking against a
 * run-time size.  The expression must derive from readable_matrix.
 *
 * @param left Matrix expression.
 * @param R Minimum row count to check.
 * @param C Minimum column count to check.
 *
 * @throws minimum_matrix_size_error at run-time if @c left is
 * dynamically-sized, and if @c left.rows() < @c R or @c left.cols() < @c
 * C.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub> void check_minimum_size(
  const readable_matrix<Sub>& left, int R, int C);

/** Front-end for compile-time and run-time matrix expression minimum size
 * checking against integer constants via int_c<R> and int_c<C>.  The
 * expression must derive from readable_matrix.
 *
 * @param left Matrix expression.
 *
 * @throws minimum_matrix_size_error at run-time if @c left is
 * dynamically-sized, and if @c left.rows() < @c R or @c left.cols() <
 * @c C.  If @c left is a fixed-size expression, then the size is checked
 * at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub, int R, int C> void check_minimum_size(
  const readable_matrix<Sub>& left, cml::int_c<R>, cml::int_c<C>);


/** Front-end to check for a square matrix.
 *
 * @throws non_square_matrix_error at run-time if @c left is a
 * dynamically-sized expression and @c left.rows() != @c left.cols().  If
 * @c left is a fixed-size expression, then the size is checked at compile
 * time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_MATRIX_SIZE_CHECKS at compile time.
 */
template<class Sub> void check_square(const readable_matrix<Sub>& left);

} // namespace cml

#define __CML_MATRIX_SIZE_CHECKING_TPP
#include <cml/matrix/size_checking.tpp>
#undef __CML_MATRIX_SIZE_CHECKING_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
