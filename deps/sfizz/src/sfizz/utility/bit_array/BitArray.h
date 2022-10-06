// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

///
class ConstBitSpan {
public:
    ConstBitSpan() noexcept = default;
    ConstBitSpan(const uint8_t* data, size_t bits) noexcept : data_(data), bits_(bits) {}
    const uint8_t* data() const noexcept { return data_; }
    size_t bit_size() const noexcept { return bits_; }
    size_t byte_size() const noexcept { return (bits_ + 7) / 8; }
    bool test(size_t i) const noexcept { return data_[i / 8] & (1u << (i % 8)); }
    bool all() const noexcept
    {
        size_t n = bits_;
        for (size_t i = 0; i < n / 8; ++i) {
            if (data_[i] != 0xff)
                return false;
        }
        return n % 8 == 0 || data_[n / 8] == (1u << (n % 8)) - 1u;
    }
    bool any() const noexcept
    {
        size_t n = bits_;
        for (size_t i = 0; i < n / 8; ++i) {
            if (data_[i] != 0x00)
                return true;
        }
        return n % 8 != 0 && (data_[n / 8] & ((1u << (n % 8)) - 1u)) != 0;
    }
    bool none() const noexcept { return !any(); }

private:
    const uint8_t* data_ = nullptr;
    size_t bits_ = 0;
};

///
class BitSpan : public ConstBitSpan {
public:
    BitSpan() noexcept = default;
    BitSpan(const uint8_t* data, size_t bits) noexcept : ConstBitSpan(data, bits) {}
    uint8_t* data() const noexcept { return const_cast<uint8_t*>(ConstBitSpan::data()); }
    void clear() { memset(data(), 0, byte_size()); }
    void set(size_t i) noexcept { data()[i / 8] |= 1u << (i % 8); }
    void set(size_t i, bool b) noexcept { if (b) set(i); else reset(i); }
    void reset(size_t i) noexcept { data()[i / 8] &= ~(1u << (i % 8)); }
    void flip(size_t i) noexcept { data()[i / 8] ^= 1u << (i % 8); }
};

///
template <size_t N>
class BitArray {
public:
    BitArray() noexcept = default;
    uint8_t* data() noexcept { return data_; }
    const uint8_t* data() const noexcept { return data_; }
    static constexpr size_t bit_size() noexcept { return N; }
    static constexpr size_t byte_size() noexcept { return (N + 7) / 8; }
    BitSpan span() noexcept { return BitSpan(data_, N); };
    ConstBitSpan span() const noexcept { return ConstBitSpan(data_, N); };
    void clear() { span().clear(); }
    bool test(size_t i) const noexcept { return span().test(i); }
    void set(size_t i) noexcept { span().set(i); }
    void set(size_t i, bool b) noexcept { span().set(i, b); }
    void reset(size_t i) noexcept { span().reset(i); }
    void flip(size_t i) noexcept { span().flip(i); }
    bool all() const noexcept { return span().all(); }
    bool any() const noexcept { return span().any(); }
    bool none() const noexcept { return span().none(); }

private:
    uint8_t data_[byte_size()] {};
};
