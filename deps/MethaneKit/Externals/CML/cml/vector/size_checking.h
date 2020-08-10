/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_size_checking_h
#define	cml_vector_size_checking_h

#include <cml/common/mpl/int_c.h>
#include <cml/common/mpl/enable_if_array.h>
#include <cml/common/exception.h>
#include <cml/vector/fwd.h>

namespace cml {

/** Exception thrown when run-time size checking is enabled, and the
 * operands of a vector expression have incompatible sizes.
 */
struct incompatible_vector_size_error : std::runtime_error {
  incompatible_vector_size_error()
    : std::runtime_error("incompatible vector expression sizes") {}
};

/** Exception thrown when run-time size checking is enabled, and the
 * operand of a vector expression does not meet a minimum size.
 */
struct minimum_vector_size_error : std::runtime_error {
  minimum_vector_size_error()
    : std::runtime_error("vector expression too short") {}
};

/** Exception thrown when run-time size checking is enabled, and the
 * operand of a vector expression does not have the required size.
 */
struct vector_size_error : std::runtime_error {
  vector_size_error()
    : std::runtime_error("incorrect vector expression size") {}
};

/** Exception thrown when run-time size checking is enabled, and the
 * size of a vector expression operand is outside the required range.
 */
struct vector_size_range_error : std::runtime_error {
  vector_size_range_error()
    : std::runtime_error("vector expression size out of range") {}
};


/** Front-end for both compile-time and run-time vector binary expression
 * length checking.  Both expressions must derive from readable_vector.
 *
 * @tparam Sub1 the actual type of the first expression.
 * @tparam Sub2 the actual type of the second expression.
 *
 * @param left First vector expression.
 * @param right Second vector expression.
 *
 * @throws incompatible_vector_size_error at run-time if either @c left or
 * @c right is a dynamically-sized expression, and @c left.size() !=
 * @c right.size().  If both are fixed-size expressions, then the sizes are
 * checked at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_VECTOR_SIZE_CHECKS at compile time.
 */
template<class Sub1, class Sub2> void check_same_size(
  const readable_vector<Sub1>& left, const readable_vector<Sub2>& right);

/** Front-end for both compile-time and run-time vector binary expression
 * length checking against a fixed-length array.  The first expression must
 * derive from readable_vector, and std::is_array<>::value must be true for
 * the second expression (e.g. double v[3]).
 *
 * @tparam Sub1 the actual type of the first expression.
 * @tparam Sub2 the actual type of the second expression.
 *
 * @param left Vector expression.
 * @param right Fixed-length array.
 *
 * @throws incompatible_vector_size_error at run-time if @c left is a
 * dynamically-sized expression, and @c left.size() != array_size_of(@c right).
 * If @c left is a fixed-size expression, then the sizes are checked at
 * compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_VECTOR_SIZE_CHECKS at compile time.
 */
template<class Sub1, class Sub2> void
check_same_size(const readable_vector<Sub1>& left, const Sub2& right,
  enable_if_array_t<Sub2>* = 0);

/** Front-end for run-time vector binary expression length checking.  The
 * first expression must derive from readable_vector, and the second must
 * implement a size() method for this overload to be enabled.
 *
 * @tparam Sub1 the actual type of the first expression.
 * @tparam Sub2 the actual type of the second expression.
 *
 * @param left First vector expression.
 * @param right Second expression.
 *
 * @throws incompatible_vector_size_error if @c left.size() != @c right.size().
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_VECTOR_SIZE_CHECKS at compile time.
 */
template<class Sub1, class Sub2>
auto check_same_size(const readable_vector<Sub1>& left, const Sub2& right)
-> decltype(right.size(), void());


/** Front-end for minimum vector expression length checking against a
 * run-time size.  The expression must derive from readable_vector.
 *
 * @param left Vector expression.
 * @param N Size to check.
 *
 * @throws minimum_vector_size_error if @c left.size() < @c N.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_VECTOR_SIZE_CHECKS at compile time.
 */
template<class Sub>
void check_minimum_size(const readable_vector<Sub>& left, int N);

/** Front-end for compile-time and run-time minimum vector expression
 * length checking against an integer constant via cml::int_c<N>.  The
 * expression must derive from readable_vector.
 *
 * @param left Vector expression.
 *
 * @throws minimum_vector_size_error at run-time if @c left is a
 * dynamically-sized expression and @c left.size() < @c N. If @c left is a
 * fixed-size expression, then the size is checked at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_VECTOR_SIZE_CHECKS at compile time.
 */
template<class Sub, int N>
void check_minimum_size(const readable_vector<Sub>& left, cml::int_c<N>);


/** Front-end for vector expression length checking against a run-time
 * size.  The expression must derive from readable_vector.
 *
 * @param left Vector expression.
 * @param N Size to check.
 *
 * @throws vector_size_error if @c left.size() != @c N.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_VECTOR_SIZE_CHECKS at compile time.
 */
template<class Sub>
void check_size(const readable_vector<Sub>& left, int N);

/** Front-end for compile-time and run-time vector expression length
 * checking against an integer constant via int_c<N>.  The expression must
 * derive from readable_vector.
 *
 * @param left Vector expression.
 *
 * @throws vector_size_error at run-time if @c left is a dynamically-sized
 * expression and @c left.size() != N. If @c left is a fixed-size expression,
 * then the size is checked at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_VECTOR_SIZE_CHECKS at compile time.
 */
template<class Sub, int N>
void check_size(const readable_vector<Sub>& left, cml::int_c<N>);


/** Front-end for vector expression length checking against a run-time
 * inclusive size range.  The expression must derive from readable_vector.
 *
 * @param left Vector expression.
 * @param Low Minimum size to check.
 * @param High Maximum size to check.
 *
 * @throws vector_size_range_error if @c left.size() < @c Low or @c
 * left.size() > @c High.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_VECTOR_SIZE_CHECKS at compile time.
 */
template<class Sub>
void check_size_range(const readable_vector<Sub>& left, int Low, int High);

/** Front-end for compile-time and run-time vector expression length
 * checking against an integer constant inclusive range via int_c<N>.  The
 * expression must derive from readable_vector.
 *
 * @param left Vector expression.
 *
 * @throws vector_size_range_error at run-time if @c left is a
 * dynamically-sized expression and @c left.size() < @c Low or @c
 * left.size() > @c High. If @c left is a fixed-size expression, then the
 * size is checked at compile time.
 *
 * @note Run-time checking can be disabled by defining
 * CML_NO_RUNTIME_VECTOR_SIZE_CHECKS at compile time.
 */
template<class Sub, int Low, int High>
void check_size_range(
  const readable_vector<Sub>& left, cml::int_c<Low>, cml::int_c<High>);

} // namespace cml

#define __CML_VECTOR_SIZE_CHECKING_TPP
#include <cml/vector/size_checking.tpp>
#undef __CML_VECTOR_SIZE_CHECKING_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
