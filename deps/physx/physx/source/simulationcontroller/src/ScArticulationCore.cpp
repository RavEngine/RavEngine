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


#include "ScArticulationCore.h"

#include "PsFoundation.h"
#include "ScPhysics.h"
#include "ScBodyCore.h"
#include "ScBodySim.h"
#include "ScArticulationSim.h"
#include "DyArticulation.h"

using namespace physx;

Sc::ArticulationCore::ArticulationCore(bool reducedCoordinate) :
	mSim(NULL), 
	mIsReducedCoordinate(reducedCoordinate)
{
	const PxTolerancesScale& scale = Physics::getInstance().getTolerancesScale();

	mCore.internalDriveIterations	= 4;
	mCore.externalDriveIterations	= 4;
	mCore.maxProjectionIterations	= 4;
	mCore.solverIterationCounts		= 1<<8 | 4;
	mCore.separationTolerance		= 0.1f * scale.length;
	mCore.sleepThreshold			= 5e-5f * scale.speed * scale.speed;
	mCore.freezeThreshold			= 5e-6f * scale.speed * scale.speed;
	mCore.wakeCounter				= Physics::sWakeCounterOnCreation;
}

Sc::ArticulationCore::~ArticulationCore()
{
}

//--------------------------------------------------------------
//
// ArticulationCore interface implementation
//
//--------------------------------------------------------------

void Sc::ArticulationCore::setWakeCounter(const PxReal v)
{
	mCore.wakeCounter = v;

#ifdef _DEBUG
	if(mSim)
		mSim->debugCheckWakeCounterOfLinks(v);
#endif
}

bool Sc::ArticulationCore::isSleeping() const
{
	return mSim ? mSim->isSleeping() : (mCore.wakeCounter == 0.0f);
}

void Sc::ArticulationCore::wakeUp(PxReal wakeCounter)
{
	mCore.wakeCounter = wakeCounter;

#ifdef _DEBUG
	if(mSim)
		mSim->debugCheckSleepStateOfLinks(false);
#endif
}

void Sc::ArticulationCore::putToSleep()
{
	mCore.wakeCounter = 0.0f;

#ifdef _DEBUG
	if(mSim)
		mSim->debugCheckSleepStateOfLinks(true);
#endif
}

PxArticulationBase* Sc::ArticulationCore::getPxArticulationBase()
{
	return gOffsetTable.convertScArticulation2Px(this, isReducedCoordinate());
}

const PxArticulationBase* Sc::ArticulationCore::getPxArticulationBase() const
{
	return gOffsetTable.convertScArticulation2Px(this, isReducedCoordinate());
}

Sc::ArticulationDriveCache* Sc::ArticulationCore::createDriveCache(PxReal compliance, PxU32 driveIterations) const
{
	return mSim ? mSim->createDriveCache(compliance, driveIterations) : NULL;
}

void Sc::ArticulationCore::updateDriveCache(ArticulationDriveCache& cache, PxReal compliance, PxU32 driveIterations) const
{
	if(mSim)
		mSim->updateDriveCache(cache, compliance, driveIterations);
}

void Sc::ArticulationCore::releaseDriveCache(Sc::ArticulationDriveCache& driveCache) const
{
	if(mSim)
		mSim->releaseDriveCache(driveCache);
}

PxU32 Sc::ArticulationCore::getCacheLinkCount(const ArticulationDriveCache& cache) const
{
	return Dy::PxvArticulationDriveCache::getLinkCount(cache);
}

void Sc::ArticulationCore::applyImpulse(Sc::BodyCore& link,
										const Sc::ArticulationDriveCache& driveCache,
										const PxVec3& force,
										const PxVec3& torque)
{
	if(mSim)
		mSim->applyImpulse(link, driveCache, force, torque);
}

void Sc::ArticulationCore::computeImpulseResponse(Sc::BodyCore& link,
												  PxVec3& linearResponse, 
												  PxVec3& angularResponse,
												  const Sc::ArticulationDriveCache& driveCache,
												  const PxVec3& force,
												  const PxVec3& torque) const
{
	if(mSim)
		mSim->computeImpulseResponse(link, linearResponse, angularResponse, driveCache, force, torque);
}

void Sc::ArticulationCore::setArticulationFlags(PxArticulationFlags flags)
{
	mCore.flags = flags;
	if (mSim)
	{
		const bool isKinematicLink = flags & PxArticulationFlag::eFIX_BASE;
		mSim->setKinematicLink(isKinematicLink);
	}
}

PxU32 Sc::ArticulationCore::getDofs() const
{
	return mSim ? mSim->getDofs() : 0;
}

PxArticulationCache* Sc::ArticulationCore::createCache() const
{
	return mSim ? mSim->createCache() : NULL;
}

PxU32 Sc::ArticulationCore::getCacheDataSize() const
{
	return mSim ? mSim->getCacheDataSize() : 0;
}

void Sc::ArticulationCore::zeroCache(PxArticulationCache& cache) const
{
	if(mSim)
		mSim->zeroCache(cache);
}

void Sc::ArticulationCore::applyCache(PxArticulationCache& cache, const PxArticulationCacheFlags flag) const
{
	if(mSim)
		mSim->applyCache(cache, flag);
}

void Sc::ArticulationCore::copyInternalStateToCache(PxArticulationCache& cache, const PxArticulationCacheFlags flag) const
{
	if(mSim)
		mSim->copyInternalStateToCache(cache, flag);
}

void Sc::ArticulationCore::releaseCache(PxArticulationCache& cache) const
{
	if(mSim)
		mSim->releaseCache(cache);
}

void Sc::ArticulationCore::packJointData(const PxReal* maximum, PxReal* reduced) const
{
	if(mSim)
		mSim->packJointData(maximum, reduced);
}

void Sc::ArticulationCore::unpackJointData(const PxReal* reduced, PxReal* maximum) const
{
	if(mSim)
		mSim->unpackJointData(reduced, maximum);
}

void Sc::ArticulationCore::commonInit() const
{
	if(mSim)
		mSim->commonInit();
}

void Sc::ArticulationCore::computeGeneralizedGravityForce(PxArticulationCache& cache) const
{
	if(mSim)
		mSim->computeGeneralizedGravityForce(cache);
}

void Sc::ArticulationCore::computeCoriolisAndCentrifugalForce(PxArticulationCache& cache) const
{
	if(mSim)
		mSim->computeCoriolisAndCentrifugalForce(cache);
}

void Sc::ArticulationCore::computeGeneralizedExternalForce(PxArticulationCache& cache) const
{
	if(mSim)
		mSim->computeGeneralizedExternalForce(cache);
}

void Sc::ArticulationCore::computeJointAcceleration(PxArticulationCache& cache) const
{
	if(mSim)
		mSim->computeJointAcceleration(cache);
}

void Sc::ArticulationCore::computeJointForce(PxArticulationCache& cache) const
{
	if(mSim)
		mSim->computeJointForce(cache);
}

void Sc::ArticulationCore::computeDenseJacobian(PxArticulationCache& cache, PxU32& nRows, PxU32& nCols) const
{
	if(mSim)
		mSim->computeDenseJacobian(cache, nRows, nCols);
}

void Sc::ArticulationCore::computeCoefficientMatrix(PxArticulationCache& cache) const
{
	if(mSim)
		mSim->computeCoefficientMatrix(cache);
}

bool Sc::ArticulationCore::computeLambda(PxArticulationCache& cache, PxArticulationCache& initialState, const PxReal* const jointTorque, const PxVec3 gravity, const PxU32 maxIter) const
{
	return mSim ? mSim->computeLambda(cache, initialState, jointTorque, gravity, maxIter) : false;
}

void Sc::ArticulationCore::computeGeneralizedMassMatrix(PxArticulationCache& cache) const
{
	if(mSim)
		mSim->computeGeneralizedMassMatrix(cache);
}

PxU32 Sc::ArticulationCore::getCoefficientMatrixSize() const
{
	return mSim ? mSim->getCoefficientMatrixSize() : 0;
}

PxSpatialVelocity Sc::ArticulationCore::getLinkVelocity(const PxU32 linkId) const
{
	return mSim ? mSim->getLinkVelocity(linkId) : PxSpatialVelocity();
}

PxSpatialVelocity Sc::ArticulationCore::getLinkAcceleration(const PxU32 linkId) const
{
	return mSim ? mSim->getLinkAcceleration(linkId) : PxSpatialVelocity();
}

IG::NodeIndex Sc::ArticulationCore::getIslandNodeIndex() const
{
	return mSim ? mSim->getIslandNodeIndex() : IG::NodeIndex(IG_INVALID_NODE);
}

void Sc::ArticulationCore::setGlobalPose()
{
	if(mSim)
		mSim->setGlobalPose();
}

void Sc::ArticulationCore::setDirty(const bool dirty)
{
	if(mSim)
		mSim->setDirty(dirty);
}
