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

#ifndef GU_DISTANCE_POINT_TRIANGLE_H
#define GU_DISTANCE_POINT_TRIANGLE_H

#include "foundation/PxVec3.h"
#include "common/PxPhysXCommonConfig.h"
#include "foundation/PxVecMath.h"

namespace physx
{
namespace Gu
{
	// PT: special version:
	// - inlined
	// - doesn't compute (s,t) output params
	// - expects precomputed edges in input
	PX_FORCE_INLINE PX_CUDA_CALLABLE PxVec3 closestPtPointTriangle2(const PxVec3& p, const PxVec3& a, const PxVec3& b, const PxVec3& c, const PxVec3& ab, const PxVec3& ac)
	{
		// Check if P in vertex region outside A
		//const PxVec3 ab = b - a;
		//const PxVec3 ac = c - a;
		const PxVec3 ap = p - a;
		const float d1 = ab.dot(ap);
		const float d2 = ac.dot(ap);
		if(d1<=0.0f && d2<=0.0f)
			return a;	// Barycentric coords 1,0,0

		// Check if P in vertex region outside B
		const PxVec3 bp = p - b;
		const float d3 = ab.dot(bp);
		const float d4 = ac.dot(bp);
		if(d3>=0.0f && d4<=d3)
			return b;	// Barycentric coords 0,1,0

		// Check if P in edge region of AB, if so return projection of P onto AB
		const float vc = d1*d4 - d3*d2;
		if(vc<=0.0f && d1>=0.0f && d3<=0.0f)
		{
			const float v = d1 / (d1 - d3);
			return a + v * ab;	// barycentric coords (1-v, v, 0)
		}

		// Check if P in vertex region outside C
		const PxVec3 cp = p - c;
		const float d5 = ab.dot(cp);
		const float d6 = ac.dot(cp);
		if(d6>=0.0f && d5<=d6)
			return c;	// Barycentric coords 0,0,1

		// Check if P in edge region of AC, if so return projection of P onto AC
		const float vb = d5*d2 - d1*d6;
		if(vb<=0.0f && d2>=0.0f && d6<=0.0f)
		{
			const float w = d2 / (d2 - d6);
			return a + w * ac;	// barycentric coords (1-w, 0, w)
		}

		// Check if P in edge region of BC, if so return projection of P onto BC
		const float va = d3*d6 - d5*d4;
		if(va<=0.0f && (d4-d3)>=0.0f && (d5-d6)>=0.0f)
		{
			const float w = (d4-d3) / ((d4 - d3) + (d5-d6));
			return b + w * (c-b);	// barycentric coords (0, 1-w, w)
		}

		// P inside face region. Compute Q through its barycentric coords (u,v,w)
		const float denom = 1.0f / (va + vb + vc);
		const float v = vb * denom;
		const float w = vc * denom;
		return a + ab*v + ac*w;
	}

	//Scales and translates triangle and query points to fit into the unit box to make calculations less prone to numerical cancellation. 
	//The returned point will still be in the same space as the input points.
	PX_FORCE_INLINE PX_CUDA_CALLABLE PxVec3 closestPtPointTriangle2UnitBox(const PxVec3& queryPoint, const PxVec3& triA, const PxVec3& triB, const PxVec3& triC)	
	{
		const PxVec3 min = queryPoint.minimum(triA.minimum(triB.minimum(triC)));
		const PxVec3 max = queryPoint.maximum(triA.maximum(triB.maximum(triC)));
		const PxVec3 size = max - min;

		PxReal invScaling = PxMax(PxMax(size.x, size.y), PxMax(1e-12f, size.z));
		PxReal scaling = 1.0f / invScaling;

		PxVec3 p = (queryPoint - min) * scaling;
		PxVec3 a = (triA - min) * scaling;
		PxVec3 b = (triB - min) * scaling;
		PxVec3 c = (triC - min) * scaling;

		PxVec3 result = closestPtPointTriangle2(p, a, b, c, b - a, c - a);

		return result * invScaling + min;
	}

	// Given the point `c`, return the closest point on the triangle (1, 0, 0), (0, 1, 0), (0, 0, 1).
	// This function is a specialization of `Gu::closestPtPointTriangle2` for this specific triangle.
	PX_FORCE_INLINE PX_CUDA_CALLABLE PxVec3 closestPtPointBaryTriangle(PxVec3 c)
	{
		const PxReal third = 1.0f / 3.0f; // constexpr
		c -= PxVec3(third * (c.x + c.y + c.z - 1.0f));

		// two negative: return positive vertex
		if (c.y < 0.0f && c.z < 0.0f)
			return PxVec3(1.0f, 0.0f, 0.0f);

		if (c.x < 0.0f && c.z < 0.0f)
			return PxVec3(0.0f, 1.0f, 0.0f);

		if (c.x < 0.0f && c.y < 0.0f)
			return PxVec3(0.0f, 0.0f, 1.0f);

		// one negative: return projection onto line if it is on the edge, or the largest vertex otherwise
		if (c.x < 0.0f)
		{
			const PxReal d = c.x * 0.5f;
			const PxReal y = c.y + d;
			const PxReal z = c.z + d;
			if (y > 1.0f)
				return PxVec3(0.0f, 1.0f, 0.0f);
			if (z > 1.0f)
				return PxVec3(0.0f, 0.0f, 1.0f);
			return PxVec3(0.0f, y, z);
		}
		if (c.y < 0.0f)
		{
			const PxReal d = c.y * 0.5f;
			const PxReal x = c.x + d;
			const PxReal z = c.z + d;
			if (x > 1.0f)
				return PxVec3(1.0f, 0.0f, 0.0f);
			if (z > 1.0f)
				return PxVec3(0.0f, 0.0f, 1.0f);
			return PxVec3(x, 0.0f, z);
		}
		if (c.z < 0.0f)
		{
			const PxReal d = c.z * 0.5f;
			const PxReal x = c.x + d;
			const PxReal y = c.y + d;
			if (x > 1.0f)
				return PxVec3(1.0f, 0.0f, 0.0f);
			if (y > 1.0f)
				return PxVec3(0.0f, 1.0f, 0.0f);
			return PxVec3(x, y, 0.0f);
		}
		return c;
	}


	PX_PHYSX_COMMON_API PxVec3 closestPtPointTriangle(const PxVec3& p, const PxVec3& a, const PxVec3& b, const PxVec3& c, float& s, float& t);

	PX_FORCE_INLINE PxReal distancePointTriangleSquared(const PxVec3& point, 
														const PxVec3& triangleOrigin, const PxVec3& triangleEdge0, const PxVec3& triangleEdge1,
														PxReal* param0=NULL, PxReal* param1=NULL)
	{
		const PxVec3 pt0 = triangleEdge0 + triangleOrigin;
		const PxVec3 pt1 = triangleEdge1 + triangleOrigin;
		float s,t;
		const PxVec3 cp = closestPtPointTriangle(point, triangleOrigin, pt0, pt1, s, t);
		if(param0)
			*param0 = s;
		if(param1)
			*param1 = t;
		return (cp - point).magnitudeSquared();
	}

	PX_PHYSX_COMMON_API aos::FloatV distancePointTriangleSquared(	const aos::Vec3VArg point, 
																	const aos::Vec3VArg a, 
																	const aos::Vec3VArg b, 
																	const aos::Vec3VArg c,
																	aos::FloatV& u,
																	aos::FloatV& v,
																	aos::Vec3V& closestP); 

	//Scales and translates triangle and query points to fit into the unit box to make calculations less prone to numerical cancellation. 
	//The returned point and squared distance will still be in the same space as the input points.
	PX_PHYSX_COMMON_API aos::FloatV distancePointTriangleSquared2UnitBox(
		const aos::Vec3VArg point,
		const aos::Vec3VArg a,
		const aos::Vec3VArg b,
		const aos::Vec3VArg c,
		aos::FloatV& u,
		aos::FloatV& v,
		aos::Vec3V& closestP);

} // namespace Gu
}

#endif
