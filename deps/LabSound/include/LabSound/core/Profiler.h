// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2020, The LabSound Authors. All rights reserved.

#include <chrono>

namespace lab
{

    struct ProfileSample
    {
        ProfileSample() { zero(); }
        bool finalized = false;
        std::chrono::duration<float, std::micro> microseconds;
        void zero() { microseconds = std::chrono::duration<float, std::micro>::zero(); finalized = true; }
    };

    struct ProfileScope
    {
        explicit ProfileScope(ProfileSample& s)
        : s(&s)
        {
            _start = std::chrono::system_clock::now();
            s.finalized = false;
        }

        ~ProfileScope()
        {
            finalize();
        }

        void finalize()
        {
            if (!s->finalized)
            {
                //std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
                s->microseconds = std::chrono::system_clock::now() - _start;
                s->finalized = true;
            }
        }

        ProfileSample* s = nullptr;
        std::chrono::system_clock::time_point _start;
    };

} // lab
