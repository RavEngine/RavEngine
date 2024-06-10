#pragma once
#include <string_view>

#if __has_include(<format>)
#include <format>
#define fmt_src std
#else
#include <fmt/format.h>
#define fmt_src fmt
#endif

namespace RavEngine{

template< class... Args >
constexpr auto Format(fmt_src::format_string<Args...> formatstr, Args&& ... args){
    return fmt_src::format(formatstr, std::forward<Args>(args)...);
}

template<typename ... T>
auto VFormat(const std::string_view formatstr, T&& ... args){
    return fmt_src::vformat(formatstr, fmt_src::make_format_args(args...));
}

}
