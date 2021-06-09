#pragma once
#include <effolkronium/random.hpp>
#include <fmt/format.h>
#include <sstream>
#include <type_traits>

namespace RavEngine{

/**
 Get the underlying type. Useful for enum class.
 */
template <typename E>
constexpr auto to_underlying(E e) noexcept
{
	return static_cast<std::underlying_type_t<E>>(e);
}


template<typename T, typename ... A>
static inline std::string StrFormat(const T& formatstr, A ... args) {
	return fmt::format(formatstr, args...);
}
/**
* Format a number with system locale separator
* @param value the number to format
* @return formatted number (eg 1000 -> 1,000 if in USA)
* @note https://stackoverflow.com/questions/7276826/c-format-number-with-commas
*/
template<class T>
static inline std::string FormatWithSep(T value)
{
	std::stringstream ss;
	ss.imbue(std::locale(""));
	ss << std::fixed << value;
	return ss.str();
}

//shortcut for defining static symbols
#define STATIC(T) decltype(T) T
	
//random generation
using Random = effolkronium::random_static;

}
