#pragma once
#include <stddef.h>
#include <cstdint>

namespace RavEngine{

typedef size_t ctti_t;

//compile-time hashing adapted from https://mikejsavage.co.uk/blog/cpp-tricks-compile-time-string-hashing.html
inline constexpr uint32_t Hash32_CT( const char * str, size_t n, uint32_t basis = uint_least32_t( 2166136261 ) ) {
        return n == 0 ? basis : Hash32_CT( str + 1, n - 1, ( basis ^ str[ 0 ] ) * uint_least32_t( 16777619 ) );
}
inline constexpr uint64_t Hash64_CT( const char * str, size_t n, uint64_t basis = uint_least64_t( 14695981039346656037U ) ) {
        return n == 0 ? basis : Hash64_CT( str + 1, n - 1, ( basis ^ str[ 0 ] ) * uint_least64_t( 1099511628211 ) );
}
template< size_t N >
inline constexpr uint32_t Hash32_CT( const char ( &s )[ N ] ) {
        return Hash32_CT( s, N - 1 );
}
template< size_t N >
inline constexpr uint64_t Hash64_CT( const char ( &s )[ N ] ) {
        return Hash64_CT( s, N - 1 );
}

template<typename T>
inline static constexpr ctti_t CTTI(){
    return Hash32_CT(__PRETTY_FUNCTION__);
}

static_assert(CTTI<float>() != CTTI<int>());

}
