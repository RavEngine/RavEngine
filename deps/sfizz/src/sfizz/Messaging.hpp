// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "Messaging.h"
#include <type_traits>
#include <cstddef>
#include <cstring>

namespace sfz {

///
inline void Client::receive(int delay, const char* path, const char* sig, const sfizz_arg_t* args)
{
    if (receive_)
        receive_(data_, delay, path, sig, args);
}

template <char... Sig>
inline void Client::receive(int delay, const char* path, OscDecayedType<Sig>... values)
{
    if (receive_) {
        constexpr size_t size = sizeof...(Sig);
        char sig[size + 1] { Sig..., '\0' };
        sfizz_arg_t args[size] { OscDataTraits<Sig>::make_arg(values)... };
        receive_(data_, delay, path, sig, args);
    }
}

///
#define OSC_SCALAR_TRAITS(tag, member)                          \
    template <> struct OscDataTraits<tag> {                     \
        typedef decltype(sfizz_arg_t::member) type;             \
        typedef type decayed_type;                              \
        static_assert(std::is_scalar<type>::value, "");         \
        static inline sfizz_arg_t make_arg(decayed_type v) {    \
            sfizz_arg_t a; a.member = v; return a;              \
        }                                                       \
    }
#define OSC_BYTEARRAY_TRAITS(tag, member)                               \
    template <> struct OscDataTraits<tag> {                             \
        typedef decltype(sfizz_arg_t::member) type;                     \
        static_assert(std::is_array<type>::value, "");                  \
        typedef const typename std::remove_all_extents<type>::type* decayed_type; \
        static inline sfizz_arg_t make_arg(decayed_type v) {            \
            sfizz_arg_t a;                                              \
            std::memcpy(a.member, v, sizeof(a.member));                 \
            return a;                                                   \
        }                                                               \
    }
#define OSC_VOID_TRAITS(tag)                                    \
    template <> struct OscDataTraits<tag> {                     \
        typedef struct Nothing {} type;                         \
        typedef type decayed_type;                              \
        static inline sfizz_arg_t make_arg(decayed_type v) {    \
            sfizz_arg_t a {}; (void)v; return a;                   \
        }                                                       \
    }

OSC_SCALAR_TRAITS('i', i);
OSC_SCALAR_TRAITS('c', i);
OSC_SCALAR_TRAITS('r', i);
OSC_BYTEARRAY_TRAITS('m', m);
OSC_SCALAR_TRAITS('h', h);
OSC_SCALAR_TRAITS('f', f);
OSC_SCALAR_TRAITS('d', d);
OSC_SCALAR_TRAITS('s', s);
OSC_SCALAR_TRAITS('S', s);
OSC_SCALAR_TRAITS('b', b);
OSC_VOID_TRAITS('T');
OSC_VOID_TRAITS('F');
OSC_VOID_TRAITS('N');
OSC_VOID_TRAITS('I');

#undef OSC_SCALAR_TRAITS
#undef OSC_BYTEARRAY_TRAITS
#undef OSC_VOID_TRAITS

} // namespace sfz
