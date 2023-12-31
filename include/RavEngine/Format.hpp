#pragma once
#include <string_view>
#if 1
    #include <fmt/format.h>
    #define fmt_src fmt
#else
    #include <format>
    #define fmt_src std
#endif

namespace RavEngine{

template< class... Args >
constexpr auto Format(fmt_src::format_string<Args...> formatstr, Args&& ... args){
    return fmt_src::format(formatstr, args...);
}

template<typename ... T>
auto VFormat(const std::string_view formatstr, const T& ... args){
    return fmt_src::vformat(formatstr, fmt_src::make_format_args(args...));
}

}
