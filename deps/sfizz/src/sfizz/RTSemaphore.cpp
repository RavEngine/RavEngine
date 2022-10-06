// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "RTSemaphore.h"
#include <limits.h>
#include <string>
#include <cerrno>
#include <ctime>

RTSemaphore::RTSemaphore(unsigned value)
{
    std::error_code ec;
    init(ec, value);
    if (ec)
        throw std::system_error(ec);
    good_ = true;
}

RTSemaphore::RTSemaphore(std::error_code& ec, unsigned value) noexcept
{
    init(ec, value);
    good_ = ec ? false : true;
}

RTSemaphore::~RTSemaphore() noexcept
{
    if (good_) {
        std::error_code ec;
        destroy(ec);
    }
}

void RTSemaphore::post()
{
    std::error_code ec;
    post(ec);
    if (ec)
        throw std::system_error(ec);
}

void RTSemaphore::wait()
{
    std::error_code ec;
    wait(ec);
    if (ec)
        throw std::system_error(ec);
}

bool RTSemaphore::try_wait()
{
    std::error_code ec;
    bool b = try_wait(ec);
    if (ec)
        throw std::system_error(ec);
    return b;
}

bool RTSemaphore::timed_wait(uint32_t milliseconds)
{
    std::error_code ec;
    bool b = timed_wait(milliseconds, ec);
    if (ec)
        throw std::system_error(ec);
    return b;
}

#if defined(__APPLE__)
void RTSemaphore::init(std::error_code& ec, unsigned value)
{
    ec.clear();
    kern_return_t ret = semaphore_create(mach_task_self(), &sem_, SYNC_POLICY_FIFO, value);
    if (ret != KERN_SUCCESS)
        ec = std::error_code(ret, mach_category());
}

void RTSemaphore::destroy(std::error_code& ec)
{
    ec.clear();
    kern_return_t ret = semaphore_destroy(mach_task_self(), sem_);
    if (ret != KERN_SUCCESS)
        ec = std::error_code(ret, mach_category());
}

void RTSemaphore::post(std::error_code& ec) noexcept
{
    ec.clear();
    kern_return_t ret = semaphore_signal(sem_);
    if (ret != KERN_SUCCESS)
        ec = std::error_code(ret, mach_category());
}

void RTSemaphore::wait(std::error_code& ec) noexcept
{
    ec.clear();
    do {
        kern_return_t ret = semaphore_wait(sem_);
        switch (ret) {
        case KERN_SUCCESS:
            return;
        case KERN_ABORTED:
            break;
        default:
            ec = std::error_code(ret, mach_category());
            return;
        }
    } while (1);
}

bool RTSemaphore::try_wait(std::error_code& ec) noexcept
{
    return timed_wait(0, ec);
}

bool RTSemaphore::timed_wait(uint32_t milliseconds, std::error_code& ec) noexcept
{
    ec.clear();
    do {
        mach_timespec_t timeout;
        timeout.tv_sec = milliseconds / 1000;
        timeout.tv_nsec = (milliseconds % 1000) * (1000L * 1000L);
        kern_return_t ret = semaphore_timedwait(sem_, timeout);
        switch (ret) {
        case KERN_SUCCESS:
            return true;
        case KERN_OPERATION_TIMED_OUT:
            return false;
        case KERN_ABORTED:
            break;
        default:
            ec = std::error_code(ret, mach_category());
            return false;
        }
    } while (1);
}

const std::error_category& RTSemaphore::mach_category()
{
    class mach_category : public std::error_category {
    public:
        const char* name() const noexcept override
        {
            return "kern_return_t";
        }

        std::string message(int condition) const override
        {
            const char* str = mach_error_string(condition);
            return str ? str : "";
        }
    };

    static const mach_category cat;
    return cat;
}
#elif defined(_WIN32)
void RTSemaphore::init(std::error_code& ec, unsigned value)
{
    ec.clear();
    sem_ = CreateSemaphore(nullptr, value, LONG_MAX, nullptr);
    if (!sem_)
        ec = std::error_code(GetLastError(), std::system_category());
}

void RTSemaphore::destroy(std::error_code& ec)
{
    ec.clear();
    if (CloseHandle(sem_) == 0)
        ec = std::error_code(GetLastError(), std::system_category());
}

void RTSemaphore::post(std::error_code& ec) noexcept
{
    ec.clear();
    if (ReleaseSemaphore(sem_, 1, nullptr) == 0)
        ec = std::error_code(GetLastError(), std::system_category());
}

void RTSemaphore::wait(std::error_code& ec) noexcept
{
    ec.clear();
    DWORD ret = WaitForSingleObject(sem_, INFINITE);
    switch (ret) {
    case WAIT_OBJECT_0:
        return;
    case WAIT_FAILED:
        ec = std::error_code(GetLastError(), std::system_category());
        return;
    default:
        ec = std::error_code(ret, std::system_category());
        return;
    }
}

bool RTSemaphore::try_wait(std::error_code& ec) noexcept
{
    return timed_wait(0, ec);
}

bool RTSemaphore::timed_wait(uint32_t milliseconds, std::error_code& ec) noexcept
{
    ec.clear();
    DWORD ret = WaitForSingleObject(sem_, milliseconds);
    switch (ret) {
    case WAIT_OBJECT_0:
        return true;
    case WAIT_TIMEOUT:
        return false;
    case WAIT_FAILED:
        ec = std::error_code(GetLastError(), std::system_category());
        return false;
    default:
        ec = std::error_code(ret, std::system_category());
        return false;
    }
}
#else
void RTSemaphore::init(std::error_code& ec, unsigned value)
{
    ec.clear();
    if (sem_init(&sem_, 0, value) != 0)
        ec = std::error_code(errno, std::generic_category());
}

void RTSemaphore::destroy(std::error_code& ec)
{
    ec.clear();
    if (sem_destroy(&sem_) != 0)
        ec = std::error_code(errno, std::generic_category());
}

void RTSemaphore::post(std::error_code& ec) noexcept
{
    ec.clear();
    while (sem_post(&sem_) != 0) {
        int e = errno;
        if (e != EINTR) {
            ec = std::error_code(e, std::generic_category());
            return;
        }
    }
}

void RTSemaphore::wait(std::error_code& ec) noexcept
{
    ec.clear();
    while (sem_wait(&sem_) != 0) {
        int e = errno;
        if (e != EINTR) {
            ec = std::error_code(e, std::generic_category());
            return;
        }
    }
}

bool RTSemaphore::try_wait(std::error_code& ec) noexcept
{
    ec.clear();
    do {
        if (sem_trywait(&sem_) == 0)
            return true;
        int e = errno;
        switch (e) {
        case EINTR:
            break;
        case EAGAIN:
            return false;
        default:
            ec = std::error_code(e, std::generic_category());
            return false;
        }
    } while (1);
}

static bool absolute_timeout(uint32_t milliseconds, timespec &result, std::error_code& ec)
{
    timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now) != 0) {
        ec = std::error_code(errno, std::generic_category());
        return false;
    }

    timespec abs;
    abs.tv_sec = now.tv_sec + milliseconds / 1000;
    abs.tv_nsec = now.tv_nsec + (milliseconds % 1000) * (1000L * 1000L);

    long abs_nsec_sec = abs.tv_nsec / (1000L * 1000L * 1000L);
    abs.tv_sec += abs_nsec_sec;
    abs.tv_nsec -= abs_nsec_sec * (1000L * 1000L * 1000L);

    result = abs;
    return true;
}

bool RTSemaphore::timed_wait(uint32_t milliseconds, std::error_code& ec) noexcept
{
    ec.clear();
    timespec abs;
    if (!absolute_timeout(milliseconds, abs, ec))
        return false;
    do {
        if (sem_timedwait(&sem_, &abs) == 0)
            return true;
        int e = errno;
        switch (e) {
        case EINTR:
            break;
        case ETIMEDOUT:
            return false;
        default:
            ec = std::error_code(e, std::generic_category());
            return false;
        }
    } while (1);
}
#endif
