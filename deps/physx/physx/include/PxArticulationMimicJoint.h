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


#ifndef PX_ARTICULATION_MIMIC_JOINT_H
#define PX_ARTICULATION_MIMIC_JOINT_H

#include "foundation/PxSimpleTypes.h"
#include "solver/PxSolverDefs.h"
#include "common/PxBase.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

class PxArticulationReducedCoordinate;
class PxArticulationJointReducedCoordinate;

/**
* \brief A mimic joint enforces a linear relationship between the positions of two joints of the same articulation instance.
\see PxArticulationReducedCoodinate::createMimicJoint()
*/
class PxArticulationMimicJoint : public PxBase
{
public:

	/**
	\brief Releases the mimic joint.

	\note Releasing a mimic joint is not allowed while the articulation is in a scene. In order to
	release a mimic joint, remove and then re-add the articulation to the scene.
	*/
	virtual void release() = 0;

	/**
	\brief Returns the articulation that this mimic joint is part of.

	\return A reference to the articulation.
	*/
	virtual PxArticulationReducedCoordinate& getArticulation() const = 0;

	/**
	\brief Get the gear of a mimic joint.
	\return The gear ratio.
	*/
	virtual PxReal getGearRatio() const = 0;

	/**
	\brief Set the gear ratio of a mimic joint.
	\param[in] gearRatio is the new gear ratio to be used in the next simulation step.
	*/
	virtual void setGearRatio(PxReal gearRatio) = 0;

	/**
	\brief Get the offset of a mimic joint.
	\return The offset.
	*/
	virtual PxReal getOffset() const = 0;

	/**
	\brief Set the offset of a mimic joint.
	\param[in] offset is the new offset to be used in the next simulation step.
	*/
	virtual void setOffset(PxReal offset) = 0;

	/**
	\brief Get the natural frequency of a mimic joint.
	\return The natural frequency.
	*/
	virtual PxReal getNaturalFrequency() const = 0;

	/**
	\brief Set the natural frequency of a mimic joint.
	\param[in] naturalFrequency is the new natural frequency to be used in the next simulation step.
	*/
	virtual void setNaturalFrequency(PxReal naturalFrequency) = 0;

	/**
	\brief Get the damping ratio of a mimic joint.
	\return The damping ratio.
	*/
	virtual PxReal getDampingRatio() const = 0;

	/**
	\brief Set the damping ratio of a mimic joint.
	\param[in] dampingRatio is the new damping ratio to be used in the next simulation step.
	*/
	virtual void setDampingRatio(PxReal dampingRatio) = 0;


	/**
	\brief Return the jointA specified in PxArticulationReducedCoordinate::createMimicJoint()
	\return The jointA specified in PxArticulationReducedCoordinate::createMimicJoint()
	\see  PxArticulationReducedCoordinate::createMimicJoint()
	*/
	virtual PxArticulationJointReducedCoordinate& getJointA() const = 0;
	
	/**
	\brief Return the jointB specified in PxArticulationReducedCoordinate::createMimicJoint()
	\return The jointB specified in PxArticulationReducedCoordinate::createMimicJoint()
	\see  PxArticulationReducedCoordinate::createMimicJoint()
	*/
	virtual PxArticulationJointReducedCoordinate& getJointB() const = 0;

	/**
	\brief Return the axisA specified in PxArticulationReducedCoordinate::createMimicJoint()
	\return The axisA specified in PxArticulationReducedCoordinate::createMimicJoint()
	\see  PxArticulationReducedCoordinate::createMimicJoint()
	*/

	virtual PxArticulationAxis::Enum getAxisA() const = 0;

	/**
	\brief Return the axisB specified in PxArticulationReducedCoordinate::createMimicJoint()
	\return The axisB specified in PxArticulationReducedCoordinate::createMimicJoint()
	\see  PxArticulationReducedCoordinate::createMimicJoint()
	*/
	virtual PxArticulationAxis::Enum getAxisB() const = 0;

	/**
	\brief Returns the string name of the dynamic type.

	\return The string name.
	*/
	virtual	const char*						getConcreteTypeName() const	PX_OVERRIDE	PX_FINAL	{ return "PxArticulationMimicJoint"; }

	virtual									~PxArticulationMimicJoint() {}

			void*							userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.

protected:
	PX_INLINE	PxArticulationMimicJoint(PxType concreteType, PxBaseFlags baseFlags) : PxBase(concreteType, baseFlags) {}
	PX_INLINE	PxArticulationMimicJoint(PxBaseFlags baseFlags) : PxBase(baseFlags) {}
};

#if !PX_DOXYGEN
} // namespace physx
#endif

#endif //PX_ARTICULATION_MIMIC_JOINT_H
