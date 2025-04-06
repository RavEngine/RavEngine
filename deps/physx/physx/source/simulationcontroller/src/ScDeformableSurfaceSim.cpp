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

#include "ScDeformableSurfaceSim.h"

#include "geometry/PxTriangleMesh.h"

using namespace physx;
using namespace Dy;

Sc::DeformableSurfaceSim::DeformableSurfaceSim(DeformableSurfaceCore& core, Scene& scene) :
	GPUActorSim(scene, core, NULL)
{
	mLLDeformableSurface = scene.createLLDeformableSurface(this);

	mNodeIndex = scene.getSimpleIslandManager()->addNode(false, false, IG::Node::eDEFORMABLE_SURFACE_TYPE, mLLDeformableSurface);

	scene.getSimpleIslandManager()->activateNode(mNodeIndex);

	mLLDeformableSurface->setElementId(mShapeSim.getElementID());
}

Sc::DeformableSurfaceSim::~DeformableSurfaceSim()
{
	if (!mLLDeformableSurface)
		return;

	mScene.destroyLLDeformableSurface(*mLLDeformableSurface);

	mScene.getSimpleIslandManager()->removeNode(mNodeIndex);

	mCore.setSim(NULL);
}

bool Sc::DeformableSurfaceSim::isSleeping() const
{
	IG::IslandSim& sim = mScene.getSimpleIslandManager()->getAccurateIslandSim();
	return sim.getActiveNodeIndex(mNodeIndex) == PX_INVALID_NODE;
}

void Sc::DeformableSurfaceSim::onSetWakeCounter()
{
	mScene.getSimulationController()->setClothWakeCounter(mLLDeformableSurface);
	if (mLLDeformableSurface->getCore().wakeCounter > 0.f)
		mScene.getSimpleIslandManager()->activateNode(mNodeIndex);
	else
		mScene.getSimpleIslandManager()->deactivateNode(mNodeIndex);
}

void Sc::DeformableSurfaceSim::attachShapeCore(ShapeCore* core)
{
	mShapeSim.setCore(core);
	{
		PX_ASSERT(getWorldBounds().isFinite());

		const PxU32 index = mShapeSim.getElementID();

		mScene.getBoundsArray().setBounds(getWorldBounds(), index);

		addToAABBMgr(Bp::FilterType::DEFORMABLE_SURFACE);
	}

	PxsShapeCore* shapeCore = const_cast<PxsShapeCore*>(&core->getCore());
	mLLDeformableSurface->setShapeCore(shapeCore);
}

PxBounds3 Sc::DeformableSurfaceSim::getWorldBounds() const
{
	const PxTriangleMeshGeometry& triGeom = static_cast<const PxTriangleMeshGeometry&>(mShapeSim.getCore().getGeometry());

	// PT: are you sure you want to go through the Px API here?
	return triGeom.triangleMesh->getLocalBounds();
}

#endif //PX_SUPPORT_GPU_PHYSX
