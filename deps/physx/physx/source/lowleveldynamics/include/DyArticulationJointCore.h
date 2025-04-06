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

#ifndef DY_ARTICULATION_JOINT_CORE_H
#define DY_ARTICULATION_JOINT_CORE_H

#include "DyArticulationCore.h"
#include "solver/PxSolverDefs.h"
#include "PxArticulationJointReducedCoordinate.h"
#include "CmSpatialVector.h"

namespace physx
{
	namespace Dy
	{
		// PT: avoid some multiplies when immediately normalizing a rotated vector
		PX_CUDA_CALLABLE PX_FORCE_INLINE PxVec3 rotateAndNormalize(const PxQuat& q, const PxVec3& v)
		{
			const float vx = v.x;
			const float vy = v.y;
			const float vz = v.z;

			const float x = q.x;
			const float y = q.y;
			const float z = q.z;
			const float w = q.w;

			const float w2 = w * w - 0.5f;
			const float dot2 = (x * vx + y * vy + z * vz);
			const PxVec3 rotated(	(vx * w2 + (y * vz - z * vy) * w + x * dot2),
									(vy * w2 + (z * vx - x * vz) * w + y * dot2),
									(vz * w2 + (x * vy - y * vx) * w + z * dot2));

			return rotated.getNormalized();
		}

		class ArticulationJointCoreData;

		PX_ALIGN_PREFIX(16)
		struct ArticulationJointCore
		{
		public:

			// PX_SERIALIZATION
			ArticulationJointCore(const PxEMPTY&) : jCalcUpdateFrames(false) 
			{ 
				PX_COMPILE_TIME_ASSERT(sizeof(PxArticulationMotions) == sizeof(PxU8)); 
			}
			//~PX_SERIALIZATION

			ArticulationJointCore(const PxTransform& parentFrame, const PxTransform& childFrame)
			{
				//PxMarkSerializedMemory(this, sizeof(ArticulationJointCore));
				init(parentFrame, childFrame);
			}

			// PT: these ones don't update the dirty flags
			PX_FORCE_INLINE	void	setLimit(PxArticulationAxis::Enum axis, const PxArticulationLimit& limit)	{ limits[axis] = limit;					}
			PX_FORCE_INLINE	void	setDrive(PxArticulationAxis::Enum axis, const PxArticulationDrive& drive)	{ drives[axis] = drive;					}
			PX_FORCE_INLINE	void	setJointType(PxArticulationJointType::Enum type)							{ jointType = PxU8(type);				}
			PX_FORCE_INLINE	void	setMaxJointVelocity(const PxReal maxJointV)								{ 
				for(PxU32 i = 0; i < PxArticulationAxis::eCOUNT; i++)
				{
					maxJointVelocity[i] = maxJointV;
				}
			}
			PX_FORCE_INLINE	void	setMaxJointVelocity(PxArticulationAxis::Enum axis, const PxReal maxJointV)								{ 
				maxJointVelocity[axis] = maxJointV;
			}
			PX_FORCE_INLINE	void	setFrictionCoefficient(const PxReal coefficient)							{ frictionCoefficient = coefficient;	}

			PX_FORCE_INLINE void setFrictionParams(PxArticulationAxis::Enum axis, const PxJointFrictionParams& jointFrictionParams) 
			{ 
				frictionParams[axis] = jointFrictionParams;
			}

			void	init(const PxTransform& parentFrame, const PxTransform& childFrame)
			{
				PX_ASSERT(parentFrame.isValid());
				PX_ASSERT(childFrame.isValid());

				parentPose			= parentFrame;
				childPose			= childFrame;
				jointOffset			= 0;
				jCalcUpdateFrames		= true;

				setFrictionCoefficient(0.05f);
				setMaxJointVelocity(100.0f);
				setJointType(PxArticulationJointType::eUNDEFINED);

				for(PxU32 i=0; i<PxArticulationAxis::eCOUNT; i++)
				{
					setLimit(PxArticulationAxis::Enum(i), PxArticulationLimit(0.0f, 0.0f));
					setDrive(PxArticulationAxis::Enum(i), PxArticulationDrive(0.0f, 0.0f, 0.0f, PxArticulationDriveType::eNONE));
					setFrictionParams(PxArticulationAxis::Enum(i), PxJointFrictionParams(0.0f, 0.0f, 0.0f));
					setFrictionParams(PxArticulationAxis::Enum(i), PxJointFrictionParams(0.0f, 0.0f, 0.0f));
					targetP[i] = 0.0f;
					targetV[i] = 0.0f;
					armature[i] = 0.0f;
					jointPos[i] = 0.0f;
					jointVel[i] = 0.0f;

					dofIds[i] = 0xff;
					invDofIds[i] = 0xff;
					motion[i] = PxArticulationMotion::eLOCKED;
				}
			}
			
			PX_CUDA_CALLABLE void setJointFrame(Cm::UnAlignedSpatialVector* motionMatrix, 
                                                const Cm::UnAlignedSpatialVector* jointAxis,
                                                PxQuat& relativeQuat,
												const PxU32 dofs)
			{
				if (jCalcUpdateFrames)
				{
					relativeQuat = (childPose.q * (parentPose.q.getConjugate())).getNormalized();

					computeMotionMatrix(motionMatrix, jointAxis, dofs);

					jCalcUpdateFrames = false;

				}
			}

			PX_CUDA_CALLABLE PX_FORCE_INLINE void computeMotionMatrix(Cm::UnAlignedSpatialVector* motionMatrix,
																	  const Cm::UnAlignedSpatialVector* jointAxis,
																	  const PxU32 dofs)
			{
				const PxVec3 childOffset = -childPose.p;

				switch (jointType)
				{
				case PxArticulationJointType::ePRISMATIC:
				{
					const Cm::UnAlignedSpatialVector& jJointAxis = jointAxis[0];
					const PxVec3 u = rotateAndNormalize(childPose.q, jJointAxis.bottom);

					motionMatrix[0] = Cm::UnAlignedSpatialVector(PxVec3(0.0f), u);

					PX_ASSERT(dofs == 1);

					break;
				}
				case PxArticulationJointType::eREVOLUTE:
				case PxArticulationJointType::eREVOLUTE_UNWRAPPED:
				{
					const Cm::UnAlignedSpatialVector& jJointAxis = jointAxis[0];
					const PxVec3 u = rotateAndNormalize(childPose.q, jJointAxis.top);
					const PxVec3 uXd = u.cross(childOffset);

					motionMatrix[0] = Cm::UnAlignedSpatialVector(u, uXd);

					break;
				}
				case PxArticulationJointType::eSPHERICAL:
				{

					for (PxU32 ind = 0; ind < dofs; ++ind)
					{
						const Cm::UnAlignedSpatialVector& jJointAxis = jointAxis[ind];
						const PxVec3 u = rotateAndNormalize(childPose.q, jJointAxis.top);

						const PxVec3 uXd = u.cross(childOffset);
						motionMatrix[ind] = Cm::UnAlignedSpatialVector(u, uXd);
					}

					break;
				}
				case PxArticulationJointType::eFIX:
				{
					PX_ASSERT(dofs == 0);
					break;
				}
				default:
					break;
				}
			}

			PX_CUDA_CALLABLE PX_FORCE_INLINE void operator=(ArticulationJointCore& other)
			{
				parentPose = other.parentPose;
				childPose = other.childPose;

				//KS - temp place to put reduced coordinate limit and drive values
				for(PxU32 i=0; i<PxArticulationAxis::eCOUNT; i++)
				{
					limits[i] = other.limits[i];
					drives[i] = other.drives[i];
					targetP[i] = other.targetP[i];
					targetV[i] = other.targetV[i];
					armature[i] = other.armature[i];
					frictionParams[i] = other.frictionParams[i];
					maxJointVelocity[i] = other.maxJointVelocity[i];

					jointPos[i] = other.jointPos[i];
					jointVel[i] = other.jointVel[i];

					dofIds[i] = other.dofIds[i];
					invDofIds[i] = other.invDofIds[i];
					motion[i] = other.motion[i];
				}

				frictionCoefficient = other.frictionCoefficient;
				jointOffset = other.jointOffset;
				jCalcUpdateFrames = other.jCalcUpdateFrames;
				jointType = other.jointType;
			}

			PX_FORCE_INLINE	void	setParentPose(const PxTransform& t)										{ parentPose = t;			jCalcUpdateFrames = true;				}
			PX_FORCE_INLINE	void	setChildPose(const PxTransform& t)										{ childPose = t;			jCalcUpdateFrames = true;				}
			PX_FORCE_INLINE	void	setMotion(PxArticulationAxis::Enum axis, PxArticulationMotion::Enum m)	{ motion[axis] = PxU8(m);}
			PX_FORCE_INLINE	void	setTargetP(PxArticulationAxis::Enum axis, PxReal value)					{ targetP[axis] = value; }
			PX_FORCE_INLINE	void	setTargetV(PxArticulationAxis::Enum axis, PxReal value)					{ targetV[axis] = value; }
			PX_FORCE_INLINE	void	setArmature(PxArticulationAxis::Enum axis, PxReal value)				{ armature[axis] = value;}

			// attachment points, don't change the order, otherwise it will break GPU code
			PxTransform						parentPose;								//28		28
			PxTransform						childPose;								//28		56

			//KS - temp place to put reduced coordinate limit and drive values
			PxArticulationLimit				limits[PxArticulationAxis::eCOUNT];		//48		104
			PxArticulationDrive				drives[PxArticulationAxis::eCOUNT];		//96		200
			PxReal							targetP[PxArticulationAxis::eCOUNT];	//24		224
			PxReal							targetV[PxArticulationAxis::eCOUNT];	//24		248
			PxReal							armature[PxArticulationAxis::eCOUNT];	//24		272
			
			PxReal							jointPos[PxArticulationAxis::eCOUNT];	//24		296	
			PxReal							jointVel[PxArticulationAxis::eCOUNT];	//24		320
			
			PxReal							frictionCoefficient;					//4			324
			PxJointFrictionParams			frictionParams[PxArticulationAxis::eCOUNT]; //72	396
			PxReal							maxJointVelocity[PxArticulationAxis::eCOUNT]; // 24			420

			//this is the dof offset for the joint in the cache. 
			PxU32							jointOffset;							//4			424

			PxU8							dofIds[PxArticulationAxis::eCOUNT];		//6			430
			PxU8							motion[PxArticulationAxis::eCOUNT];		//6			436
			PxU8							invDofIds[PxArticulationAxis::eCOUNT];	//6			442

			bool							jCalcUpdateFrames;						//1			443
			PxU8							jointType;								//1			444
			PxReal padding[1];														//4		````448
		}PX_ALIGN_SUFFIX(16);
	}
}

#endif
