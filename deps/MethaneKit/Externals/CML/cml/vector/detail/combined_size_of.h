/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_detail_combined_size_of_h
#define	cml_vector_detail_combined_size_of_h

#include <cml/common/mpl/plus_c.h>

namespace cml {
namespace detail {

template<class Sub> inline
enable_if_fixed_size_t<Sub, cml::int_c<array_size_of_c<Sub>::value>>
combined_size_of(const readable_vector<Sub>&)
{
  return array_size_of_c<Sub>::value;
}

template<class Sub, class... Elements> inline
enable_if_fixed_size_t<
  Sub, cml::int_c<
    cml::plus_c<array_size_of_c<Sub>::value,int(sizeof...(Elements))>::value
    >
  >
combined_size_of(const readable_vector<Sub>&, const Elements&...)
{
  return cml::int_c<array_size_of_c<Sub>::value + int(sizeof...(Elements))>();
}

template<class Sub> inline
enable_if_dynamic_size_t<Sub, int>
combined_size_of(const readable_vector<Sub>& sub)
{
  return sub.size();
}

template<class Sub, class... Elements> inline
enable_if_dynamic_size_t<Sub, int>
combined_size_of(const readable_vector<Sub>& sub, const Elements&...)
{
  return sub.size() + int(sizeof...(Elements));
}

} // namespace detail
} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
