#ifndef UUIDS_SIMPLE_RND_GENERATOR_HPP
#define UUIDS_SIMPLE_RND_GENERATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "generator_base.h"
#include <ctime>
#include <cstdlib>

namespace uuids
{
    struct simple_rnd_generator : public generator_base
    {
    public:
        simple_rnd_generator()
        {
            srand(static_cast<unsigned int>(std::time(NULL)));
        };
        ~simple_rnd_generator() {}

        uint16_t get_version()
        {
            return 0x4000;
        }

        uint64_t generate_timestamp()
        {
            uint64_t n = 0;
            for (uint8_t i = 0; i < 8; ++i)
            {
                uint8_t rnd = static_cast<uint8_t>(rand() % UINT8_MAX);
                n |= (static_cast<uint64_t>(rnd) << (i * 8)); // cast avoids overflow
            }
            return n;
        }

        uint8_t get_variant()
        {
            return 0x80;
        }

        uint16_t generate_clock_sequence()
        {
            uint16_t n = 0;
            for (uint8_t i = 0; i < 2; ++i)
            {
                uint8_t rnd = static_cast<uint8_t>(rand() % UINT8_MAX);
                n |= (static_cast<uint16_t>(rnd) << (i * 8)); // cast avoids overflow
            }
            return n;
        }

        uint8_t *get_node()
        {
            static uint8_t node[6];
            for (uint8_t i = 0; i < 6; ++i)
                node[i] = static_cast<uint8_t>(rand() % UINT8_MAX);
            return node;
        }
    };
} // namespace uuids

#endif // UUIDS_SIMPLE_RND_GENERATOR_HPP
