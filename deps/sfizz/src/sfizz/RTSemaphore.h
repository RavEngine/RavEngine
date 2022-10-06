// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#if defined(__APPLE__)
#include <mach/mach.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <semaphore.h>
#endif
#include <cstdint>
#include <system_error>

class RTSemaphore {
public:
    explicit RTSemaphore(unsigned value = 0);
    explicit RTSemaphore(std::error_code& ec, unsigned value = 0) noexcept;
    ~RTSemaphore() noexcept;

    RTSemaphore(const RTSemaphore&) = delete;
    RTSemaphore& operator=(const RTSemaphore&) = delete;

    explicit operator bool() const noexcept { return good_; }

    void post();
    void wait();
    bool try_wait();
    bool timed_wait(uint32_t milliseconds);

    void post(std::error_code& ec) noexcept;
    void wait(std::error_code& ec) noexcept;
    bool try_wait(std::error_code& ec) noexcept;
    bool timed_wait(uint32_t milliseconds, std::error_code& ec) noexcept;

private:
    void init(std::error_code& ec, unsigned value);
    void destroy(std::error_code& ec);

private:
#if defined(__APPLE__)
    semaphore_t sem_ {};
    static const std::error_category& mach_category();
#elif defined(_WIN32)
    HANDLE sem_ {};
#else
    sem_t sem_ {};
#endif
    bool good_ {};
};
