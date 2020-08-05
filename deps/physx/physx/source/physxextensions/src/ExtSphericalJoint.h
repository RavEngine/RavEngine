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

#ifndef NP_SPHERICALJOINTCONSTRAINT_H
#define NP_SPHERICALJOINTCONSTRAINT_H

#include "extensions/PxSphericalJoint.h"

#include "ExtJoint.h"
#include "CmUtils.h"

namespace physx
{
struct PxSphericalJointGeneratedValues;
namespace Ext
{
	struct SphericalJointData: public JointData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
		PxJointLimitCone		limit;

		PxReal					projectionLinearTolerance;

		PxSphericalJointFlags	jointFlags;
		// forestall compiler complaints about not being able to generate a constructor
	private:
		SphericalJointData(const PxJointLimitCone &cone):
			limit(cone) {}
	};
    
    typedef Joint<PxSphericalJoint, PxSphericalJointGeneratedValues> SphericalJointT;
   
	class SphericalJoint : public SphericalJointT
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
									SphericalJoint(PxBaseFlags baseFlags) : SphericalJointT(baseFlags) {}
		virtual		void			exportExtraData(PxSerializationContext& context);
					void			importExtraData(PxDeserializationContext& context);
					void			resolveReferences(PxDeserializationContext& context);
		static		SphericalJoint*	createObject(PxU8*& address, PxDeserializationContext& context);
		static		void			getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

		SphericalJoint(const PxTolerancesScale& /*scale*/, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1) :
			SphericalJointT(PxJointConcreteType::eSPHERICAL, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE, actor0, localFrame0, actor1, localFrame1, sizeof(SphericalJointData), "SphericalJointData")
		{
			SphericalJointData* data = static_cast<SphericalJointData*>(mData);

			data->projectionLinearTolerance	= 1e10f;
			data->limit						= PxJointLimitCone(PxPi/2, PxPi/2);
			data->jointFlags				= PxSphericalJointFlags();
		}

		// PxSphericalJoint
		virtual	void					setLimitCone(const PxJointLimitCone &limit);
		virtual	PxJointLimitCone		getLimitCone() const;
		virtual	void					setSphericalJointFlags(PxSphericalJointFlags flags);
		virtual	void					setSphericalJointFlag(PxSphericalJointFlag::Enum flag, bool value);
		virtual	PxSphericalJointFlags	getSphericalJointFlags(void) const;
		virtual	void					setProjectionLinearTolerance(PxReal distance);
		virtual	PxReal					getProjectionLinearTolerance() const;
		virtual PxReal					getSwingYAngle() const;
		virtual PxReal					getSwingZAngle() const;
		//~PxSphericalJoint

		bool					attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1);
		
		static const PxConstraintShaderTable& getConstraintShaderTable() { return sShaders; }

		virtual PxConstraintSolverPrep getPrep() const { return sShaders.solverPrep; }
		
	private:

		static PxConstraintShaderTable sShaders;

		PX_FORCE_INLINE SphericalJointData& data() const				
		{	
			return *static_cast<SphericalJointData*>(mData);
		}
	};

} // namespace Ext

namespace Ext
{
	// global function to share the joint shaders with API capture	
	extern "C" const PxConstraintShaderTable* GetSphericalJointShaderTable();
}

}

#endif
