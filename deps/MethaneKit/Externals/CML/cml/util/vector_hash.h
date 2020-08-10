/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef cml_util_vector_hash_h
#define cml_util_vector_hash_h

#include <cml/common/hash.h>
#include <cml/vector/vector.h>
#include <cml/vector/traits.h>

/* Need specialization in std: */
namespace std {
template<class Element, class StorageType>
struct hash<cml::vector<Element,StorageType>>
{
  typedef cml::vector<Element,StorageType> vector_type;
  inline std::size_t operator()(const vector_type& v) const {
    std::size_t seed = 0;
    std::hash<cml::value_type_of_t<vector_type>> hasher;
    for(int i = 0; i < v.size(); ++ i)
      cml::detail::hash_combine(seed, hasher(v.get(i)));
    return seed;
  }
};
}

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
