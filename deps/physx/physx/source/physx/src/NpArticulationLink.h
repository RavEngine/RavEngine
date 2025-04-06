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

#ifndef NP_ARTICULATION_LINK_H
#define NP_ARTICULATION_LINK_H

#include "NpRigidBodyTemplate.h"
#include "PxArticulationLink.h"

#if PX_ENABLE_DEBUG_VISUALIZATION
	#include "common/PxRenderOutput.h"
#else
	PX_CATCH_UNDEFINED_ENABLE_DEBUG_VISUALIZATION
#endif

namespace physx
{

class NpArticulationLink;
class NpArticulationJointReducedCoordinate;
class PxConstraintVisualizer;

typedef NpRigidBodyTemplate<PxArticulationLink> NpArticulationLinkT;

class NpArticulationLinkArray : public PxInlineArray<NpArticulationLink*, 4>  //!!!AL TODO: check if default of 4 elements makes sense
{
public:
// PX_SERIALIZATION
	NpArticulationLinkArray(const PxEMPTY) : PxInlineArray<NpArticulationLink*, 4> (PxEmpty) {}
//~PX_SERIALIZATION
	NpArticulationLinkArray() : PxInlineArray<NpArticulationLink*, 4>("articulationLinkArray") {}
};

class NpArticulationLink : public NpArticulationLinkT
{
public:
// PX_SERIALIZATION
											NpArticulationLink(PxBaseFlags baseFlags) : NpArticulationLinkT(baseFlags), mChildLinks(PxEmpty)	{}
				void						preExportDataReset() { NpArticulationLinkT::preExportDataReset(); }
	virtual		void						exportExtraData(PxSerializationContext& context);
				void						importExtraData(PxDeserializationContext& context);
				void						resolveReferences(PxDeserializationContext& context);
	virtual		void						requiresObjects(PxProcessPxBaseCallback& c);
	virtual		bool						isSubordinate()  const	 { return true; } 
	static		NpArticulationLink*			createObject(PxU8*& address, PxDeserializationContext& context);
//~PX_SERIALIZATION
											NpArticulationLink(const PxTransform& bodyPose, PxArticulationReducedCoordinate& root, NpArticulationLink* parent);
	virtual									~NpArticulationLink();

	// PxBase
	virtual		void						release()	PX_OVERRIDE PX_FINAL;
	//~PxBase

	// PxActor
	virtual		PxActorType::Enum			getType() const	PX_OVERRIDE PX_FINAL	{ return PxActorType::eARTICULATION_LINK; }
	//~PxActor

	// PxRigidActor
	virtual		PxTransform					getGlobalPose() const	PX_OVERRIDE PX_FINAL;
	virtual		void 						setGlobalPose(const PxTransform& /*pose*/, bool /*wake*/) PX_OVERRIDE PX_FINAL	{ /*return false; */}
	virtual	    bool						attachShape(PxShape& shape)	PX_OVERRIDE PX_FINAL;
	virtual     void						detachShape(PxShape& shape, bool wakeOnLostTouch = true)	PX_OVERRIDE PX_FINAL;
	//~PxRigidActor

	// PxRigidBody
	virtual		void						setCMassLocalPose(const PxTransform&)	PX_OVERRIDE PX_FINAL;
	virtual		PxVec3						getLinearAcceleration()		const PX_OVERRIDE PX_FINAL;
	virtual		PxVec3						getAngularAcceleration()	const PX_OVERRIDE PX_FINAL;
	virtual		void						addForce(const PxVec3& force, PxForceMode::Enum mode = PxForceMode::eFORCE, bool autowake = true)	PX_OVERRIDE PX_FINAL;
	virtual		void						addTorque(const PxVec3& torque, PxForceMode::Enum mode = PxForceMode::eFORCE, bool autowake = true)	PX_OVERRIDE PX_FINAL;
	virtual		void						clearForce(PxForceMode::Enum mode = PxForceMode::eFORCE)	PX_OVERRIDE PX_FINAL;
	virtual		void						clearTorque(PxForceMode::Enum mode = PxForceMode::eFORCE)	PX_OVERRIDE PX_FINAL;
	virtual		void						setForceAndTorque(const PxVec3& force, const PxVec3& torque, PxForceMode::Enum mode = PxForceMode::eFORCE)	PX_OVERRIDE PX_FINAL;
	//~PxRigidBody

	// PxArticulationLink
	virtual		PxArticulationReducedCoordinate&		getArticulation() const	PX_OVERRIDE PX_FINAL;
	virtual		PxArticulationJointReducedCoordinate*	getInboundJoint() const	PX_OVERRIDE PX_FINAL;
	virtual		PxU32									getInboundJointDof() const	PX_OVERRIDE PX_FINAL;
	virtual		PxU32									getNbChildren() const	PX_OVERRIDE PX_FINAL;
	virtual		PxU32									getLinkIndex() const	PX_OVERRIDE PX_FINAL;
	virtual		PxU32									getChildren(PxArticulationLink** userBuffer, PxU32 bufferSize, PxU32 startIndex) const	PX_OVERRIDE PX_FINAL;
	virtual		void									setCfmScale(const PxReal cfmScale)	PX_OVERRIDE PX_FINAL;
	virtual		PxReal									getCfmScale() const	PX_OVERRIDE PX_FINAL;
	//~PxArticulationLink

				void						releaseInternal();

	PX_INLINE	PxArticulationReducedCoordinate&	getRoot()			{ return *mRoot; }
	PX_INLINE	NpArticulationLink*					getParent()			{ return mParent; }
	PX_INLINE	const NpArticulationLink*			getParent()	const	{ return mParent; }

	PX_INLINE	void						setInboundJoint(PxArticulationJointReducedCoordinate& joint)
											{
												mInboundJoint = &joint;
												OMNI_PVD_SET(OMNI_PVD_CONTEXT_HANDLE, PxArticulationLink, inboundJoint, *this, mInboundJoint);
											}
			
				void 						setGlobalPoseInternal(const PxTransform& pose, bool autowake);
				void						setLLIndex(const PxU32 index) { mLLIndex = index; }
				void						setInboundJointDof(const PxU32 index);
	static PX_FORCE_INLINE size_t			getCoreOffset() { return PX_OFFSET_OF_RT(NpArticulationLink, mCore); }
private:
	PX_INLINE	void						addToChildList(NpArticulationLink& link) { mChildLinks.pushBack(&link); }
	PX_INLINE	void						removeFromChildList(NpArticulationLink& link) { PX_ASSERT(mChildLinks.find(&link) != mChildLinks.end()); mChildLinks.findAndReplaceWithLast(&link); }

public:
	PX_INLINE	NpArticulationLink* const*	getChildren() { return mChildLinks.empty() ? NULL : &mChildLinks.front(); }
				void						setFixedBaseLink(bool value);

#if PX_ENABLE_DEBUG_VISUALIZATION
				void						visualize(PxRenderOutput& out, NpScene& scene, float scale)	const;
				void						visualizeJoint(PxConstraintVisualizer& jointViz)			const;
#else
				PX_CATCH_UNDEFINED_ENABLE_DEBUG_VISUALIZATION
#endif

private:
				PxArticulationReducedCoordinate*		mRoot;  //!!!AL TODO: Revisit: Could probably be avoided if registration and deregistration in root is handled differently
				PxArticulationJointReducedCoordinate*	mInboundJoint;
				NpArticulationLink*						mParent;  //!!!AL TODO: Revisit: Some memory waste but makes things faster
				NpArticulationLinkArray					mChildLinks;
				PxU32									mLLIndex;
				PxU32									mInboundJointDof;
};

}

#endif
