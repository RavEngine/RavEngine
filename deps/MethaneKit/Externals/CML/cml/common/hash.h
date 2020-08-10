/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef cml_common_hash_h
#define cml_common_hash_h

#include <cml/common/mpl/int_c.h>
#include <functional>
#include <cstdint>

namespace cml {
namespace detail {

/* Adapted mrom boost/functional/hash/hash.hpp> */
#if defined(_MSC_VER)
#define _HASH_ROTL32(x, r) _rotl(x,r)
#else
#define _HASH_ROTL32(x, r) ((x << r) | (x >> (32 - r)))
#endif

template<typename U32> inline void
hash_combine_impl(U32& h1, U32 k1, cml::int_c<4>)
{
  const auto c1 = UINT32_C(0xcc9e2d51);
  const auto c2 = UINT32_C(0x1b873593);

  k1 *= c1;
  k1 = _HASH_ROTL32(k1, 15);
  k1 *= c2;

  h1 ^= k1;
  h1 = _HASH_ROTL32(h1, 13);
  h1 = h1 * 5 + 0xe6546b64;
}

#undef _CML_HASH_ROTL32

template<typename U64> inline void
hash_combine_impl(U64& h, U64 k, cml::int_c<8>)
{
  const auto m = UINT64_C(0xc6a4a7935bd1e995);
  const auto r = 47;

  k *= m;
  k ^= k >> r;
  k *= m;

  h ^= k;
  h *= m;

  // Completely arbitrary number, to prevent 0's
  // from hashing to 0.
  h += UINT64_C(0xe6546b64);
}


template<typename UInt> inline void
hash_combine(UInt& h, UInt k)
{
  hash_combine_impl(h, k, cml::int_c<sizeof(UInt)>());
}

}
}

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
