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
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
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
// Copyright (c) 2008-2025 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#ifndef PX_CONE_LIMITED_CONSTRAINT_H
#define PX_CONE_LIMITED_CONSTRAINT_H


#include "foundation/PxVec3.h"
#include "foundation/PxVec4.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

/**
\brief A constraint descriptor for limiting movement to a conical region.
*/
struct PxConeLimitedConstraint
{
	PxConeLimitedConstraint()
	{
		setToDefault();
	}

	/**
	\brief Set values such that constraint is disabled.
	*/
	PX_INLINE void setToDefault()
	{
		mAxis = PxVec3(0.f, 0.f, 0.f);
		mAngle = -1.f;
		mLowLimit = -1.f;
		mHighLimit = -1.f;
	}

	/**
	\brief Checks for valitity.

	\return	true if the constaint is valid
	*/
	PX_INLINE bool isValid() const
	{
		//disabled
		if (mAngle < 0.f && mLowLimit < 0.f && mHighLimit < 0.f)
		{
			return true;
		}

		if (!mAxis.isNormalized())
		{
			return false;
		}

		//negative signifies that cone is disabled
		if (mAngle >= PxPi)
		{
			return false;
		}

		//negative signifies that distance limits are disabled
		if (mLowLimit > mHighLimit && mHighLimit >= 0.0f && mLowLimit >= 0.0f)
		{
			return false;
		}

		return true;
	}

	PxVec3 mAxis;		//!< Axis of the cone in actor space
	PxReal mAngle;		//!< Opening angle in radians, negative indicates unlimited
	PxReal mLowLimit;	//!< Minimum distance, negative indicates unlimited
	PxReal mHighLimit;	//!< Maximum distance, negative indicates unlimited
};

/**
\brief Compressed form of cone limit parameters
\see PxConeLimitedConstraint
*/
PX_ALIGN_PREFIX(16)
struct PxConeLimitParams
{
	PX_CUDA_CALLABLE PxConeLimitParams() {}

	PX_CUDA_CALLABLE PxConeLimitParams(const PxConeLimitedConstraint& coneLimitedConstraint) :
		lowHighLimits(coneLimitedConstraint.mLowLimit, coneLimitedConstraint.mHighLimit, 0.0f, 0.0f),
		axisAngle(coneLimitedConstraint.mAxis, coneLimitedConstraint.mAngle)
	{
	}

	PxVec4 lowHighLimits; // [lowLimit, highLimit, unused, unused]
	PxVec4 axisAngle; // [axis.x, axis.y, axis.z, angle]
}PX_ALIGN_SUFFIX(16);

#if !PX_DOXYGEN
} // namespace physx
#endif

#endif
