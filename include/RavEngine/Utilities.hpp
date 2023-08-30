#pragma once
#include <effolkronium/random.hpp>
#include <fmt/format.h>
#include <sstream>
#include <type_traits>
#if !RVE_SERVER
#include <RGL/Types.hpp>
#endif

namespace RavEngine{

/**
 Get the underlying type. Useful for enum class.
 */
template <typename E>
inline constexpr auto to_underlying(E e) noexcept
{
	return static_cast<std::underlying_type_t<E>>(e);
}

/**
 Use for binding objects that have a stable location in memory to an InputManager
 */
template<typename T>
class PointerInputBinder{
	T* ptr;
public:
	PointerInputBinder(decltype(ptr) ptr) : ptr(ptr){}
	
	size_t get_id() const{
		return reinterpret_cast<size_t>(ptr);
	}
	T* get() const{
		return ptr;
	}
};


template<typename T, typename ... A>
static inline std::string StrFormat(const T& formatstr, A&& ... args) {
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

/**
Load a shader given its filename (ie "myshader.vsh"). Must include extension.
@param name filename of the shader
@return RGLShaderLibrary
*/
#if !RVE_SERVER
RGLShaderLibraryPtr LoadShaderByFilename(const std::string& name, RGLDevicePtr device);
#endif

}
