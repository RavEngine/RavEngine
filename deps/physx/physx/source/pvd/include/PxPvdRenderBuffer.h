//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2019 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#ifndef PXPVDSDK_PXPVDRENDERBUFFER_H
#define PXPVDSDK_PXPVDRENDERBUFFER_H

/** \addtogroup pvd
@{
*/

#include "foundation/PxVec3.h"

#if !PX_DOXYGEN
namespace physx
{
namespace pvdsdk
{
#endif

/**
\brief Default color values used for debug rendering.
*/
struct PvdDebugColor
{
	enum Enum
	{
		eARGB_BLACK     = 0xff000000,
		eARGB_RED       = 0xffff0000,
		eARGB_GREEN     = 0xff00ff00,
		eARGB_BLUE      = 0xff0000ff,
		eARGB_YELLOW    = 0xffffff00,
		eARGB_MAGENTA   = 0xffff00ff,
		eARGB_CYAN      = 0xff00ffff,
		eARGB_WHITE     = 0xffffffff,
		eARGB_GREY      = 0xff808080,
		eARGB_DARKRED   = 0x88880000,
		eARGB_DARKGREEN = 0x88008800,
		eARGB_DARKBLUE  = 0x88000088
	};
};

/**
\brief Used to store a single point and colour for debug rendering.
*/
struct PvdDebugPoint
{
	PvdDebugPoint(const PxVec3& p, const uint32_t& c) : pos(p), color(c)
	{
	}

	PxVec3 pos;
	uint32_t color;
};

/**
\brief Used to store a single line and colour for debug rendering.
*/
struct PvdDebugLine
{
	PvdDebugLine(const PxVec3& p0, const PxVec3& p1, const uint32_t& c) : pos0(p0), color0(c), pos1(p1), color1(c)
	{
	}

	PxVec3 pos0;
	uint32_t color0;
	PxVec3 pos1;
	uint32_t color1;
};

/**
\brief Used to store a single triangle and colour for debug rendering.
*/
struct PvdDebugTriangle
{
	PvdDebugTriangle(const PxVec3& p0, const PxVec3& p1, const PxVec3& p2, const uint32_t& c)
	: pos0(p0), color0(c), pos1(p1), color1(c), pos2(p2), color2(c)
	{
	}

	PxVec3 pos0;
	uint32_t color0;
	PxVec3 pos1;
	uint32_t color1;
	PxVec3 pos2;
	uint32_t color2;
};

/**
\brief Used to store a text for debug rendering. Doesn't own 'string' array.
*/
struct PvdDebugText
{
	PvdDebugText() : string(0)
	{
	}

	PvdDebugText(const PxVec3& p, const float& s, const uint32_t& c, const char* str)
	: position(p), size(s), color(c), string(str)
	{
	}

	PxVec3 position;
	float size;
	uint32_t color;
	const char* string;
};

#if !PX_DOXYGEN
}
} // namespace physx
#endif

/** @} */
#endif // PXPVDSDK_PXPVDRENDERBUFFER_H
