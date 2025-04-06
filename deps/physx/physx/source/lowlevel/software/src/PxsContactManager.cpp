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

#include "PxsContactManager.h"

using namespace physx;

PxsContactManager::PxsContactManager(PxU32 index) : mFlags(0), mCmIndex(index)
{
	// PT: TODO: any reason why we don't initialize all members here, e.g. shapeCore pointers?
	// PT: it might be because of the way we preallocate contact managers in the pipeline, and release the ones
	// we filtered out. Maybe properly initializing everything "for no reason" in that case is costly.
	// Still, it is unclear why we initialize *some* of the members there then.
	mNpUnit.mRigidCore0			= NULL;
	mNpUnit.mRigidCore1			= NULL;
	mNpUnit.mRestDistance		= 0;
	mNpUnit.mFrictionDataPtr	= NULL;
	mNpUnit.mFrictionPatchCount	= 0;

	mNpUnit.mFlags = 0;
	mNpUnit.setDominance0(1u);
	mNpUnit.setDominance1(1u);
}

PxsContactManager::~PxsContactManager()
{
}

void PxsContactManager::setCCD(bool enable)
{
	PxU32 flags = mFlags & (~PXS_CM_CCD_CONTACT);
	if (enable)
		flags |= PXS_CM_CCD_LINEAR;
	else
		flags &= ~PXS_CM_CCD_LINEAR;

	mFlags = flags;
}

