#pragma once
#include <string_view>
#include "Types.hpp"

#if __has_include(<format>)
#include <format>
#define fmt_src std
#else
#include <fmt/format.h>
#define fmt_src fmt
#endif

template <>
struct fmt_src::formatter<EntityHandle> {
    
    constexpr auto parse(fmt_src::format_parse_context& ctx) {
        return ctx.begin();
    }
    
    auto format(const EntityHandle& id, fmt_src::format_context& ctx) const {
        return fmt_src::format_to(ctx.out(), "{}", entity_id_t(id.id));
    }
};

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
