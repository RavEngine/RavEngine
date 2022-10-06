// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "HelpersNEON.h"
#include "Common.h"

#if SFIZZ_HAVE_NEON
#include <arm_neon.h>
#endif

using Type = float;
constexpr unsigned TypeAlignment = 4;
constexpr unsigned ByteAlignment = TypeAlignment * sizeof(Type);
