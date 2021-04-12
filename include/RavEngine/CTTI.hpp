#pragma once
#include <stddef.h>
#include <cstdint>
#include <string_view>

#ifdef _WIN32
    #define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

namespace RavEngine{

// Derive from this struct to get a free CTTI specialization
struct AutoCTTI{};

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

inline constexpr uint32_t Hash32_CT(const std::string_view& v) {
    return Hash32_CT(v.data(), v.size() - 1);
}

inline constexpr uint64_t Hash64_CT(const std::string_view& v) {
    return Hash64_CT(v.data(), v.size() - 1);
}

// Type name extraction 

template <typename T>
constexpr std::string_view type_name_impl();

template <>
constexpr std::string_view type_name_impl<void>() {
    return "void";
}

namespace detail {

    using type_name_prober = void;

    template <typename T>
    inline constexpr std::string_view wrapped_type_name()
    {
        return __PRETTY_FUNCTION__;
    }

    inline constexpr std::size_t wrapped_type_name_prefix_length() {
        return wrapped_type_name<type_name_prober>().find(type_name_impl<type_name_prober>());
    }

    inline constexpr std::size_t wrapped_type_name_suffix_length() {
        return wrapped_type_name<type_name_prober>().length()
            - wrapped_type_name_prefix_length()
            - type_name_impl<type_name_prober>().length();
    }
}

/**
* Derive the type name string
*/
template <typename T>
inline constexpr std::string_view type_name_impl() {
    constexpr auto wrapped_name = detail::wrapped_type_name<T>();
    constexpr auto prefix_length = detail::wrapped_type_name_prefix_length();
    constexpr auto suffix_length = detail::wrapped_type_name_suffix_length();
    constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
    return wrapped_name.substr(prefix_length, type_name_length);
}

//already defiend for void, don't want to define it again
//defines automatically only for primitive types
template<typename T, std::enable_if_t<std::is_fundamental<T>::value && !std::is_same<T,void>::value, bool> = false>
inline constexpr std::string_view type_name() {
    return type_name_impl<T>();
}

// for structs, provide an automatic CTTI implementation using the derivation
template <typename T, std::enable_if_t<std::is_base_of<RavEngine::AutoCTTI,T>::value,bool> = false>
inline constexpr std::string_view type_name() {
#ifdef _MSC_VER
	constexpr auto str = type_name_impl<T>();
    short offset = 0;
    static_assert(str[0] == 'c' || str[0] == 's', "Type must be a class or struct");
	if constexpr (str[0] == 'c'){
		offset = std::strlen("class "); //advance past 'class'
	}
	else if (str[0] == 's'){
		offset = std::strlen("struct "); //advance past 'struct'
	}
    return std::string_view(str.data() + offset, str.size() - offset);
#else
	return type_name_impl<T>();
#endif
}

/**
@return a hashcode for a type
@note type_name<T>() must be specialized for non-fundamental types
*/
template<typename T>
inline static constexpr ctti_t CTTI(){
    return Hash32_CT(type_name<T>());
}

}
