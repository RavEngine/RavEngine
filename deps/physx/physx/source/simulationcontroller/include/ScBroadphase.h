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

#ifndef SC_BROADPHASE_H
#define SC_BROADPHASE_H

#include "PxvConfig.h"
#include "foundation/PxArray.h"

// PT: this class captures parts of the Sc::Scene that deals with broadphase matters.

namespace physx
{
	class PxBroadPhaseCallback;

namespace Bp
{
	class AABBManagerBase;
}

namespace Sc
{
	class ObjectIDTracker;

	class BroadphaseManager
	{
		public:
													BroadphaseManager();
													~BroadphaseManager();

			PX_FORCE_INLINE	void					setBroadPhaseCallback(PxBroadPhaseCallback* callback)	{ mBroadPhaseCallback = callback;	}
			PX_FORCE_INLINE	PxBroadPhaseCallback*	getBroadPhaseCallback()	const							{ return mBroadPhaseCallback;		}

							void					prepareOutOfBoundsCallbacks(Bp::AABBManagerBase* aabbManager);
							bool					fireOutOfBoundsCallbacks(Bp::AABBManagerBase* aabbManager, const ObjectIDTracker& tracker, PxU64 contextID);

							void					flush(Bp::AABBManagerBase* aabbManager);

							PxBroadPhaseCallback*	mBroadPhaseCallback;
							PxArray<PxU32>			mOutOfBoundsIDs;
	};

}
}

#endif
