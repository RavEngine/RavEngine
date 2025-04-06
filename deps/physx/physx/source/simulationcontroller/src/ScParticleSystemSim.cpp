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

#include "foundation/PxPreprocessor.h"

#if PX_SUPPORT_GPU_PHYSX

#include "ScParticleSystemSim.h"

using namespace physx;
using namespace Dy;

Sc::ParticleSystemSim::ParticleSystemSim(ParticleSystemCore& core, Scene& scene) :
	GPUActorSim(scene, core, &core.getShapeCore())
{
	createLowLevelVolume();

	mLLParticleSystem = scene.createLLParticleSystem(this);

	mNodeIndex = scene.getSimpleIslandManager()->addNode(false, false, IG::Node::ePARTICLESYSTEM_TYPE, mLLParticleSystem);

	scene.getSimpleIslandManager()->activateNode(mNodeIndex);

	//mCore.setSim(this);

	mLLParticleSystem->setElementId(mShapeSim.getElementID());

	PxParticleSystemGeometry geometry;
	geometry.mSolverType = PxParticleSolverType::ePBD;

	core.getShapeCore().setGeometry(geometry);
	
	PxsShapeCore* shapeCore = const_cast<PxsShapeCore*>(&core.getShapeCore().getCore());
	mLLParticleSystem->setShapeCore(shapeCore);
}

Sc::ParticleSystemSim::~ParticleSystemSim()
{
	if (!mLLParticleSystem)
		return;

	mScene.destroyLLParticleSystem(*mLLParticleSystem);

	mScene.getSimpleIslandManager()->removeNode(mNodeIndex);

	mCore.setSim(NULL);
}

void Sc::ParticleSystemSim::createLowLevelVolume()
{
	//PX_ASSERT(getWorldBounds().isFinite());

	const PxU32 index = mShapeSim.getElementID();

	if (!(static_cast<Sc::ParticleSystemSim&>(mShapeSim.getActor()).getCore().getFlags() & PxParticleFlag::eDISABLE_RIGID_COLLISION))
	{
		mScene.getBoundsArray().setBounds(PxBounds3(PxVec3(PX_MAX_BOUNDS_EXTENTS), PxVec3(-PX_MAX_BOUNDS_EXTENTS)), index);
		mShapeSim.setInBroadPhase();
	}
	else
		mScene.getAABBManager()->reserveSpaceForBounds(index);

	addToAABBMgr(Bp::FilterType::PARTICLESYSTEM);
}

bool Sc::ParticleSystemSim::isSleeping() const
{
	return false;
}

void Sc::ParticleSystemSim::sleepCheck(PxReal dt)
{
	PX_UNUSED(dt);
}

/*void Sc::ParticleSystemSim::activate()
{
	activateInteractions(*this);
}

void Sc::ParticleSystemSim::deactivate()
{
	deactivateInteractions(*this);
}*/

#endif //PX_SUPPORT_GPU_PHYSX
