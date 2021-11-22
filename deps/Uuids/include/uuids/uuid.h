#pragma once
#ifndef UUIDS_UUID_HPP
#define UUIDS_UUID_HPP

#if defined(RND_UUIDS)
#include "generators/rnd_generator.h"
#elif defined(TIME_BASED_UUIDS)
#include "generators/time_based_generator.h"
#else
#include "generators/simple_rnd_generator.h"
#endif

#include <string>
#include <sstream>
#include <cstring>
#include <array>

/* --------------------------------------------------------------------------
Uuid layout:
x x x x x x x x - x x x x -M x x x -N x x x - x x x x x x x x x x x x (hexas)
0   1   2   3     4   5    6   7    8   9     10  11  12  13  14  15  (bytes)

The 4 bits of M (6-th byte) define the uuid version.
The first 3 msbs of N (8-th byte) define the uuid variant.
Bytes are given specific group, listed below:
    time_low                   0-3
    time_mid                   4-5
    time_hi_and_version        6-7
    clock_seq_hi_and_reserved  8
    clock_seq_low              9
    node                       10-15
-------------------------------------------------------------------------- */

namespace uuids
{
    struct uuid final
    {
    public:
        virtual ~uuid() {}
        static uuid empty() { return uuid(); }
        static uuid create()
        {
            uuid d;
            d._generate(_default_generator);
            return d;
        }
        static uuid create(generator_base &generator)
        {
            uuid d;
            d._generate(generator);
            return d;
        }
		
		uuid(const char bytes[16]){
			uint8_t offset = 0;
			std::memcpy(&_time_low,bytes+offset,sizeof(_time_low));
			offset += sizeof(_time_low);
			
			std::memcpy(&_time_mid,bytes+offset,sizeof(_time_mid));
			offset += sizeof(_time_mid);
			
			std::memcpy(&_time_hi_and_version,bytes+offset,sizeof(_time_hi_and_version));
			offset += sizeof(_time_hi_and_version);
			
			std::memcpy(&_clock_seq_hi_and_reserved,bytes+offset,sizeof(_clock_seq_hi_and_reserved));
			offset += sizeof(_clock_seq_hi_and_reserved);
			
			std::memcpy(&_clock_seq_low,bytes+offset,sizeof(_clock_seq_low));
			offset += sizeof(_clock_seq_low);
			
			std::memcpy(&_node,bytes+offset,sizeof(_node));
		}
		
		uuid(){}

    public:
        uuid clone() { return uuid(*this); }
        std::string to_string() const { return _to_string(); }
		
		inline bool operator==( const uuid &rhs) const { return compare(*this, rhs) == 0; }
		inline bool operator!=( const uuid &rhs) const { return compare(*this, rhs) != 0; }
		inline bool operator<(const uuid &rhs) const { return compare(*this, rhs) < 0; }
		inline bool operator>(const uuid &rhs) const { return compare(*this, rhs) > 0; }
		inline bool operator<=(const uuid &rhs) const { return compare(*this, rhs) <= 0; }
		inline bool operator>=(const uuid &rhs) const { return compare(*this, rhs) >= 0; }
        
        inline uuid& operator=(const uuid& other){
            if (&other != this){
                _time_low = other._time_low;
                _time_mid = other._time_mid;
                _time_hi_and_version = other._time_hi_and_version;
                _clock_seq_hi_and_reserved = other._clock_seq_hi_and_reserved;
                _clock_seq_low = other._clock_seq_low;
                std::memcpy(_node, other._node, sizeof(_node[0]) * 6);
            }
            return *this;
        }

#define CHECK(f1, f2) \
    if (f1 != f2)     \
        return f1 < f2 ? -1 : 1;
        // compare two UUID's "lexically" and return
        // -1   u1 is lexically before u2
        //  0   u1 is equal to u2
        //  1   u1 is lexically after u2
        // Note that lexical ordering is not temporal ordering!
        static inline int compare(const uuid &u1, const uuid &u2)
        {
            CHECK(u1._time_low, u2._time_low);
            CHECK(u1._time_mid, u2._time_mid);
            CHECK(u1._time_hi_and_version, u2._time_hi_and_version);
            CHECK(u1._clock_seq_hi_and_reserved, u2._clock_seq_hi_and_reserved);
            CHECK(u1._clock_seq_low, u2._clock_seq_low);
            for (int i = 0; i < 6; ++i)
                CHECK(u1._node[i], u2._node[i]);
            return 0;
        }
#undef CHECK

    private:
        inline void _generate(generator_base &gntr)
        {
            // set node before clock, because node might reset the node
            _set_node(gntr);
            _set_time_and_version(gntr);
            _set_clock_and_variant(gntr);
        }

		inline void _set_time_and_version(generator_base &gntr)
        {
            // generate timestamp (64 bits, first 4 msbs to be discarded) and version
            uint64_t stamp = gntr.generate_timestamp();
            uint16_t version = gntr.get_version();

            // set time_low as stamp's bits 0-31
            _time_low = static_cast<uint32_t>(stamp);

            // set time_mid as stamp's bits 32-47
            _time_mid = static_cast<uint16_t>(stamp >> 32);

            // set time_hi_and_version bits 0-11 as stamp's bits 48-59
            _time_hi_and_version = (static_cast<uint16_t>(stamp >> 48) & ~version_mask);

            // set time_hi_and_version bits 12-15 as version
            _time_hi_and_version += (version & version_mask);
        }

		inline void _set_clock_and_variant(generator_base &gntr)
        {
            // generate clock sequence (16 bits, first 2 msbs to be discarded)
            uint16_t clock = gntr.generate_clock_sequence();
            uint8_t variant = gntr.get_variant();

            // set clock_seq_low as clock's bits 0-7
            _clock_seq_low = static_cast<uint8_t>(clock);

            // set clock_seq_hi_and_reserved bits 0-5 as clock's bits 8-13
            _clock_seq_hi_and_reserved = (static_cast<uint8_t>(clock >> 8) & ~variant_mask);

            // set clock_seq_hi_and_reserved bits 14-15 as variant
            _clock_seq_hi_and_reserved += (variant & variant_mask);
        }

		inline void _set_node(generator_base &gntr)
        {
            // get node (48-bit MAC address if available; otherwise random)
            uint8_t *node = gntr.get_node();

            // copy each node
            for (int i = 0; i < 6; ++i)
                _node[i] = node[i];
        }

    private:
        uint32_t _time_low{};                 // 0-3
        uint16_t _time_mid{};                 // 4-5
        uint16_t _time_hi_and_version{};      // 6-7
        uint8_t _clock_seq_hi_and_reserved{}; // 8
        uint8_t _clock_seq_low{};             // 9
        uint8_t _node[6];                     // 10-15

        inline static const constexpr uint16_t version_mask = 0xf000; // 1111 0000 0000 0000
        inline static const constexpr uint8_t variant_mask = 0xc0;    // 1100 0000

#if defined(RND_UUIDS)
        inline static rnd_generator _default_generator = rnd_generator();
#elif defined(TIME_BASED_UUIDS)
        inline static time_based_generator _default_generator = time_based_generator();
#else
        inline static simple_rnd_generator _default_generator = simple_rnd_generator();
#endif

    private:
		inline uuid &_copy_bytes(const uuid &other)
        {
            // no move constructor or assignment since fields are of very simple types
            _time_low = other._time_low;
            _time_mid = other._time_mid;
            _time_hi_and_version = other._time_hi_and_version;
            _clock_seq_hi_and_reserved = other._clock_seq_hi_and_reserved;
            _clock_seq_low = other._clock_seq_low;
            for (int i = 0; i < 6; ++i)
                _node[i] = other._node[i];
            return *this;
        }

		inline std::string _to_string() const
        {
			constexpr uint16_t size = 36 + 1;
            char buffer[size];
            snprintf(buffer, size, "%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x",
                    _time_low,
                    _time_mid,
                    _time_hi_and_version,
                    _clock_seq_hi_and_reserved,
                    _clock_seq_low,
                    _node[0],
                    _node[1],
                    _node[2],
                    _node[3],
                    _node[4],
                    _node[5]);

            return std::string(buffer);
        }
    public:
		inline std::array<char,16> raw() const {
            std::array<char, 16> data;
            char* buffer = data.data();
            memset(buffer,0,16);
            uint8_t offset = 0;
           
            memcpy(buffer, &_time_low, sizeof(_time_low));
            offset += sizeof(_time_low);

            memcpy(buffer + offset, &_time_mid, sizeof(_time_mid));
            offset += sizeof(_time_mid);

            memcpy(buffer + offset, &_time_hi_and_version, sizeof(_time_hi_and_version));
            offset += sizeof(_time_hi_and_version);

            memcpy(buffer + offset, &_clock_seq_hi_and_reserved, sizeof(_clock_seq_hi_and_reserved));
            offset += sizeof(_clock_seq_hi_and_reserved);

            memcpy(buffer + offset, &_clock_seq_low, sizeof(_clock_seq_low));
            offset += sizeof(_clock_seq_low);

            memcpy(buffer + offset, _node, sizeof(_node));
    
            return data;
        }
        inline std::bitset<128> raw_bitset() const{
            return std::bitset<128>(*(raw().data()));
        }
    };

} // namespace uuids

namespace std
{
    typedef uuids::uuid id;

    template <>
    struct hash<id>
    {
		// hashing by performing higher XOR lower
        inline std::size_t operator()(id const &d) const noexcept
        {
            //TODO: fix - this hash is very slow
			auto hash = std::hash<std::string>()(d.to_string());
			return hash;
			
			//auto raw = d.raw();
//			uint64_t lower, higher;
//			std::memcpy(&higher,raw.data(), sizeof(higher));
//			std::memcpy(&lower, raw.data() + sizeof(higher), sizeof(lower));
			//return (*reinterpret_cast<uint64_t*>(raw.data())) ^ (*reinterpret_cast<uint64_t*>(raw.data() + sizeof(uint64_t)));
        }
    };

    static inline ostream &operator<<(ostream &os, const id &d)
    {
        os << d.to_string();
        return os;
    }
} // namespace std

#endif // UUIDS_UUID_HPP
