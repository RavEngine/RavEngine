#pragma once
#include <effolkronium/random.hpp>
#include <fmt/format.h>

namespace RavEngine{

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
