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

#ifndef NP_ARTICULATION_JOINT_RC_H
#define NP_ARTICULATION_JOINT_RC_H

#include "PxArticulationJointReducedCoordinate.h"
#include "ScArticulationJointCore.h"
#include "NpArticulationLink.h"
#include "NpBase.h"

#if PX_ENABLE_DEBUG_VISUALIZATION
	#include "common/PxRenderOutput.h"
#else
	PX_CATCH_UNDEFINED_ENABLE_DEBUG_VISUALIZATION
#endif

namespace physx
{
	class NpScene;
	class NpArticulationLink;

	class NpArticulationJointReducedCoordinate : public PxArticulationJointReducedCoordinate, public NpBase
	{
	public:
		// PX_SERIALIZATION
													NpArticulationJointReducedCoordinate(PxBaseFlags baseFlags)
														: PxArticulationJointReducedCoordinate(baseFlags), NpBase(PxEmpty), mCore(PxEmpty) {}
					void							preExportDataReset() { mCore.preExportDataReset(); }

		virtual		void							resolveReferences(PxDeserializationContext& context);
		static		NpArticulationJointReducedCoordinate* createObject(PxU8*& address, PxDeserializationContext& context);
					void							exportExtraData(PxSerializationContext&) {}
					void							importExtraData(PxDeserializationContext&) {}
		virtual		void							requiresObjects(PxProcessPxBaseCallback&) {}
		virtual		bool							isSubordinate()  const { return true; }
		//~PX_SERIALIZATION
													NpArticulationJointReducedCoordinate(NpArticulationLink& parent, const PxTransform& parentFrame, NpArticulationLink& child, const PxTransform& childFrame);
		virtual										~NpArticulationJointReducedCoordinate();

		// PxBase
		virtual		void				release()	PX_OVERRIDE PX_FINAL;
		//~PxBase

		// PxArticulationJointReducedCoordinate
		virtual     PxArticulationLink&	getParentArticulationLink() const PX_OVERRIDE PX_FINAL	{ return *mParent; }
		virtual		void				setParentPose(const PxTransform& pose)	PX_OVERRIDE PX_FINAL;
		virtual		PxTransform			getParentPose() const	PX_OVERRIDE PX_FINAL;
		virtual     PxArticulationLink&	getChildArticulationLink() const	PX_OVERRIDE PX_FINAL	{ return *mChild; }
		virtual		void				setChildPose(const PxTransform& pose)	PX_OVERRIDE PX_FINAL;
		virtual		PxTransform			getChildPose() const	PX_OVERRIDE PX_FINAL;
		virtual		void				setJointType(PxArticulationJointType::Enum jointType)	PX_OVERRIDE PX_FINAL;
		virtual		PxArticulationJointType::Enum	getJointType() const	PX_OVERRIDE PX_FINAL;
		virtual		void				setMotion(PxArticulationAxis::Enum axis, PxArticulationMotion::Enum motion)	PX_OVERRIDE PX_FINAL;
		virtual		PxArticulationMotion::Enum	getMotion(PxArticulationAxis::Enum axis) const	PX_OVERRIDE PX_FINAL;
		virtual		void				setLimitParams(PxArticulationAxis::Enum axis, const PxArticulationLimit& limit)	PX_OVERRIDE PX_FINAL;
		virtual		PxArticulationLimit	getLimitParams(PxArticulationAxis::Enum axis) const	PX_OVERRIDE PX_FINAL;
		virtual		void				setDriveParams(PxArticulationAxis::Enum axis, const PxArticulationDrive& drive)	PX_OVERRIDE PX_FINAL;
		virtual		PxArticulationDrive	getDriveParams(PxArticulationAxis::Enum axis) const	PX_OVERRIDE PX_FINAL;
		virtual		void				setDriveTarget(PxArticulationAxis::Enum axis, const PxReal target, bool autowake = true)	PX_OVERRIDE PX_FINAL;
		virtual		PxReal				getDriveTarget(PxArticulationAxis::Enum axis) const	PX_OVERRIDE PX_FINAL;
		virtual		void				setDriveVelocity(PxArticulationAxis::Enum axis, const PxReal targetVel, bool autowake = true)	PX_OVERRIDE PX_FINAL;
		virtual		PxReal				getDriveVelocity(PxArticulationAxis::Enum axis) const	PX_OVERRIDE PX_FINAL;
		virtual		void				setArmature(PxArticulationAxis::Enum axis, const PxReal armature)	PX_OVERRIDE PX_FINAL;
		virtual		PxReal				getArmature(PxArticulationAxis::Enum axis) const	PX_OVERRIDE PX_FINAL;
		virtual		void				setFrictionCoefficient(const PxReal coefficient)	PX_OVERRIDE PX_FINAL;
		virtual		PxReal				getFrictionCoefficient() const	PX_OVERRIDE PX_FINAL;
		virtual		void				setMaxJointVelocity(const PxReal maxJointV)	PX_OVERRIDE PX_FINAL;
		virtual		PxReal				getMaxJointVelocity() const	PX_OVERRIDE PX_FINAL;
		virtual		void				setMaxJointVelocity(PxArticulationAxis::Enum axis, const PxReal maxJointV)	PX_OVERRIDE PX_FINAL;
		virtual		PxReal				getMaxJointVelocity(PxArticulationAxis::Enum axis) const	PX_OVERRIDE PX_FINAL;
		virtual		void				setFrictionParams(PxArticulationAxis::Enum axis, const PxJointFrictionParams& jointFrictionParams) PX_OVERRIDE PX_FINAL;
		virtual		PxJointFrictionParams	getFrictionParams(PxArticulationAxis::Enum axis) const PX_OVERRIDE PX_FINAL;
		virtual		void				setJointPosition(PxArticulationAxis::Enum axis, const PxReal jointPos)	PX_OVERRIDE PX_FINAL;
		virtual		PxReal				getJointPosition(PxArticulationAxis::Enum axis) const	PX_OVERRIDE PX_FINAL;
		virtual		void				setJointVelocity(PxArticulationAxis::Enum axis, const PxReal jointVel)	PX_OVERRIDE PX_FINAL;
		virtual		PxReal				getJointVelocity(PxArticulationAxis::Enum axis) const	PX_OVERRIDE PX_FINAL;
		virtual		void				setName(const char* name)	PX_OVERRIDE	PX_FINAL;
		virtual		const char*			getName() const	PX_OVERRIDE	PX_FINAL;
		//~PxArticulationJointReducedCoordinate

		PX_FORCE_INLINE	Sc::ArticulationJointCore&	getCore()		{ return mCore; }
		static PX_FORCE_INLINE size_t				getCoreOffset()	{ return PX_OFFSET_OF_RT(NpArticulationJointReducedCoordinate, mCore); }

		PX_INLINE void						scSetParentPose(const PxTransform& v)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setParentPose(v);
			UPDATE_PVD_PROPERTY
		}

		PX_INLINE void						scSetChildPose(const PxTransform& v)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setChildPose(v);
			UPDATE_PVD_PROPERTY
		}

		PX_INLINE void						scSetJointType(PxArticulationJointType::Enum v)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setJointType(v);
			UPDATE_PVD_PROPERTY
		}
		PX_INLINE void						scSetFrictionCoefficient(const PxReal v)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setFrictionCoefficient(v);
			UPDATE_PVD_PROPERTY
		}
		PX_INLINE void scSetFrictionParams(PxArticulationAxis::Enum axis, const PxJointFrictionParams& jointFrictionParams)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setFrictionParams(axis, jointFrictionParams);
			UPDATE_PVD_PROPERTY
		}
		PX_INLINE void						scSetMaxJointVelocity(const PxReal v)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setMaxJointVelocity(v);
			UPDATE_PVD_PROPERTY
		}
		PX_INLINE void						scSetMaxJointVelocity(PxArticulationAxis::Enum axis, const PxReal v)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setMaxJointVelocity(axis, v);
			UPDATE_PVD_PROPERTY
		}

		PX_INLINE void						scSetLimit(PxArticulationAxis::Enum axis, const PxArticulationLimit& pair)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setLimit(axis, pair);
			UPDATE_PVD_PROPERTY
		}

		PX_INLINE void						scSetDrive(PxArticulationAxis::Enum axis, const PxArticulationDrive& drive)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setDrive(axis, drive);
			UPDATE_PVD_PROPERTY
		}

		PX_INLINE void						scSetDriveTarget(PxArticulationAxis::Enum axis, PxReal targetP)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setTargetP(axis, targetP);
			UPDATE_PVD_PROPERTY
		}

		PX_INLINE void						scSetDriveVelocity(PxArticulationAxis::Enum axis, PxReal targetP)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setTargetV(axis, targetP);
			UPDATE_PVD_PROPERTY
		}

		PX_INLINE void						scSetArmature(PxArticulationAxis::Enum axis, PxReal armature)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setArmature(axis, armature);
			UPDATE_PVD_PROPERTY
		}

		PX_INLINE void						scSetMotion(PxArticulationAxis::Enum axis, PxArticulationMotion::Enum motion)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setMotion(axis, motion);
			UPDATE_PVD_PROPERTY
		}

		PX_INLINE void						scSetJointPosition(PxArticulationAxis::Enum axis, const PxReal jointPos)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setJointPosition(axis, jointPos);
			UPDATE_PVD_PROPERTY
		}

		PX_INLINE void						scSetJointVelocity(PxArticulationAxis::Enum axis, const PxReal jointVel)
		{
			PX_ASSERT(!isAPIWriteForbidden());
			mCore.setJointVelocity(axis, jointVel);
			UPDATE_PVD_PROPERTY
		}

		PX_INLINE	const NpArticulationLink&		getParent() const { return *mParent; }
		PX_INLINE	NpArticulationLink&				getParent() { return *mParent; }

		PX_INLINE	const NpArticulationLink&		getChild() const { return *mChild; }
		PX_INLINE	NpArticulationLink&				getChild() { return *mChild; }

		Sc::ArticulationJointCore					mCore;
		NpArticulationLink*							mParent;
		NpArticulationLink*							mChild;
		const char*									mName;
#if PX_CHECKED
	private:
					bool							isValidMotion(PxArticulationAxis::Enum axis, PxArticulationMotion::Enum motion);
#endif
	};
}

#endif
