// License: BSD 2 Clause
// Copyright (C) 2003+, Apple Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef LabSound_Assertions_h
#define LabSound_Assertions_h

#include "LabSound/core/Macros.h"
#include "LabSound/extended/Logging.h"

// FIXME: Change to use something other than ASSERT to avoid this conflict with the underlying platform
#if defined(LABSOUND_PLATFORM_WINDOWS)
#undef ASSERT
#define __func__ __FUNCTION__
#endif

#define CRASH() ((void) 0)

#define ASSERT(assertion)                                              \
    (!(assertion) ?                                                    \
         (LabSoundAssertLog(__FILE__, __LINE__, __func__, #assertion), \
          CRASH()) :                                                   \
         (void) 0)

#define ASSERT_AT(assertion, file, line, function)             \
    (!(assertion) ?                                            \
         (LabSoundAssertLog(file, line, function, #assertion), \
          CRASH()) :                                           \
         (void) 0)

#define ASSERT_NOT_REACHED()                                \
    do                                                      \
    {                                                       \
        LabSoundAssertLog(__FILE__, __LINE__, __func__, 0); \
        CRASH();                                            \
    } while (0)

#define ASSERT_WITH_MESSAGE(assertion, ...)                              \
    do                                                                   \
        if (!(assertion))                                                \
        {                                                                \
            LabSoundAssertLog(__FILE__, __LINE__, __func__, #assertion); \
            CRASH();                                                     \
        }                                                                \
    while (0)

#define FATAL(...)                                    \
    do                                                \
    {                                                 \
        LabSoundLog(__FILE__, __LINE__, __VA_ARGS__); \
        CRASH();                                      \
    } while (0)

#define ASSERT_UNUSED(variable, assertion) ASSERT(assertion)

#define ASSERT_WITH_SECURITY_IMPLICATION(assertion) ASSERT(assertion)

#endif
