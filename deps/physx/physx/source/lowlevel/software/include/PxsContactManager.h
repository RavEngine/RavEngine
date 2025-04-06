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

#ifndef PXS_CONTACT_MANAGER_H
#define PXS_CONTACT_MANAGER_H

#include "PxvConfig.h"
#include "PxcNpWorkUnit.h"

namespace physx
{
class PxsRigidBody;

namespace Dy
{
	class DynamicsContext;
}

namespace Sc
{
	class ShapeInteraction;
}

/**
\brief Additional header structure for CCD contact data stream.
*/
struct PxsCCDContactHeader
{
	/**
	\brief Stream for next collision. The same pair can collide multiple times during multiple CCD passes.
	*/
	PxsCCDContactHeader* nextStream;	//4    //8
	/**
	\brief Size (in bytes) of the CCD contact stream (not including force buffer)
	*/
	PxU16 contactStreamSize;			//6    //10
	/**
	\brief Defines whether the stream is from a previous pass.
	
	It could happen that the stream can not get allocated because we run out of memory. In that case the current event should not use the stream
	from an event of the previous pass.
	*/
	PxU16 isFromPreviousPass;			//8    //12

	PxU8 pad[12 - sizeof(PxsCCDContactHeader*)];	//16
};

PX_COMPILE_TIME_ASSERT((sizeof(PxsCCDContactHeader) & 0xF) == 0);

class PxsContactManager
{
public:
											PxsContactManager(PxU32 index);
											~PxsContactManager();

	PX_FORCE_INLINE	void					setDisableStrongFriction(PxU32 d)	{ (!d)	? mNpUnit.mFlags &= ~PxcNpWorkUnitFlag::eDISABLE_STRONG_FRICTION 
																						: mNpUnit.mFlags |= PxcNpWorkUnitFlag::eDISABLE_STRONG_FRICTION; }

	PX_FORCE_INLINE	PxReal					getRestDistance()			const	{ return mNpUnit.mRestDistance;	}
	PX_FORCE_INLINE	void					setRestDistance(PxReal v)			{ mNpUnit.mRestDistance = v;	}

	PX_FORCE_INLINE	PxU8					getDominance0()				const	{ return mNpUnit.getDominance0();	}
	PX_FORCE_INLINE	void					setDominance0(PxU8 v)				{ mNpUnit.setDominance0(v);			}

	PX_FORCE_INLINE	PxU8					getDominance1()				const	{ return mNpUnit.getDominance1();	}
	PX_FORCE_INLINE	void					setDominance1(PxU8 v)				{ mNpUnit.setDominance1(v);			}

	PX_FORCE_INLINE	PxU16					getTouchStatus()			const	{ return PxU16(mNpUnit.mStatusFlags & PxcNpWorkUnitStatusFlag::eHAS_TOUCH); }
	PX_FORCE_INLINE	PxU16					touchStatusKnown()			const	{ return PxU16(mNpUnit.mStatusFlags & PxcNpWorkUnitStatusFlag::eTOUCH_KNOWN); }
	PX_FORCE_INLINE	PxI32					getTouchIdx()				const	{ return (mNpUnit.mStatusFlags & PxcNpWorkUnitStatusFlag::eHAS_TOUCH) ? 1 : (mNpUnit.mStatusFlags & PxcNpWorkUnitStatusFlag::eHAS_NO_TOUCH ? -1 : 0); }

	PX_FORCE_INLINE	PxU32					getIndex()					const	{ return mCmIndex;	}

	PX_FORCE_INLINE	PxU16					getHasCCDRetouch()			const	{ return PxU16(mNpUnit.mStatusFlags & PxcNpWorkUnitStatusFlag::eHAS_CCD_RETOUCH); }
	PX_FORCE_INLINE	void					clearCCDRetouch()					{ mNpUnit.mStatusFlags &= ~PxcNpWorkUnitStatusFlag::eHAS_CCD_RETOUCH; }
	PX_FORCE_INLINE	void					raiseCCDRetouch()					{ mNpUnit.mStatusFlags |= PxcNpWorkUnitStatusFlag::eHAS_CCD_RETOUCH; }

	// flags stuff - needs to be refactored

	PX_FORCE_INLINE	PxIntBool				isChangeable()				const	{ return PxIntBool(mFlags & PXS_CM_CHANGEABLE);		}
	PX_FORCE_INLINE	PxIntBool				getCCD()					const	{ return PxIntBool((mFlags & PXS_CM_CCD_LINEAR) && (mNpUnit.mFlags & PxcNpWorkUnitFlag::eDETECT_CCD_CONTACTS)); }
	PX_FORCE_INLINE	PxIntBool				getHadCCDContact()			const	{ return PxIntBool(mFlags & PXS_CM_CCD_CONTACT); }
	PX_FORCE_INLINE	void					setHadCCDContact()					{ mFlags |= PXS_CM_CCD_CONTACT; }
					void					setCCD(bool enable);
	PX_FORCE_INLINE	void					clearCCDContactInfo()				{ mFlags &= ~PXS_CM_CCD_CONTACT; mNpUnit.mCCDContacts = NULL; }

	PX_FORCE_INLINE	PxcNpWorkUnit&			getWorkUnit()						{ return mNpUnit;	}
	PX_FORCE_INLINE	const PxcNpWorkUnit&	getWorkUnit()				const	{ return mNpUnit;	}

	PX_FORCE_INLINE	PxsRigidBody*			getRigidBody0()				const	{ return mRigidBody0;		}
	PX_FORCE_INLINE	PxsRigidBody*			getRigidBody1()				const	{ return mRigidBody1;		}
	PX_FORCE_INLINE	Sc::ShapeInteraction*	getShapeInteraction()		const	{ return mShapeInteraction; }
	
	// Setup solver-constraints
	PX_FORCE_INLINE	void					resetCachedState()
											{ 
												// happens when the body transform or shape relative transform changes.
												mNpUnit.clearCachedState();
											}
private:
					//KS - moving this up - we want to get at flags
					
					PxsRigidBody*			mRigidBody0;
					PxsRigidBody*			mRigidBody1;
					PxU32					mFlags;
					PxU32					mCmIndex;	// PT: moved to padding bytes from mNpUnit
					Sc::ShapeInteraction*	mShapeInteraction;

	// everything required for narrow phase to run
					PxcNpWorkUnit			mNpUnit;
	enum
	{
		PXS_CM_CHANGEABLE	= (1 << 0),
		PXS_CM_CCD_LINEAR	= (1 << 1),
		PXS_CM_CCD_CONTACT	= (1 << 2)
	};

	friend class Sc::ShapeInteraction;
};

}

#endif
