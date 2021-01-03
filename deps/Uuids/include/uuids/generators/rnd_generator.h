#ifndef UUIDS_RND_GENERATOR_HPP
#define UUIDS_RND_GENERATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "generator_base.h"
#include <cstdint>
#include <chrono>
#include <random>

namespace uuids
{
    struct rnd_generator : public generator_base
    {
    public:
        rnd_generator()
        {
            _rnd_engine.seed(std::chrono::system_clock::now().time_since_epoch().count());
        };
        ~rnd_generator() {}

        uint16_t get_version()
        {
            return 0x4000;
        }

        uint64_t generate_timestamp()
        {
            uint64_t n = 0;
            for (uint8_t i = 0; i < 8; ++i)
            {
                uint8_t rnd = _rnd_uint8_distribution(_rnd_engine);
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
                uint8_t rnd = _rnd_uint8_distribution(_rnd_engine);
                n |= (static_cast<uint16_t>(rnd) << (i * 8)); // cast avoids overflow
            }
            return n;
        }

        uint8_t *get_node()
        {
            static uint8_t node[6];
            for (uint8_t i = 0; i < 6; ++i)
                node[i] = _rnd_uint8_distribution(_rnd_engine);
            return node;
        }

    private:
        inline static std::default_random_engine _rnd_engine = std::default_random_engine();
        inline static std::uniform_int_distribution<uint8_t> _rnd_uint8_distribution = std::uniform_int_distribution<uint8_t>(0, UINT8_MAX);
    };
} // namespace uuids

#endif // UUIDS_RND_GENERATOR_HPP
