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


#ifndef PXS_MATERIAL_H
#define PXS_MATERIAL_H

#include "foundation/PxVec3.h"
#include "common/PxMetaData.h"
#include "PxMaterial.h"
#include "PsUserAllocated.h"
#include "PsUtilities.h"
#include "CmPhysXCommon.h"
#include "CmUtils.h"

namespace physx
{

#define	MATERIAL_INVALID_HANDLE	0xffff

	PX_ALIGN_PREFIX(16) struct PxsMaterialData 
	{
		PxReal					dynamicFriction;				//4
		PxReal					staticFriction;					//8
		PxReal					restitution;					//12
		PxMaterialFlags			flags;							//14
		PxU8					fricRestCombineMode;			//15
		PxU8					padding;						//16

		PxsMaterialData()
		:	dynamicFriction(0.0f)
		,	staticFriction(0.0f)
		,	restitution(0.0f)
		,	fricRestCombineMode((PxCombineMode::eAVERAGE << 4) | PxCombineMode::eAVERAGE)
		,	padding(PX_PADDING_8)
		{}

		PxsMaterialData(const PxEMPTY) {}

		PX_CUDA_CALLABLE PX_FORCE_INLINE PxCombineMode::Enum getFrictionCombineMode() const
		{
			return PxCombineMode::Enum(fricRestCombineMode >> 4);
		}

		PX_CUDA_CALLABLE PX_FORCE_INLINE PxCombineMode::Enum getRestitutionCombineMode() const
		{
			return PxCombineMode::Enum(fricRestCombineMode & 0xf);
		}

		PX_FORCE_INLINE void setFrictionCombineMode(PxCombineMode::Enum frictionFlags)
		{
			fricRestCombineMode = Ps::to8((fricRestCombineMode & 0xf) | (frictionFlags << 4));
		}

		PX_FORCE_INLINE void setRestitutionCombineMode(PxCombineMode::Enum restitutionFlags)
		{
			fricRestCombineMode = Ps::to8((fricRestCombineMode & 0xf0) | (restitutionFlags));
		}

	}PX_ALIGN_SUFFIX(16);


	class PxsMaterialCore : public PxsMaterialData, public Ps::UserAllocated
	{
	public:
					
											PxsMaterialCore(const PxsMaterialData& desc)
											:	PxsMaterialData(desc)
											,	mNxMaterial(0)
											,	mMaterialIndex(MATERIAL_INVALID_HANDLE)
											,	mPadding(PX_PADDING_16)
											{
											}

											PxsMaterialCore()
											:	mNxMaterial(0)
											,	mMaterialIndex(MATERIAL_INVALID_HANDLE)
											,	mPadding(PX_PADDING_16)
											{
											}

											PxsMaterialCore(const PxEMPTY) : PxsMaterialData(PxEmpty) {}

											~PxsMaterialCore()
											{
											}

	PX_FORCE_INLINE	void					setNxMaterial(PxMaterial* m)					{ mNxMaterial = m;		}
	PX_FORCE_INLINE	PxMaterial*				getNxMaterial()					const			{ return mNxMaterial;	}
	PX_FORCE_INLINE	void					setMaterialIndex(const PxU16 materialIndex)		{ mMaterialIndex = materialIndex; }
	PX_FORCE_INLINE	PxU16					getMaterialIndex()				const			{ return mMaterialIndex; }

	protected:
					PxMaterial*				mNxMaterial;
					PxU16					mMaterialIndex; //handle assign by the handle manager
					PxU16					mPadding;
	};

} //namespace phyxs

#endif
