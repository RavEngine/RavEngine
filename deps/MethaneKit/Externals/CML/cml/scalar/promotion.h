/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_scalar_promotion_h
#define	cml_scalar_promotion_h

#include <cml/common/type_util.h>
#include <cml/scalar/traits.h>

namespace cml {

/** Use C++ type deduction via std::common_type to determine the result of
 * combining N scalars.
 */
template<class... Scalars> using
  scalar_promote = std::common_type<Scalars...>;

/** Use C++ type deduction via std::common_type to determine the result of
 * combining N scalars.
 */
template<class... Scalars> using scalar_promote_t
  = typename scalar_promote<Scalars...>::type;

/** Helper to simplify scalar promotion from objects with a value_type
 * typedef.
 */
template<class... Subs> struct value_type_promote {
  typedef typename scalar_promote<value_type_of_t<Subs>...>::type type;
};
// XXX This could be a template alias, except VC++12 can't grok it.
// Moreover, without value_type_of_t<Subs>, even this fails to compile...

/** Convenience alias for value_type_trait_promote<>::type. */
template<class... Subs> using value_type_promote_t
  = typename value_type_promote<Subs...>::type;

/** Helper to simplify scalar promotion from objects that implement a
 * traits class with a value_type typedef.
 */
template<class... Subs> struct value_type_trait_promote {
  typedef typename scalar_promote<value_type_trait_of_t<Subs>...>::type type;
};
// XXX This could be a template alias, except VC++12 can't grok it.
// Moreover, without value_type_trait_of_t<Subs>, even this fails to
// compile...

/** Convenience alias for value_type_trait_promote<>::type. */
template<class... Subs> using value_type_trait_promote_t
  = typename value_type_trait_promote<Subs...>::type;

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
