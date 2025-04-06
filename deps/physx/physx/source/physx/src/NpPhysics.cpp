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

#include "NpPhysics.h"

// PX_SERIALIZATION
#include "foundation/PxProfiler.h"
#include "foundation/PxIO.h"
#include "foundation/PxErrorCallback.h"
#include "foundation/PxString.h"
#include "foundation/PxPhysicsVersion.h"
#include "common/PxTolerancesScale.h"
#include "CmCollection.h"
#include "CmUtils.h"
#include "NpRigidStatic.h"
#include "NpRigidDynamic.h"
#include "omnipvd/OmniPvdPxSampler.h"
#if PX_SUPPORT_GPU_PHYSX
#include "NpDeformableVolume.h"
#include "NpPBDParticleSystem.h"
#endif
#include "NpArticulationReducedCoordinate.h"
#include "NpArticulationLink.h"
#include "NpMaterial.h"
#include "GuHeightFieldData.h"
#include "GuHeightField.h"
#include "GuConvexMesh.h"
#include "GuTriangleMesh.h"
#include "foundation/PxIntrinsics.h"
#include "PxvGlobals.h"		// dynamic registration of HFs & articulations in LL
#include "GuOverlapTests.h" // dynamic registration of HFs in Gu
#include "PxDeletionListener.h"
#include "PxPhysicsSerialization.h"
#include "PvdPhysicsClient.h"
#include "omnipvd/NpOmniPvdSetData.h"
#include "cudamanager/PxCudaContext.h"
#include "common/PxProfileZone.h"

#if PX_SUPPORT_OMNI_PVD
#include "omnipvd/NpOmniPvd.h"
#include "OmniPvdWriter.h"
#endif

#if PX_SUPPORT_GPU_PHYSX
#include "PxPhysXGpu.h"
#endif

//~PX_SERIALIZATION

#include "NpFactory.h"

#if PX_SWITCH
#include "switch/NpMiddlewareInfo.h"
#endif

#if PX_SUPPORT_OMNI_PVD
#	define OMNI_PVD_NOTIFY_ADD(OBJECT) mOmniPvdListener.onObjectAdd(OBJECT)
#	define OMNI_PVD_NOTIFY_REMOVE(OBJECT) mOmniPvdListener.onObjectRemove(OBJECT)
#else
#	define OMNI_PVD_NOTIFY_ADD(OBJECT)
#	define OMNI_PVD_NOTIFY_REMOVE(OBJECT)
#endif

using namespace physx;
using namespace Cm;

bool		NpPhysics::apiReentryLock	= false;
NpPhysics*	NpPhysics::mInstance		= NULL;
PxU32		NpPhysics::mRefCount		= 0;

NpPhysics::NpPhysics(const PxTolerancesScale& scale, const PxvOffsetTable& pxvOffsetTable, bool trackOutstandingAllocations, pvdsdk::PsPvd* pvd, PxFoundation& foundation, PxOmniPvd* omniPvd) :
	mSceneArray					("physicsSceneArray"),
	mPhysics					(scale, pxvOffsetTable),
	mDeletionListenersExist		(false),
	mFoundation					(foundation)
{
	PX_UNUSED(trackOutstandingAllocations);

	//mMasterMaterialTable.reserve(10);
		
#if PX_SUPPORT_PVD	
	mPvd = pvd;
	if(pvd)
	{		
	    mPvdPhysicsClient = PX_NEW(Vd::PvdPhysicsClient)(mPvd);	
	    foundation.registerErrorCallback(*mPvdPhysicsClient);
		foundation.registerAllocationListener(*mPvd);	
	}
	else
	{		
		mPvdPhysicsClient = NULL;
	}
#else
	PX_UNUSED(pvd);
#endif

	// PT: please leave this commented-out block here.
/*
	printf("sizeof(NpScene)          = %d\n", sizeof(NpScene));
	printf("sizeof(NpShape)          = %d\n", sizeof(NpShape));
	printf("sizeof(NpActor)          = %d\n", sizeof(NpActor));
	printf("sizeof(NpRigidStatic)    = %d\n", sizeof(NpRigidStatic));
	printf("sizeof(NpRigidDynamic)   = %d\n", sizeof(NpRigidDynamic));
	printf("sizeof(NpMaterial)       = %d\n", sizeof(NpMaterial));
	printf("sizeof(NpConstraint)     = %d\n", sizeof(NpConstraint));
	printf("sizeof(NpAggregate)      = %d\n", sizeof(NpAggregate));
	printf("sizeof(NpArticulationRC) = %d\n", sizeof(NpArticulationReducedCoordinate));

	printf("sizeof(GeometryUnion)            = %d\n", sizeof(GeometryUnion));
	printf("sizeof(PxGeometry)               = %d\n", sizeof(PxGeometry));
	printf("sizeof(PxPlaneGeometry)          = %d\n", sizeof(PxPlaneGeometry));
	printf("sizeof(PxSphereGeometry)         = %d\n", sizeof(PxSphereGeometry));
	printf("sizeof(PxCapsuleGeometry)        = %d\n", sizeof(PxCapsuleGeometry));
	printf("sizeof(PxBoxGeometry)            = %d\n", sizeof(PxBoxGeometry));
	printf("sizeof(PxConvexMeshGeometry)     = %d\n", sizeof(PxConvexMeshGeometry));
	printf("sizeof(PxConvexMeshGeometryLL)   = %d\n", sizeof(PxConvexMeshGeometryLL));
	printf("sizeof(PxTriangleMeshGeometry)   = %d\n", sizeof(PxTriangleMeshGeometry));
	printf("sizeof(PxTriangleMeshGeometryLL) = %d\n", sizeof(PxTriangleMeshGeometryLL));
	printf("sizeof(PxsShapeCore)             = %d\n", sizeof(PxsShapeCore));
*/

#if PX_SUPPORT_OMNI_PVD
	mOmniPvdSampler = NULL;
	mOmniPvd = NULL;
	if (omniPvd)
	{
		OmniPvdWriter* omniWriter = omniPvd->getWriter();
		if (omniWriter && omniWriter->getWriteStream())
		{
			mOmniPvdSampler = PX_NEW(::OmniPvdPxSampler)();
			mOmniPvd = omniPvd;
			NpOmniPvd* npOmniPvd = static_cast<NpOmniPvd*>(mOmniPvd);
			NpOmniPvd::incRefCount();
			npOmniPvd->mPhysXSampler = mOmniPvdSampler; // Dirty hack to do startSampling from PxOmniPvd
			mOmniPvdSampler->setOmniPvdInstance(npOmniPvd);

			foundation.registerErrorCallback(*mOmniPvdSampler);
		}
	}
#else
	PX_UNUSED(omniPvd);
#endif
}

NpPhysics::~NpPhysics()
{
	// Release all scenes in case the user didn't do it
	PxU32 nbScenes = mSceneArray.size();
	NpScene** scenes = mSceneArray.begin();
	for(PxU32 i=0;i<nbScenes;i++)
		PX_DELETE(scenes[i]);
	mSceneArray.clear();

	//PxU32 matCount = mMasterMaterialTable.size();
	//while (mMasterMaterialTable.size() > 0)
	//{
	//	// It's done this way since the material destructor removes the material from the table and adjusts indices

	//	PX_ASSERT(mMasterMaterialTable[0]->getRefCount() == 1);
	//	mMasterMaterialTable[0]->decRefCount();
	//}
	//mMasterMaterialTable.clear();

	mMasterMaterialManager.releaseMaterials();
#if PX_SUPPORT_GPU_PHYSX
	mMasterDeformableSurfaceMaterialManager.releaseMaterials();
	mMasterDeformableVolumeMaterialManager.releaseMaterials();
	mMasterPBDMaterialManager.releaseMaterials();
#endif

#if PX_SUPPORT_PVD	
	if(mPvd)
	{	
		mPvdPhysicsClient->destroyPvdInstance(this);
		mPvd->removeClient(mPvdPhysicsClient);
		mFoundation.deregisterErrorCallback(*mPvdPhysicsClient);
		PX_DELETE(mPvdPhysicsClient);	
		mFoundation.deregisterAllocationListener(*mPvd);
	}	
#endif

	const DeletionListenerMap::Entry* delListenerEntries = mDeletionListenerMap.getEntries();
	const PxU32 delListenerEntryCount = mDeletionListenerMap.size();
	for(PxU32 i=0; i < delListenerEntryCount; i++)
	{
		NpDelListenerEntry* listener = delListenerEntries[i].second;
		PX_DELETE(listener);
	}
	mDeletionListenerMap.clear();

#if PX_SUPPORT_OMNI_PVD
	OMNI_PVD_DESTROY(OMNI_PVD_CONTEXT_HANDLE, PxPhysics, static_cast<PxPhysics&>(*this))
	if (mOmniPvd)
	{
		mFoundation.deregisterErrorCallback(*mOmniPvdSampler);
		NpOmniPvd::decRefCount();
	}

	PX_DELETE(mOmniPvdSampler);
#endif

#if PX_SUPPORT_GPU_PHYSX
	PxPhysXGpu* gpu = PxvGetPhysXGpu(false);
	if (gpu)
		PxvReleasePhysXGpu(gpu);
#endif
}

PxOmniPvd* NpPhysics::getOmniPvd()
{
#if PX_SUPPORT_OMNI_PVD
	return mOmniPvd;
#else
	return NULL;
#endif
}

void NpPhysics::initOffsetTables(PxvOffsetTable& pxvOffsetTable)
{
	// init offset tables for Pxs/Sc/Px conversions
	{
		Sc::OffsetTable& offsetTable =  Sc::gOffsetTable;
		offsetTable.scRigidStatic2PxActor			= -ptrdiff_t(NpRigidStatic::getCoreOffset());
		offsetTable.scRigidDynamic2PxActor			= -ptrdiff_t(NpRigidDynamic::getCoreOffset());
		offsetTable.scArticulationLink2PxActor		= -ptrdiff_t(NpArticulationLink::getCoreOffset());
#if PX_SUPPORT_GPU_PHYSX
		offsetTable.scDeformableSurface2PxActor	= -ptrdiff_t(NpDeformableSurface::getCoreOffset());
		offsetTable.scDeformableVolume2PxActor	= -ptrdiff_t(NpDeformableVolume::getCoreOffset());
		offsetTable.scPBDParticleSystem2PxActor		= -ptrdiff_t(NpPBDParticleSystem::getCoreOffset());
#endif
		offsetTable.scArticulationRC2Px				= -ptrdiff_t(NpArticulationReducedCoordinate::getCoreOffset());
		offsetTable.scArticulationJointRC2Px		= -ptrdiff_t(NpArticulationJointReducedCoordinate::getCoreOffset());
		offsetTable.scConstraint2Px					= -ptrdiff_t(NpConstraint::getCoreOffset());
		offsetTable.scShape2Px						= -ptrdiff_t(NpShape::getCoreOffset());

		for(PxU32 i=0;i<PxActorType::eACTOR_COUNT;i++)
			offsetTable.scCore2PxActor[i] = 0;
		offsetTable.scCore2PxActor[PxActorType::eRIGID_STATIC] = offsetTable.scRigidStatic2PxActor;
		offsetTable.scCore2PxActor[PxActorType::eRIGID_DYNAMIC] = offsetTable.scRigidDynamic2PxActor;
		offsetTable.scCore2PxActor[PxActorType::eARTICULATION_LINK] = offsetTable.scArticulationLink2PxActor;
		offsetTable.scCore2PxActor[PxActorType::eDEFORMABLE_SURFACE] = offsetTable.scDeformableSurface2PxActor;
		offsetTable.scCore2PxActor[PxActorType::eDEFORMABLE_VOLUME] = offsetTable.scDeformableVolume2PxActor;
		offsetTable.scCore2PxActor[PxActorType::ePBD_PARTICLESYSTEM] = offsetTable.scPBDParticleSystem2PxActor;
	}
	{
		Sc::OffsetTable& scOffsetTable = Sc::gOffsetTable;
		pxvOffsetTable.pxsShapeCore2PxShape			= scOffsetTable.scShape2Px				- ptrdiff_t(Sc::ShapeCore::getCoreOffset());
		pxvOffsetTable.pxsRigidCore2PxRigidBody		= scOffsetTable.scRigidDynamic2PxActor	- ptrdiff_t(Sc::BodyCore::getCoreOffset());
		pxvOffsetTable.pxsRigidCore2PxRigidStatic	= scOffsetTable.scRigidStatic2PxActor	- ptrdiff_t(Sc::StaticCore::getCoreOffset());
	}
}

NpPhysics* NpPhysics::createInstance(PxU32 version, PxFoundation& foundation, const PxTolerancesScale& scale, bool trackOutstandingAllocations, pvdsdk::PsPvd* pvd, PxOmniPvd* omniPvd)
{
#if PX_SWITCH
	NpSetMiddlewareInfo();  // register middleware info such that PhysX usage can be tracked
#endif
	
	if (version!=PX_PHYSICS_VERSION) 
	{
		char buffer[256];
		Pxsnprintf(buffer, 256, "Wrong version: PhysX version is 0x%08x, tried to create 0x%08x", PX_PHYSICS_VERSION, version);
		foundation.getErrorCallback().reportError(PxErrorCode::eINVALID_PARAMETER, buffer, PX_FL);
		return NULL;
	}

	if (!scale.isValid())
	{
		foundation.getErrorCallback().reportError(PxErrorCode::eINVALID_PARAMETER, "Scale invalid.\n", PX_FL);
		return NULL; 
	}

	if(0 == mRefCount)
	{
		PX_ASSERT(&foundation == &PxGetFoundation());

		PxIncFoundationRefCount();

		// init offset tables for Pxs/Sc/Px conversions
		PxvOffsetTable pxvOffsetTable;
		initOffsetTables(pxvOffsetTable);

		//SerialFactory::createInstance();
		mInstance = PX_NEW (NpPhysics)(scale, pxvOffsetTable, trackOutstandingAllocations, pvd, foundation, omniPvd);
		NpFactory::createInstance();

#if PX_SUPPORT_OMNI_PVD
		if (omniPvd)
			NpFactory::getInstance().addFactoryListener(mInstance->mOmniPvdListener);
#endif

#if PX_SUPPORT_PVD			
	    if(pvd)
		{			
			NpFactory::getInstance().setNpFactoryListener( *mInstance->mPvdPhysicsClient );					
			pvd->addClient(mInstance->mPvdPhysicsClient);
		}
#endif

		NpFactory::getInstance().addFactoryListener(mInstance->mDeletionMeshListener);
	}
	++mRefCount;

	return mInstance;
}

PxU32 NpPhysics::releaseInstance()
{
	PX_ASSERT(mRefCount > 0);
	if (--mRefCount) 
		return mRefCount;

#if PX_SUPPORT_PVD	
	if(mInstance->mPvd)
	{	
		NpFactory::getInstance().removeFactoryListener( *mInstance->mPvdPhysicsClient );		
	}
#endif
	
	NpFactory::destroyInstance();

	PX_ASSERT(mInstance);
	PX_DELETE(mInstance);

	PxDecFoundationRefCount();

	return mRefCount;
}

void NpPhysics::release()
{
	NpPhysics::releaseInstance();
}

PxScene* NpPhysics::createScene(const PxSceneDesc& desc)
{
	PX_CHECK_AND_RETURN_NULL(desc.isValid(), "Physics::createScene: desc.isValid() is false!");
	const PxTolerancesScale& scale = mPhysics.getTolerancesScale();
	const PxTolerancesScale& descScale = desc.getTolerancesScale();
	PX_UNUSED(scale);
	PX_UNUSED(descScale);
	PX_CHECK_AND_RETURN_NULL((descScale.length == scale.length) && (descScale.speed == scale.speed), "Physics::createScene: PxTolerancesScale must be the same as used for creation of PxPhysics!");

	PxMutex::ScopedLock lock(mSceneAndMaterialMutex);  // done here because scene constructor accesses profiling manager of the SDK

	NpScene* npScene = PX_NEW (NpScene)(desc, *this);
	if(!npScene)
	{
		mFoundation.error(PxErrorCode::eINTERNAL_ERROR, PX_FL, "Unable to create scene.");
		return NULL;
	}
	if(!npScene->getTaskManagerFast())
	{
		mFoundation.error(PxErrorCode::eINTERNAL_ERROR, PX_FL, "Unable to create scene. Task manager creation failed.");
		return NULL;
	}

	npScene->loadFromDesc(desc);

	OMNI_PVD_ADD(OMNI_PVD_CONTEXT_HANDLE, PxPhysics, scenes, static_cast<PxPhysics&>(*this), static_cast<PxScene&>(*npScene))

#if PX_SUPPORT_PVD
	if(mPvd)
	{
		npScene->getScenePvdClientInternal().setPsPvd(mPvd);		
		mPvd->addClient(&npScene->getScenePvdClientInternal());
	}
#endif

	// checks for allocation errors in cuda context.
#if PX_SUPPORT_GPU_PHYSX
	bool gpuValid = true;

	if (npScene->getCudaContextManager() && npScene->getScScene().isUsingGpuDynamicsOrBp())
	{
		gpuValid = !npScene->getCudaContextManager()->getCudaContext()->isInAbortMode();
	}
#else
	bool gpuValid = true;
#endif

	if (!sendMaterialTable(*npScene) || !npScene->getScScene().isValid() || !gpuValid)
	{
		PX_DELETE(npScene);
		mFoundation.error(PxErrorCode::eOUT_OF_MEMORY, PX_FL, "Unable to create scene.");
		return NULL;
	}

	mSceneArray.pushBack(npScene);
	return npScene;
}

void NpPhysics::releaseSceneInternal(PxScene& scene)
{
	NpScene* pScene =  static_cast<NpScene*>(&scene);

	OMNI_PVD_REMOVE(OMNI_PVD_CONTEXT_HANDLE, PxPhysics, scenes, static_cast<PxPhysics&>(*this), scene)

	PxMutex::ScopedLock lock(mSceneAndMaterialMutex);
	for(PxU32 i=0;i<mSceneArray.size();i++)
	{
		if(mSceneArray[i]==pScene)
		{
			mSceneArray.replaceWithLast(i);
			PX_DELETE(pScene);
			return;
		}
	}
}

PxU32 NpPhysics::getNbScenes() const
{
	PxMutex::ScopedLock lock(const_cast<PxMutex&>(mSceneAndMaterialMutex));
	return mSceneArray.size();
}

PxU32 NpPhysics::getScenes(PxScene** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	PxMutex::ScopedLock lock(const_cast<PxMutex&>(mSceneAndMaterialMutex));
	return Cm::getArrayOfPointers(userBuffer, bufferSize, startIndex, mSceneArray.begin(), mSceneArray.size());
}

PxRigidStatic* NpPhysics::createRigidStatic(const PxTransform& globalPose)
{
	PX_CHECK_AND_RETURN_NULL(globalPose.isSane(), "PxPhysics::createRigidStatic: invalid transform");
	return NpFactory::getInstance().createRigidStatic(globalPose.getNormalized());
}

PxShape* NpPhysics::createShape(const PxGeometry& geometry, PxMaterial*const * materials, PxU16 materialCount, bool isExclusive, PxShapeFlags shapeFlags)
{
	PX_CHECK_AND_RETURN_NULL(materials, "createShape: material pointer is NULL");
	PX_CHECK_AND_RETURN_NULL(materialCount>0, "createShape: material count is zero");

#if PX_CHECKED
	const bool isHeightfield = geometry.getType() == PxGeometryType::eHEIGHTFIELD;
	const bool hasMeshTypeGeom = isHeightfield || (geometry.getType() == PxGeometryType::eTRIANGLEMESH) || (geometry.getType() == PxGeometryType::eTETRAHEDRONMESH);
	PX_CHECK_AND_RETURN_NULL(!(hasMeshTypeGeom && (shapeFlags & PxShapeFlag::eTRIGGER_SHAPE)), "NpPhysics::createShape: triangle mesh/heightfield/tetrahedron mesh triggers are not supported!");
	PX_CHECK_AND_RETURN_NULL(!((shapeFlags & PxShapeFlag::eSIMULATION_SHAPE) && (shapeFlags & PxShapeFlag::eTRIGGER_SHAPE)), "NpPhysics::createShape: shapes cannot simultaneously be trigger shapes and simulation shapes.");
#endif

	return NpFactory::getInstance().createShape(geometry, shapeFlags, materials, materialCount, isExclusive);
}

#if PX_SUPPORT_GPU_PHYSX
PxShape* NpPhysics::createShape(const PxGeometry& geometry, PxDeformableSurfaceMaterial* const* materials, PxU16 materialCount, bool isExclusive, PxShapeFlags shapeFlags)
{
	PX_CHECK_AND_RETURN_NULL(materials, "createShape: material pointer is NULL");
	PX_CHECK_AND_RETURN_NULL(materialCount > 0, "createShape: material count is zero");
	PX_CHECK_AND_RETURN_NULL(geometry.getType() == PxGeometryType::eTRIANGLEMESH, "createShape: deformable surface only accept PxTriangleMeshGeometry");
	PX_CHECK_AND_RETURN_NULL(shapeFlags & PxShapeFlag::eSIMULATION_SHAPE, "createShape: deformable surface shapes must be simulation shapes");

	return NpFactory::getInstance().createShape(geometry, shapeFlags, materials, materialCount, isExclusive);
}

PxShape* NpPhysics::createShape(const PxGeometry& geometry, PxDeformableVolumeMaterial*const * materials, PxU16 materialCount, bool isExclusive, PxShapeFlags shapeFlags)
{
	PX_CHECK_AND_RETURN_NULL(materials, "createShape: material pointer is NULL");
	PX_CHECK_AND_RETURN_NULL(materialCount > 0, "createShape: material count is zero");
	PX_CHECK_AND_RETURN_NULL(geometry.getType() == PxGeometryType::eTETRAHEDRONMESH, "createShape: deformable volumes only accept PxTetrahedronMeshGeometry");
	PX_CHECK_AND_RETURN_NULL(shapeFlags & PxShapeFlag::eSIMULATION_SHAPE, "createShape: deformable volume shapes must be simulation shapes");

	return NpFactory::getInstance().createShape(geometry, shapeFlags, materials, materialCount, isExclusive);
}
#else
PxShape* NpPhysics::createShape(const PxGeometry& geometry, PxDeformableSurfaceMaterial* const* materials, PxU16 materialCount, bool isExclusive, PxShapeFlags shapeFlags)
{
	PX_UNUSED(geometry);
	PX_UNUSED(materials);
	PX_UNUSED(materialCount);
	PX_UNUSED(isExclusive);
	PX_UNUSED(shapeFlags);
	return NULL;
}

PxShape* NpPhysics::createShape(const PxGeometry&, PxDeformableVolumeMaterial*const *, PxU16, bool, PxShapeFlags)
{
	return NULL;
}
#endif

PxU32 NpPhysics::getNbShapes()	const
{
	return NpFactory::getInstance().getNbShapes();
}

PxU32 NpPhysics::getShapes(PxShape** userBuffer, PxU32 bufferSize, PxU32 startIndex)	const
{
	return NpFactory::getInstance().getShapes(userBuffer, bufferSize, startIndex);
}

PxRigidDynamic* NpPhysics::createRigidDynamic(const PxTransform& globalPose)
{
	PX_CHECK_AND_RETURN_NULL(globalPose.isSane(), "PxPhysics::createRigidDynamic: invalid transform");
	return NpFactory::getInstance().createRigidDynamic(globalPose.getNormalized());
}

PxConstraint* NpPhysics::createConstraint(PxRigidActor* actor0, PxRigidActor* actor1, PxConstraintConnector& connector, const PxConstraintShaderTable& shaders, PxU32 dataSize)
{
	return NpFactory::getInstance().createConstraint(actor0, actor1, connector, shaders, dataSize);
}

PxU32 NpPhysics::getNbConstraints() const
{
	return NpFactory::getInstance().getNbConstraints();
}

PxArticulationReducedCoordinate* NpPhysics::createArticulationReducedCoordinate()
{
	return NpFactory::getInstance().createArticulationRC();
}

PxU32 NpPhysics::getNbArticulations() const
{
	return NpFactory::getInstance().getNbArticulations();
}

PxDeformableAttachment* NpPhysics::createDeformableAttachment(const PxDeformableAttachmentData& data)
{
#if PX_SUPPORT_GPU_PHYSX
	return NpFactory::getInstance().createDeformableAttachment(data);
#else
	PX_UNUSED(data);
	return NULL;
#endif
}

PxDeformableElementFilter* NpPhysics::createDeformableElementFilter(const PxDeformableElementFilterData& data)
{
#if PX_SUPPORT_GPU_PHYSX
	return NpFactory::getInstance().createDeformableElementFilter(data);
#else
	PX_UNUSED(data);
	return NULL;
#endif
}

PxDeformableSurface* NpPhysics::createDeformableSurface(PxCudaContextManager& cudaContextManager)
{
#if PX_SUPPORT_GPU_PHYSX
	return NpFactory::getInstance().createDeformableSurface(cudaContextManager);
#else
	PX_UNUSED(cudaContextManager);
	return NULL;
#endif
}

PxDeformableVolume* NpPhysics::createDeformableVolume(PxCudaContextManager& cudaContextManager)
{
#if PX_SUPPORT_GPU_PHYSX
	return NpFactory::getInstance().createDeformableVolume(cudaContextManager);
#else
	PX_UNUSED(cudaContextManager);
	return NULL;
#endif
}

PxPBDParticleSystem* NpPhysics::createPBDParticleSystem(PxCudaContextManager& cudaContextManager, PxU32 maxNeighborhood, PxReal neighborhoodScale)
{
#if PX_SUPPORT_GPU_PHYSX
	return NpFactory::getInstance().createPBDParticleSystem(maxNeighborhood, neighborhoodScale, cudaContextManager);
#else
	PX_UNUSED(cudaContextManager);
	PX_UNUSED(maxNeighborhood);
	PX_UNUSED(neighborhoodScale);
	return NULL;
#endif
}

PxAggregate* NpPhysics::createAggregate(PxU32 maxActors, PxU32 maxShapes, PxAggregateFilterHint filterHint)
{
	PX_CHECK_AND_RETURN_VAL(!(PxGetAggregateSelfCollisionBit(filterHint) && PxGetAggregateType(filterHint)==PxAggregateType::eSTATIC),
		"PxPhysics::createAggregate: static aggregates with self-collisions are not allowed.", NULL);

	return NpFactory::getInstance().createAggregate(maxActors, maxShapes, filterHint);
}

PxU32 NpPhysics::getNbAggregates() const
{
	return NpFactory::getInstance().getNbAggregates();
}

///////////////////////////////////////////////////////////////////////////////

template<class NpMaterialT>
static NpMaterialT* addMaterial(
#if PX_SUPPORT_OMNI_PVD
	NpPhysics::OmniPvdListener& mOmniPvdListener,
#endif
	NpMaterialT* m, NpMaterialManager<NpMaterialT>& materialManager, PxMutex& mutex, PxArray<NpScene*>& sceneArray, const char* error)
{
	if(!m)
		return NULL;

	OMNI_PVD_NOTIFY_ADD(m);

	PxMutex::ScopedLock lock(mutex);

	//the handle is set inside the setMaterial method
	if(materialManager.setMaterial(*m))
	{
		// Let all scenes know of the new material
		const PxU32 nbScenes = sceneArray.size();
		for(PxU32 i=0; i<nbScenes; i++)
			sceneArray[i]->addMaterial(*m);
		return m;
	}
	else
	{
		PxGetFoundation().error(PxErrorCode::eINVALID_PARAMETER, PX_FL, error);
		m->release();
		return NULL;
	}
}

template<class NpMaterialT, class PxMaterialT>
static PxU32 getMaterials(const NpMaterialManager<NpMaterialT>& materialManager, const PxMutex& mutex, PxMaterialT** userBuffer, PxU32 bufferSize, PxU32 startIndex)
{
	PxMutex::ScopedLock lock(const_cast<PxMutex&>(mutex));
	NpMaterialManagerIterator<NpMaterialT> iter(materialManager);
	PxU32 writeCount =0;
	PxU32 index = 0;
	NpMaterialT* mat;
	while(iter.getNextMaterial(mat))
	{
		if(index++ < startIndex)
			continue;
		if(writeCount == bufferSize)
			break;
		userBuffer[writeCount++] = mat;
	}
	return writeCount;
}

template<class NpMaterialT>
static void removeMaterialFromTable(
#if PX_SUPPORT_OMNI_PVD
	NpPhysics::OmniPvdListener& mOmniPvdListener,
#endif
	NpMaterialT& m, NpMaterialManager<NpMaterialT>& materialManager, PxMutex& mutex, PxArray<NpScene*>& sceneArray)
{
	OMNI_PVD_NOTIFY_REMOVE(&m);

	PxMutex::ScopedLock lock(mutex);

	// Let all scenes know of the deleted material
	const PxU32 nbScenes = sceneArray.size();
	for(PxU32 i=0; i<nbScenes; i++)
		sceneArray[i]->removeMaterial(m);

	materialManager.removeMaterial(m);
}

template<class NpMaterialT>
static void updateMaterial(NpMaterialT& m, NpMaterialManager<NpMaterialT>& materialManager, PxMutex& mutex, PxArray<NpScene*>& sceneArray)
{
	PxMutex::ScopedLock lock(mutex);

	// Let all scenes know of the updated material
	const PxU32 nbScenes = sceneArray.size();
	for(PxU32 i=0; i<nbScenes; i++)
		sceneArray[i]->updateMaterial(m);

	materialManager.updateMaterial(m);
}

#if PX_SUPPORT_OMNI_PVD
	#define	_addMaterial(p0, p1, p2, p3, p4)			::addMaterial(mOmniPvdListener, p0, p1, p2, p3, p4)
	#define	_removeMaterialFromTable(p0, p1, p2, p3)	::removeMaterialFromTable(mOmniPvdListener, p0, p1, p2, p3)
#else
	#define	_addMaterial(p0, p1, p2, p3, p4)			::addMaterial(p0, p1, p2, p3, p4)
	#define	_removeMaterialFromTable(p0, p1, p2, p3)	::removeMaterialFromTable(p0, p1, p2, p3)
#endif

#define IMPLEMENT_INTERNAL_MATERIAL_FUNCTIONS(NpMaterialT, manager, errorMsg)		\
NpMaterialT* NpPhysics::addMaterial(NpMaterialT* m)									\
{																					\
	return _addMaterial(m, manager, mSceneAndMaterialMutex, mSceneArray, errorMsg);	\
}																					\
void NpPhysics::removeMaterialFromTable(NpMaterialT& m)								\
{																					\
	_removeMaterialFromTable(m, manager, mSceneAndMaterialMutex, mSceneArray);		\
}																					\
void NpPhysics::updateMaterial(NpMaterialT& m)										\
{																					\
	::updateMaterial(m, manager, mSceneAndMaterialMutex, mSceneArray);				\
}

///////////////////////////////////////////////////////////////////////////////

template<class NpMaterialT>
static void sendMaterialTable(NpScene& scene, const NpMaterialManager<NpMaterialT>& materialManager)
{
	NpMaterialManagerIterator<NpMaterialT> iter(materialManager);
	NpMaterialT* mat;
	while(iter.getNextMaterial(mat))
		scene.addMaterial(*mat);
}

bool NpPhysics::sendMaterialTable(NpScene& scene)
{
	// relies on NpPhysics::mSceneAndMaterialMutex being locked when calling this to
	// prevent clashes with mMasterMaterialManager access as well as access to the
	// event list in NpScene::addMaterial()

	::sendMaterialTable(scene, mMasterMaterialManager);

#if PX_SUPPORT_GPU_PHYSX
	::sendMaterialTable(scene, mMasterDeformableSurfaceMaterialManager);
	::sendMaterialTable(scene, mMasterDeformableVolumeMaterialManager);
	::sendMaterialTable(scene, mMasterPBDMaterialManager);
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////////////

PxMaterial* NpPhysics::createMaterial(PxReal staticFriction, PxReal dynamicFriction, PxReal restitution)
{
	PxMaterial* m = NpFactory::getInstance().createMaterial(staticFriction, dynamicFriction, restitution);
	return addMaterial(static_cast<NpMaterial*>(m));
}

PxU32 NpPhysics::getNbMaterials() const
{
	PxMutex::ScopedLock lock(const_cast<PxMutex&>(mSceneAndMaterialMutex));
	return mMasterMaterialManager.getNumMaterials();
}

PxU32 NpPhysics::getMaterials(PxMaterial** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return ::getMaterials(mMasterMaterialManager, mSceneAndMaterialMutex, userBuffer, bufferSize, startIndex);
}

IMPLEMENT_INTERNAL_MATERIAL_FUNCTIONS(NpMaterial, mMasterMaterialManager, "PxPhysics::createMaterial: limit of 64K materials reached.")

///////////////////////////////////////////////////////////////////////////////

// PT: all the virtual functions that are unconditionally defined in the API / virtual interface cannot be compiled away entirely.
// But the internal functions like addXXXX() can.

#if PX_SUPPORT_GPU_PHYSX
PxDeformableSurfaceMaterial* NpPhysics::createDeformableSurfaceMaterial(PxReal youngs, PxReal poissons, PxReal dynamicFriction, PxReal thickness, 
	PxReal bendingStiffness, PxReal elasticityDamping, PxReal bendingDamping)
{
	PxDeformableSurfaceMaterial* m = NpFactory::getInstance().createDeformableSurfaceMaterial(youngs, poissons, dynamicFriction, thickness, bendingStiffness, elasticityDamping, bendingDamping);
	return addMaterial(static_cast<NpDeformableSurfaceMaterial*>(m));
}

PxU32 NpPhysics::getNbDeformableSurfaceMaterials() const
{
	PxMutex::ScopedLock lock(const_cast<PxMutex&>(mSceneAndMaterialMutex));
	return mMasterDeformableSurfaceMaterialManager.getNumMaterials();
}

PxU32 NpPhysics::getDeformableSurfaceMaterials(PxDeformableSurfaceMaterial** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return ::getMaterials(mMasterDeformableSurfaceMaterialManager, mSceneAndMaterialMutex, userBuffer, bufferSize, startIndex);
}

IMPLEMENT_INTERNAL_MATERIAL_FUNCTIONS(NpDeformableSurfaceMaterial, mMasterDeformableSurfaceMaterialManager, "PxPhysics::createDeformableSurfaceMaterial: limit of 64K materials reached.")
#else
PxDeformableSurfaceMaterial* NpPhysics::createDeformableSurfaceMaterial(PxReal, PxReal, PxReal, PxReal, PxReal, PxReal, PxReal) { return NULL; }
PxU32 NpPhysics::getNbDeformableSurfaceMaterials() const { return 0; }
PxU32 NpPhysics::getDeformableSurfaceMaterials(PxDeformableSurfaceMaterial**, PxU32, PxU32) const { return 0; }
#endif

///////////////////////////////////////////////////////////////////////////////

#if PX_SUPPORT_GPU_PHYSX
	PxDeformableVolumeMaterial* NpPhysics::createDeformableVolumeMaterial(PxReal youngs, PxReal poissons, PxReal dynamicFriction, PxReal elasticityDamping)
	{
		PxDeformableVolumeMaterial* m = NpFactory::getInstance().createDeformableVolumeMaterial(youngs, poissons, dynamicFriction, elasticityDamping);
		return addMaterial(static_cast<NpDeformableVolumeMaterial*>(m));
	}

	PxU32 NpPhysics::getNbDeformableVolumeMaterials() const
	{
		PxMutex::ScopedLock lock(const_cast<PxMutex&>(mSceneAndMaterialMutex));
		return mMasterDeformableVolumeMaterialManager.getNumMaterials();
	}

	PxU32 NpPhysics::getDeformableVolumeMaterials(PxDeformableVolumeMaterial** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
	{
		return ::getMaterials(mMasterDeformableVolumeMaterialManager, mSceneAndMaterialMutex, userBuffer, bufferSize, startIndex);
	}

	IMPLEMENT_INTERNAL_MATERIAL_FUNCTIONS(NpDeformableVolumeMaterial, mMasterDeformableVolumeMaterialManager, "PxPhysics::createDeformableVolumeMaterial: limit of 64K materials reached.")
#else
	PxDeformableVolumeMaterial* NpPhysics::createDeformableVolumeMaterial(PxReal, PxReal, PxReal, PxReal)	{ return NULL;	}
	PxU32 NpPhysics::getNbDeformableVolumeMaterials()												const	{ return 0;		}
	PxU32 NpPhysics::getDeformableVolumeMaterials(PxDeformableVolumeMaterial**, PxU32, PxU32)		const	{ return 0;		}
#endif

///////////////////////////////////////////////////////////////////////////////

#if PX_SUPPORT_GPU_PHYSX
	PxPBDMaterial* NpPhysics::createPBDMaterial(PxReal friction, PxReal damping, PxReal adhesion, PxReal viscosity, PxReal vorticityConfinement, PxReal surfaceTension, 
		PxReal cohesion, PxReal lift, PxReal drag, PxReal cflCoefficient, PxReal gravityScale)
	{
		PxPBDMaterial* m = NpFactory::getInstance().createPBDMaterial(friction, damping, adhesion, viscosity, vorticityConfinement, surfaceTension, cohesion, lift, drag, cflCoefficient, gravityScale);
		return addMaterial(static_cast<NpPBDMaterial*>(m));
	}

	PxU32 NpPhysics::getNbPBDMaterials() const
	{
		PxMutex::ScopedLock lock(const_cast<PxMutex&>(mSceneAndMaterialMutex));
		return mMasterPBDMaterialManager.getNumMaterials();
	}

	PxU32 NpPhysics::getPBDMaterials(PxPBDMaterial** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
	{
		return ::getMaterials(mMasterPBDMaterialManager, mSceneAndMaterialMutex, userBuffer, bufferSize, startIndex);
	}

	IMPLEMENT_INTERNAL_MATERIAL_FUNCTIONS(NpPBDMaterial, mMasterPBDMaterialManager, "PxPhysics::createPBDMaterial: limit of 64K materials reached.")
#else
	PxPBDMaterial* NpPhysics::createPBDMaterial(PxReal, PxReal, PxReal, PxReal, PxReal, PxReal, PxReal, PxReal, PxReal, PxReal, PxReal)	{ return NULL;	}
	PxU32 NpPhysics::getNbPBDMaterials()																						const	{ return 0;		}
	PxU32 NpPhysics::getPBDMaterials(PxPBDMaterial**, PxU32, PxU32)																const	{ return 0;		}
#endif

///////////////////////////////////////////////////////////////////////////////

PxTriangleMesh* NpPhysics::createTriangleMesh(PxInputStream& stream)
{
	return NpFactory::getInstance().createTriangleMesh(stream);
}

PxU32 NpPhysics::getNbTriangleMeshes() const
{
	return NpFactory::getInstance().getNbTriangleMeshes();
}

PxU32 NpPhysics::getTriangleMeshes(PxTriangleMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return NpFactory::getInstance().getTriangleMeshes(userBuffer, bufferSize, startIndex);
}

///////////////////////////////////////////////////////////////////////////////

PxTetrahedronMesh* NpPhysics::createTetrahedronMesh(PxInputStream& stream)
{
	return NpFactory::getInstance().createTetrahedronMesh(stream);
}

PxU32 NpPhysics::getNbTetrahedronMeshes() const
{
	return NpFactory::getInstance().getNbTetrahedronMeshes();
}

PxU32 NpPhysics::getTetrahedronMeshes(PxTetrahedronMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return NpFactory::getInstance().getTetrahedronMeshes(userBuffer, bufferSize, startIndex);
}

PxDeformableVolumeMesh* NpPhysics::createDeformableVolumeMesh(PxInputStream& stream)
{
	return NpFactory::getInstance().createDeformableVolumeMesh(stream);
}

///////////////////////////////////////////////////////////////////////////////

PxHeightField* NpPhysics::createHeightField(PxInputStream& stream)
{
	return NpFactory::getInstance().createHeightField(stream);
}

PxU32 NpPhysics::getNbHeightFields() const
{
	return NpFactory::getInstance().getNbHeightFields();
}

PxU32 NpPhysics::getHeightFields(PxHeightField** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return NpFactory::getInstance().getHeightFields(userBuffer, bufferSize, startIndex);
}

///////////////////////////////////////////////////////////////////////////////

PxConvexMesh* NpPhysics::createConvexMesh(PxInputStream& stream)
{
	return NpFactory::getInstance().createConvexMesh(stream);
}

PxU32 NpPhysics::getNbConvexMeshes() const
{
	return NpFactory::getInstance().getNbConvexMeshes();
}

PxU32 NpPhysics::getConvexMeshes(PxConvexMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return NpFactory::getInstance().getConvexMeshes(userBuffer, bufferSize, startIndex);
}

///////////////////////////////////////////////////////////////////////////////

PxBVH* NpPhysics::createBVH(PxInputStream& stream)
{
	return NpFactory::getInstance().createBVH(stream);
}

PxU32 NpPhysics::getNbBVHs() const
{
	return NpFactory::getInstance().getNbBVHs();
}

PxU32 NpPhysics::getBVHs(PxBVH** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return NpFactory::getInstance().getBVHs(userBuffer, bufferSize, startIndex);
}

///////////////////////////////////////////////////////////////////////////////

#if PX_SUPPORT_GPU_PHYSX
PxParticleBuffer* NpPhysics::createParticleBuffer(PxU32 maxParticles, PxU32 maxVolumes, PxCudaContextManager* cudaContextManager)
{
	if (cudaContextManager)
	{
		return NpFactory::getInstance().createParticleBuffer(maxParticles, maxVolumes, *cudaContextManager);
	}
	return NULL;
}

PxParticleAndDiffuseBuffer* NpPhysics::createParticleAndDiffuseBuffer(PxU32 maxParticles, PxU32 maxVolumes, PxU32 maxDiffuseParticles, PxCudaContextManager* cudaContextManager)
{
	if (cudaContextManager)
	{
		return NpFactory::getInstance().createParticleAndDiffuseBuffer(maxParticles, maxVolumes, maxDiffuseParticles, *cudaContextManager);
	}
	return NULL;
}

PxParticleClothBuffer* NpPhysics::createParticleClothBuffer(PxU32 maxParticles, PxU32 maxNumVolumes, PxU32 maxNumCloths, PxU32 maxNumTriangles, PxU32 maxNumSprings, PxCudaContextManager* cudaContextManager)
{
	if (cudaContextManager)
	{
		return NpFactory::getInstance().createParticleClothBuffer(maxParticles, maxNumVolumes, maxNumCloths, maxNumTriangles, maxNumSprings, *cudaContextManager);
	}
	return NULL;
}

PxParticleRigidBuffer* NpPhysics::createParticleRigidBuffer(PxU32 maxParticles, PxU32 maxNumVolumes, PxU32 maxNumRigids, PxCudaContextManager* cudaContextManager)
{
	if (cudaContextManager)
	{
		return NpFactory::getInstance().createParticleRigidBuffer(maxParticles, maxNumVolumes, maxNumRigids, *cudaContextManager);
	}
	return NULL;
}
#else
PxParticleBuffer* NpPhysics::createParticleBuffer(PxU32, PxU32, PxCudaContextManager*)
{
	return NULL;
}

PxParticleAndDiffuseBuffer* NpPhysics::createParticleAndDiffuseBuffer(PxU32, PxU32, PxU32, PxCudaContextManager*)
{
	return NULL;
}

PxParticleClothBuffer* NpPhysics::createParticleClothBuffer(PxU32, PxU32, PxU32, PxU32, PxU32, PxCudaContextManager*)
{
	return NULL;
}

PxParticleRigidBuffer* NpPhysics::createParticleRigidBuffer(PxU32, PxU32, PxU32, PxCudaContextManager*)
{
	return NULL;
}
#endif

///////////////////////////////////////////////////////////////////////////////

PxPruningStructure* NpPhysics::createPruningStructure(PxRigidActor*const* actors, PxU32 nbActors)
{
	PX_SIMD_GUARD;

	PX_ASSERT(actors);
	PX_ASSERT(nbActors > 0);

	Sq::PruningStructure* ps = PX_NEW(Sq::PruningStructure)();	
	if(!ps->build(actors, nbActors))
	{
		PX_DELETE(ps);		
	}
	return ps;
}

///////////////////////////////////////////////////////////////////////////////

void NpPhysics::registerDeletionListener(PxDeletionListener& observer, const PxDeletionEventFlags& deletionEvents, bool restrictedObjectSet)
{
	PxMutex::ScopedLock lock(mDeletionListenerMutex);

	const DeletionListenerMap::Entry* entry = mDeletionListenerMap.find(&observer);
	if(!entry)
	{
		NpDelListenerEntry* e = PX_NEW(NpDelListenerEntry)(deletionEvents, restrictedObjectSet);
		if (e)
		{
			if (mDeletionListenerMap.insert(&observer, e))
				mDeletionListenersExist = true;
			else
			{
				PX_DELETE(e);
				PX_ALWAYS_ASSERT();
			}
		}
	}
	else
		PX_ASSERT(mDeletionListenersExist);
}

void NpPhysics::unregisterDeletionListener(PxDeletionListener& observer)
{
	PxMutex::ScopedLock lock(mDeletionListenerMutex);

	const DeletionListenerMap::Entry* entry = mDeletionListenerMap.find(&observer);
	if(entry)
	{
		NpDelListenerEntry* e = entry->second;
		mDeletionListenerMap.erase(&observer);
		PX_DELETE(e);
	}
	mDeletionListenersExist = mDeletionListenerMap.size()>0;
}

void NpPhysics::registerDeletionListenerObjects(PxDeletionListener& observer, const PxBase* const* observables, PxU32 observableCount)
{
	PxMutex::ScopedLock lock(mDeletionListenerMutex);

	const DeletionListenerMap::Entry* entry = mDeletionListenerMap.find(&observer);
	if(entry)
	{
		NpDelListenerEntry* e = entry->second;
		PX_CHECK_AND_RETURN(e->restrictedObjectSet, "PxPhysics::registerDeletionListenerObjects: deletion listener is not configured to receive events from specific objects.");

		e->registeredObjects.reserve(e->registeredObjects.size() + observableCount);
		for(PxU32 i=0; i < observableCount; i++)
			e->registeredObjects.insert(observables[i]);
	}
	else
	{
		PX_CHECK_AND_RETURN(false, "PxPhysics::registerDeletionListenerObjects: deletion listener has to be registered in PxPhysics first.");
	}
}

void NpPhysics::unregisterDeletionListenerObjects(PxDeletionListener& observer, const PxBase* const* observables, PxU32 observableCount)
{
	PxMutex::ScopedLock lock(mDeletionListenerMutex);

	const DeletionListenerMap::Entry* entry = mDeletionListenerMap.find(&observer);
	if(entry)
	{
		NpDelListenerEntry* e = entry->second;
		if (e->restrictedObjectSet)
		{
			for(PxU32 i=0; i < observableCount; i++)
				e->registeredObjects.erase(observables[i]);
		}
		else
		{
			PX_CHECK_AND_RETURN(false, "PxPhysics::unregisterDeletionListenerObjects: deletion listener is not configured to receive events from specific objects.");
		}
	}
	else
	{
		PX_CHECK_AND_RETURN(false, "PxPhysics::unregisterDeletionListenerObjects: deletion listener has to be registered in PxPhysics first.");
	}
}

void NpPhysics::notifyDeletionListeners(const PxBase* base, void* userData, PxDeletionEventFlag::Enum deletionEvent)
{
	// we don't protect the check for whether there are any listeners, because we don't want to take a hit in the 
	// common case where there are no listeners. Note the API comments here, that users should not register or 
	// unregister deletion listeners while deletions are occurring

	if(mDeletionListenersExist)
	{
		PxMutex::ScopedLock lock(mDeletionListenerMutex);

		const DeletionListenerMap::Entry* delListenerEntries = mDeletionListenerMap.getEntries();
		const PxU32 delListenerEntryCount = mDeletionListenerMap.size();
		for(PxU32 i=0; i < delListenerEntryCount; i++)
		{
			const NpDelListenerEntry* entry = delListenerEntries[i].second;
			
			if (entry->flags & deletionEvent)
			{
				if(!entry->restrictedObjectSet || entry->registeredObjects.contains(base))
				{
					PX_PROFILE_ZONE("USERCODE - PxDeletionListener::onRelease", 0);	// PT: note that we do not have a scene context ID here
					delListenerEntries[i].first->onRelease(base, userData, deletionEvent);
				}
			}
		}
	}
}

#if PX_SUPPORT_OMNI_PVD
void NpPhysics::OmniPvdListener::onObjectAdd(const PxBase* object)
{
	OmniPvdPxSampler* sampler = ::OmniPvdPxSampler::getInstance();
	if (sampler)
		sampler->onObjectAdd(*object);
}

void NpPhysics::OmniPvdListener::onObjectRemove(const PxBase* object)
{
	OmniPvdPxSampler* sampler = ::OmniPvdPxSampler::getInstance();
	if (sampler)
		sampler->onObjectRemove(*object);
}
#endif

///////////////////////////////////////////////////////////////////////////////

const PxTolerancesScale& NpPhysics::getTolerancesScale() const
{
	return mPhysics.getTolerancesScale();
}

PxFoundation& NpPhysics::getFoundation()
{
	return mFoundation;
}

PxPhysics& PxGetPhysics()
{
	return NpPhysics::getInstance();
}

PxPhysics* PxCreatePhysics(PxU32 version, PxFoundation& foundation, const physx::PxTolerancesScale& scale, bool trackOutstandingAllocations, PxPvd* pvd, PxOmniPvd* omniPvd)
{
	return NpPhysics::createInstance(version, foundation, scale, trackOutstandingAllocations, static_cast<pvdsdk::PsPvd*>(pvd), omniPvd);
}

void PxAddCollectionToPhysics(const PxCollection& collection)
{
	NpFactory& factory = NpFactory::getInstance();
	const Cm::Collection& c = static_cast<const Cm::Collection&>(collection);	
    factory.addCollection(c);
}
