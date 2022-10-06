// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <atomic>
#include "Debug.h"

#if __cplusplus >= 201703L
/**
 * @brief Tries to catch memory leaks by counting constructions
 * and deletions of objects. This will trap at the end of the program
 * execution if some elements were not properly deleted for one reason
 * or another. Use by adding the LEAK_DETECTOR macro at the end of a class
 * definition with the proper class name, e.g.
 *
 * @code{.cpp}
 * class Buffer
 * {
 *      // Some code for buffer
 *      LEAK_DETECTOR(Buffer);
 * }
 * @endcode
 *
 * @tparam Owner
 */
template <class Owner>
class LeakDetector {
public:
    LeakDetector()
    {
        objectCounter.count++;
    }
    LeakDetector(const LeakDetector&)
    {
        objectCounter.count++;
    }
    ~LeakDetector()
    {
        objectCounter.count--;
        if (objectCounter.count.load() < 0) {
            DBG("Deleted a dangling pointer for class " << Owner::getClassName());
            // Deleted a dangling pointer!
            ASSERTFALSE;
        }
    }

    LeakDetector& operator=(const LeakDetector&) = default;

private:
    struct ObjectCounter {
        ObjectCounter() = default;
        ~ObjectCounter()
        {
            if (auto residualCount = count.load() > 0) {
                DBG("Leaked " << residualCount << " instance(s) of class " << Owner::getClassName());
                // Leaked ojects
                ASSERTFALSE;
            }
        };
        std::atomic<int> count { 0 };
    };
	inline static ObjectCounter objectCounter;
};

#ifndef NDEBUG
#define LEAK_DETECTOR(Class)                             \
    friend class LeakDetector<Class>;                    \
    static const char* getClassName() { return #Class; } \
    LeakDetector<Class> leakDetector;
#else
#define LEAK_DETECTOR(Class)
#endif

#else
#define LEAK_DETECTOR(Class)
#endif
