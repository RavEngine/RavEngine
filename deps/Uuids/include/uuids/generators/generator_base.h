#ifndef UUIDS_GENERATOR_BASE_HPP
#define UUIDS_GENERATOR_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>

namespace uuids
{
    struct generator_base
    {
        virtual uint16_t get_version() = 0;
        virtual uint64_t generate_timestamp() = 0;
        virtual uint8_t get_variant() = 0;
        virtual uint16_t generate_clock_sequence() = 0;
        virtual uint8_t *get_node() = 0;
    };
} // namespace uuids

#endif // UUIDS_GENERATOR_BASE_HPP
