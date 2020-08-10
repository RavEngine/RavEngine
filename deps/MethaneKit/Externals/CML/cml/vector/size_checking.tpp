/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_VECTOR_SIZE_CHECKING_TPP
#error "vector/size_checking.tpp not included correctly"
#endif

#include <cml/common/array_size_of.h>
#include <cml/common/promotion.h>
#include <cml/vector/readable_vector.h>

namespace cml {
namespace detail {

/* No-op binary vector expression size checking: */
template<class Sub1, class Sub2> inline void
check_same_size(const readable_vector<Sub1>&, const Sub2&, any_size_tag) {}

/* Compile-time binary vector expression size checking: */
template<class Sub1, class Sub2> inline void
check_same_size(const readable_vector<Sub1>&, const Sub2&, fixed_size_tag)
{
  static_assert(array_size_of_c<Sub1>::value == array_size_of_c<Sub2>::value,
    "incompatible vector expression sizes");
}

/* Run-time binary vector expression size checking: */
template<class Sub1, class Sub2> inline void
check_same_size(
  const readable_vector<Sub1>& left, const Sub2& right, dynamic_size_tag
  )
{
#ifndef CML_NO_RUNTIME_VECTOR_SIZE_CHECKS
  cml_require(array_size_of(left) == array_size_of(right),
    incompatible_vector_size_error, /**/);
#endif
}



/* No-op minimum vector size checking. */
template<class Sub> inline void
check_minimum_size(
  const readable_vector<Sub>&, int, enable_if_any_size_t<Sub>* = nullptr
  )
{
}

/* Compile-time minimum size checking against a constant. */
template<class Sub, int N> inline void
check_minimum_size(const readable_vector<Sub>&,
  cml::int_c<N>, enable_if_fixed_size_t<Sub>* = nullptr
  )
{
  static_assert(
    array_size_of_c<Sub>::value >= N, "vector expression too short");
}

/* Run-time minimum vector size checking. */
template<class Sub> inline void
check_minimum_size(const readable_vector<Sub>& sub, int N)
{
#ifndef CML_NO_RUNTIME_VECTOR_SIZE_CHECKS
  cml_require(array_size_of(sub) >= N, minimum_vector_size_error, /**/);
#endif
}



/* No-op vector size checking. */
template<class Sub> inline void
check_size(
  const readable_vector<Sub>&, int, enable_if_any_size_t<Sub>* = nullptr
  )
{
}

/* Compile-time size checking against a constant. */
template<class Sub, int N> inline void
check_size(const readable_vector<Sub>&,
  cml::int_c<N>, enable_if_fixed_size_t<Sub>* = nullptr
  )
{
  static_assert(
    array_size_of_c<Sub>::value == N, "incorrect vector expression size");
}

/* Run-time vector size checking. */
template<class Sub> inline void
check_size(const readable_vector<Sub>& sub, int N)
{
#ifndef CML_NO_RUNTIME_VECTOR_SIZE_CHECKS
  cml_require(array_size_of(sub) == N, vector_size_error, /**/);
#endif
}



/* No-op vector size range checking. */
template<class Sub> inline void
check_size_range(
  const readable_vector<Sub>&, int, int, enable_if_any_size_t<Sub>* = nullptr
  )
{
}

/* Compile-time size range checking against constants. */
template<class Sub, int Low, int High> inline void
check_size_range(const readable_vector<Sub>&,
  cml::int_c<Low>, cml::int_c<High>,
  enable_if_fixed_size_t<Sub>* = nullptr
  )
{
  static_assert(
    array_size_of_c<Sub>::value >= Low && array_size_of_c<Sub>::value <= High,
    "vector expression size out of range");
}

/* Run-time vector size checking. */
template<class Sub> inline void
check_size_range(const readable_vector<Sub>& sub, int Low, int High)
{
#ifndef CML_NO_RUNTIME_VECTOR_SIZE_CHECKS
  cml_require(
    array_size_of(sub) >= Low && array_size_of(sub) <= High,
    vector_size_range_error, /**/);
#endif
}

} // namespace detail


/* check_same_size: */

template<class Sub1, class Sub2> inline void
check_same_size(
  const readable_vector<Sub1>& left, const readable_vector<Sub2>& right
  )
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef size_tag_of_t<Sub2> tag2;
  detail::check_same_size(
    left, right.actual(), size_check_promote_t<tag1,tag2>());
}

template<class Sub1, class Sub2> inline void
check_same_size(
  const readable_vector<Sub1>& left, const Sub2& right,
  enable_if_array_t<Sub2>*
  )
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef tag1 tag2;			// dynamic/dynamic or fixed/fixed.
  detail::check_same_size(left, right, size_check_promote_t<tag1,tag2>());
}

template<class Sub1, class Sub2> auto
check_same_size(const readable_vector<Sub1>& left, const Sub2& right)
-> decltype(right.size(), void())
{
  typedef size_tag_of_t<Sub1> tag1;
  typedef dynamic_size_tag tag2;	// dynamic/dynamic or fixed/dynamic.
  detail::check_same_size(left, right, size_check_promote_t<tag1,tag2>());
}


/* check_minimum_size: */

template<class Sub> inline void
check_minimum_size(const readable_vector<Sub>& left, int N)
{
  detail::check_minimum_size(left, N);
}

template<class Sub, int N> inline void
check_minimum_size(const readable_vector<Sub>& left, cml::int_c<N>)
{
  detail::check_minimum_size(left, cml::int_c<N>());
}


/* check_size: */

template<class Sub> inline void
check_size(const readable_vector<Sub>& left, int N)
{
  detail::check_size(left, N);
}

template<class Sub, int N> inline void
check_size(const readable_vector<Sub>& left, cml::int_c<N>)
{
  detail::check_size(left, cml::int_c<N>());
}


/* check_size_range: */

template<class Sub> inline void
check_size_range(const readable_vector<Sub>& left, int Low, int High)
{
  detail::check_size_range(left, Low, High);
}

template<class Sub, int Low, int High> inline void
check_size_range(
  const readable_vector<Sub>& left, cml::int_c<Low>, cml::int_c<High>
  )
{
  detail::check_size_range(left, cml::int_c<Low>(), cml::int_c<High>());
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
