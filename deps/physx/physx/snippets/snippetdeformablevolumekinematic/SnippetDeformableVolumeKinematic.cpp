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

// ****************************************************************************
// This snippet demonstrates how to setup deformable volumes.
// ****************************************************************************

#include <ctype.h>
#include "PxPhysicsAPI.h"
#include "../snippetcommon/SnippetPrint.h"
#include "../snippetcommon/SnippetPVD.h"
#include "../snippetutils/SnippetUtils.h"
#include "../snippetdeformablevolumekinematic/SnippetDeformableVolumeKinematic.h"
#include "../snippetdeformablevolumekinematic/MeshGenerator.h"
#include "extensions/PxTetMakerExt.h"
#include "extensions/PxDeformableVolumeExt.h"
#include "extensions/PxCudaHelpersExt.h"

using namespace physx;
using namespace meshgenerator;

static PxDefaultAllocator		gAllocator;
static PxDefaultErrorCallback	gErrorCallback;
static PxFoundation*			gFoundation			= NULL;
static PxPhysics*				gPhysics			= NULL;
static PxCudaContextManager*	gCudaContextManager	= NULL;
static PxDefaultCpuDispatcher*	gDispatcher			= NULL;
static PxScene*					gScene				= NULL;
static PxMaterial*				gMaterial			= NULL;
static PxPvd*					gPvd				= NULL;
PxArray<DeformableVolume>		gDeformableVolumes;

void addDeformableVolume(PxDeformableVolume* deformableVolume, const PxTransform& transform, const PxReal density, const PxReal scale)
{
	PxVec4* simPositionInvMassPinned;
	PxVec4* simVelocityPinned;
	PxVec4* collPositionInvMassPinned;
	PxVec4* restPositionPinned;

	PxDeformableVolumeExt::allocateAndInitializeHostMirror(*deformableVolume, gCudaContextManager, simPositionInvMassPinned, simVelocityPinned, collPositionInvMassPinned, restPositionPinned);
	
	const PxReal maxInvMassRatio = 50.f;

	PxDeformableVolumeExt::transform(*deformableVolume, transform, scale, simPositionInvMassPinned, simVelocityPinned, collPositionInvMassPinned, restPositionPinned);
	PxDeformableVolumeExt::updateMass(*deformableVolume, density, maxInvMassRatio, simPositionInvMassPinned);
	PxDeformableVolumeExt::copyToDevice(*deformableVolume, PxDeformableVolumeDataFlag::eALL, simPositionInvMassPinned, simVelocityPinned, collPositionInvMassPinned, restPositionPinned);

	DeformableVolume volume(deformableVolume, gCudaContextManager);

	gDeformableVolumes.pushBack(volume);

	PX_EXT_PINNED_MEMORY_FREE(*gCudaContextManager, simPositionInvMassPinned);
	PX_EXT_PINNED_MEMORY_FREE(*gCudaContextManager, simVelocityPinned);
	PX_EXT_PINNED_MEMORY_FREE(*gCudaContextManager, collPositionInvMassPinned);
	PX_EXT_PINNED_MEMORY_FREE(*gCudaContextManager, restPositionPinned);
}

static PxDeformableVolume* createDeformableVolume(const PxCookingParams& params, const PxArray<PxVec3>& triVerts, const PxArray<PxU32>& triIndices, bool useCollisionMeshForSimulation = false)
{
	PxDeformableVolumeMaterial* material = PxGetPhysics().createDeformableVolumeMaterial(1e+6f, 0.45f, 0.5f);
	material->setDamping(0.005f);

	PxDeformableVolumeMesh* deformableVolumeMesh;

	PxU32 numVoxelsAlongLongestAABBAxis = 8;

	PxSimpleTriangleMesh surfaceMesh;
	surfaceMesh.points.count = triVerts.size();
	surfaceMesh.points.data = triVerts.begin();
	surfaceMesh.triangles.count = triIndices.size() / 3;
	surfaceMesh.triangles.data = triIndices.begin();

	if (useCollisionMeshForSimulation)
	{
		deformableVolumeMesh = PxDeformableVolumeExt::createDeformableVolumeMeshNoVoxels(params, surfaceMesh, gPhysics->getPhysicsInsertionCallback());
	}
	else 
	{
		deformableVolumeMesh = PxDeformableVolumeExt::createDeformableVolumeMesh(params, surfaceMesh, numVoxelsAlongLongestAABBAxis, gPhysics->getPhysicsInsertionCallback());
	}	

	//Alternatively one can cook a deformable volume mesh in a single step
	//tetMesh = cooking.createDeformableVolumeMesh(simulationMeshDesc, collisionMeshDesc, deformableVolumeDesc, physics.getPhysicsInsertionCallback());
	PX_ASSERT(deformableVolumeMesh);

	if (!gCudaContextManager)
		return NULL;
	PxDeformableVolume* deformableVolume = gPhysics->createDeformableVolume(*gCudaContextManager);
	if (deformableVolume)
	{
		PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;

		PxDeformableVolumeMaterial* materialPtr = PxGetPhysics().createDeformableVolumeMaterial(1e+6f, 0.45f, 0.5f);
		materialPtr->setMaterialModel(PxDeformableVolumeMaterialModel::eNEO_HOOKEAN);
		PxTetrahedronMeshGeometry geometry(deformableVolumeMesh->getCollisionMesh());
		PxShape* shape = gPhysics->createShape(geometry, &materialPtr, 1, true, shapeFlags);
		if (shape)
		{
			deformableVolume->attachShape(*shape);
			shape->setSimulationFilterData(PxFilterData(0, 0, 2, 0));
		}
		deformableVolume->attachSimulationMesh(*deformableVolumeMesh->getSimulationMesh(), *deformableVolumeMesh->getDeformableVolumeAuxData());

		gScene->addActor(*deformableVolume);

		addDeformableVolume(deformableVolume, PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxIdentity)), 100.f, 1.0f);
		deformableVolume->setDeformableBodyFlag(PxDeformableBodyFlag::eDISABLE_SELF_COLLISION, true);
		deformableVolume->setSolverIterationCounts(30);


	}
	return deformableVolume;
}

static void createDeformableVolumes(const PxCookingParams& params)
{
	PxCudaContextManager* cudaContextManager = gScene->getCudaContextManager();
	if (!cudaContextManager)
	{
		printf("The Deformable Volumes feature currently only runs on GPU.\n");
		return;
	}

	PxArray<PxVec3> triVerts;
	PxArray<PxU32> triIndices;
	
	PxReal maxEdgeLength = 0.75f;

	createCube(triVerts, triIndices, PxVec3(0, 0, 0), PxVec3(2.5f, 10, 2.5f));
	PxRemeshingExt::limitMaxEdgeLength(triIndices, triVerts, maxEdgeLength);

	PxVec3 position(0, 5.0f, 0);
	for (PxU32 i = 0; i < triVerts.size(); ++i)
	{
		PxVec3& p = triVerts[i];
		PxReal corr = PxSqrt(p.x*p.x + p.z*p.z);
		if (corr != 0)
			corr = PxMax(PxAbs(p.x), PxAbs(p.z)) / corr;
		PxReal scaling = 0.75f + 0.5f * (PxCos(1.5f*p.y) + 1.0f);
		p.x *= scaling * corr;
		p.z *= scaling * corr;
		p += position;
	}
	PxRemeshingExt::limitMaxEdgeLength(triIndices, triVerts, maxEdgeLength);


	PxDeformableVolume* deformableVolume = createDeformableVolume(params, triVerts, triIndices, true);
	

	DeformableVolume* dv = &gDeformableVolumes[0];
	dv->copyDeformedVerticesFromGPU();

	PxU32 vertexCount = dv->mDeformableVolume->getSimulationMesh()->getNbVertices();

	PxVec4* kinematicTargets = PX_EXT_PINNED_MEMORY_ALLOC(PxVec4, *cudaContextManager, vertexCount);	
	PxVec4* positionInvMass = dv->mPositionsInvMass;
	for (PxU32 i = 0; i < vertexCount; ++i)
	{
		PxVec4& p = positionInvMass[i];	
		bool kinematic = false;
		if (i < triVerts.size())
		{
			if (p.y > 9.9f)
				kinematic = true;

			if (p.y > 5 - 0.1f && p.y < 5 + 0.1f)
				kinematic = true;

			if (p.y < 0.1f)
				kinematic = true;
		}
		
		kinematicTargets[i] = PxConfigureDeformableVolumeKinematicTarget(p, kinematic);
	}

	PxVec4* kinematicTargetsD = PX_EXT_DEVICE_MEMORY_ALLOC(PxVec4, *cudaContextManager, vertexCount);
	cudaContextManager->getCudaContext()->memcpyHtoD(reinterpret_cast<CUdeviceptr>(deformableVolume->getSimPositionInvMassBufferD()), positionInvMass, vertexCount * sizeof(PxVec4));
	cudaContextManager->getCudaContext()->memcpyHtoD(reinterpret_cast<CUdeviceptr>(kinematicTargetsD), kinematicTargets, vertexCount * sizeof(PxVec4));
	deformableVolume->setDeformableVolumeFlag(PxDeformableVolumeFlag::ePARTIALLY_KINEMATIC, true);
	deformableVolume->setKinematicTargetBufferD(kinematicTargetsD);
	
	dv->mTargetPositionsH = kinematicTargets;
	dv->mTargetPositionsD = kinematicTargetsD;
	dv->mTargetCount = vertexCount;
}

void initPhysics(bool /*interactive*/)
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);
	
	// initialize cuda
	PxCudaContextManagerDesc cudaContextManagerDesc;
	gCudaContextManager = PxCreateCudaContextManager(*gFoundation, cudaContextManagerDesc, PxGetProfilerCallback());
	if (gCudaContextManager && !gCudaContextManager->contextIsValid())
	{
		PX_RELEASE(gCudaContextManager);
		printf("Failed to initialize cuda context.\n");
	}

	PxTolerancesScale scale;
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale, true, gPvd);
	PxInitExtensions(*gPhysics, gPvd);

	PxCookingParams params(scale);
	params.meshWeldTolerance = 0.001f;
	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES);
	params.buildTriangleAdjacencies = false;
	params.buildGPUData = true;

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	if (!sceneDesc.cudaContextManager)
		sceneDesc.cudaContextManager = gCudaContextManager;
	
	sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
	sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;

	PxU32 numCores = SnippetUtils::getNbPhysicalCores();
	gDispatcher = PxDefaultCpuDispatcherCreate(numCores == 0 ? 0 : numCores - 1);
	sceneDesc.cpuDispatcher	= gDispatcher;
	sceneDesc.filterShader	= PxDefaultSimulationFilterShader;

	sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
	sceneDesc.gpuMaxNumPartitions = 8;

	sceneDesc.solverType = PxSolverType::eTGS;

	gScene = gPhysics->createScene(sceneDesc);
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.f);

	createDeformableVolumes(params);

	// Setup rigid bodies
	const PxReal dynamicsDensity = 10;
	const PxReal boxSize = 0.5f;
	const PxReal spacing = 0.6f;
	const PxReal boxMass = boxSize * boxSize * boxSize * dynamicsDensity;
	const PxU32 gridSizeA = 13;
	const PxU32 gridSizeB = 3;
	const PxReal initialRadius = 1.65f;
	const PxReal distanceJointStiffness = 500.0f;
	const PxReal distanceJointDamping = 0.5f;
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(0.5f * boxSize, 0.5f * boxSize, 0.5f * boxSize), *gMaterial);
	shape->setDensityForFluid(dynamicsDensity);
	PxArray<PxRigidDynamic*> rigids;
	for (PxU32 i = 0; i < gridSizeA; ++i)
		for (PxU32 j = 0; j < gridSizeB; ++j)
		{
			PxReal x = PxCos((2 * PxPi*i) / gridSizeA);
			PxReal y = PxSin((2 * PxPi*i) / gridSizeA);
			PxVec3 pos = PxVec3((x*j)*spacing + x * initialRadius, 8, (y *j)*spacing + y * initialRadius);

			PxReal d = 0.0f;
			{
				PxReal x2 = PxCos((2 * PxPi*(i + 1)) / gridSizeA);
				PxReal y2 = PxSin((2 * PxPi*(i + 1)) / gridSizeA);
				PxVec3 pos2 = PxVec3((x2*j)*spacing + x2 * initialRadius, 8, (y2 *j)*spacing + y2 * initialRadius);
				d = (pos - pos2).magnitude();
			}

			PxRigidDynamic* body = gPhysics->createRigidDynamic(PxTransform(pos));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, boxMass);
			gScene->addActor(*body);
			rigids.pushBack(body);

			if (j > 0)
			{
				PxDistanceJoint* joint = PxDistanceJointCreate(*gPhysics, rigids[rigids.size() - 2], PxTransform(PxIdentity), body, PxTransform(PxIdentity));
				joint->setMaxDistance(spacing);
				joint->setMinDistance(spacing*0.5f);
				joint->setDistanceJointFlags(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED | PxDistanceJointFlag::eMIN_DISTANCE_ENABLED | PxDistanceJointFlag::eSPRING_ENABLED);
				joint->setStiffness(distanceJointStiffness);
				joint->setDamping(distanceJointDamping);
				joint->setConstraintFlags(PxConstraintFlag::eCOLLISION_ENABLED);
			}

			if (i > 0)
			{
				PxDistanceJoint* joint = PxDistanceJointCreate(*gPhysics, rigids[rigids.size() - gridSizeB - 1], PxTransform(PxIdentity), body, PxTransform(PxIdentity));
				joint->setMaxDistance(d);
				joint->setMinDistance(d*0.5f);
				joint->setDistanceJointFlags(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED | PxDistanceJointFlag::eMIN_DISTANCE_ENABLED | PxDistanceJointFlag::eSPRING_ENABLED);
				joint->setStiffness(distanceJointStiffness);
				joint->setDamping(distanceJointDamping);
				joint->setConstraintFlags(PxConstraintFlag::eCOLLISION_ENABLED);
				if (i == gridSizeA - 1)
				{
					PxDistanceJoint* joint2 = PxDistanceJointCreate(*gPhysics, rigids[j], PxTransform(PxIdentity), body, PxTransform(PxIdentity));
					joint2->setMaxDistance(d);
					joint2->setMinDistance(d*0.5f);
					joint2->setDistanceJointFlags(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED | PxDistanceJointFlag::eMIN_DISTANCE_ENABLED | PxDistanceJointFlag::eSPRING_ENABLED);
					joint2->setStiffness(distanceJointStiffness);
					joint2->setDamping(distanceJointDamping);
					joint->setConstraintFlags(PxConstraintFlag::eCOLLISION_ENABLED);
				}
			}
		}
	shape->release();
}

PxReal simTime = 0.0f;
void stepPhysics(bool /*interactive*/)
{
	const PxReal dt = 1.0f / 60.f;

	gScene->simulate(dt);
	gScene->fetchResults(true);

	for (PxU32 i = 0; i < gDeformableVolumes.size(); i++)
	{
		DeformableVolume* dv = &gDeformableVolumes[i]; 
		dv->copyDeformedVerticesFromGPU();	


		PxCudaContextManager* cudaContextManager = dv->mCudaContextManager;
		//Update the kinematic targets to get some motion
		if (i == 0)
		{
			PxReal scaling = PxMin(0.01f, simTime * 0.1f);
			PxReal velocity = 1.0f;
			for (PxU32 j = 0; j < dv->mTargetCount; ++j)
			{
				PxVec4& target = dv->mTargetPositionsH[j];
				if (target.w == 0.0f)
				{
					PxReal phase = target.y*2.0f;
					target.x += scaling * PxSin(velocity * simTime + phase);
					target.z += scaling * PxCos(velocity * simTime + phase);
				}
			}

			PxScopedCudaLock _lock(*cudaContextManager);
			cudaContextManager->getCudaContext()->memcpyHtoD(reinterpret_cast<CUdeviceptr>(dv->mTargetPositionsD), dv->mTargetPositionsH, dv->mTargetCount * sizeof(PxVec4));
		}
	}
	simTime += dt;
}
	
void cleanupPhysics(bool /*interactive*/)
{
	for (PxU32 i = 0; i < gDeformableVolumes.size(); i++)
		gDeformableVolumes[i].release();
	gDeformableVolumes.reset();

	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PX_RELEASE(gPhysics);
	if (gPvd)
	{
		PxPvdTransport* transport = gPvd->getTransport();
		PX_RELEASE(gPvd);
		PX_RELEASE(transport);
	}
	PxCloseExtensions();
	PX_RELEASE(gCudaContextManager);
	PX_RELEASE(gFoundation);

	printf("SnippetDeformableVolumeKinematic done.\n");
}

int snippetMain(int, const char*const*)
{
#ifdef RENDER_SNIPPET
	extern void renderLoop();
	renderLoop();
#else
	static const PxU32 frameCount = 100;
	initPhysics(false);
	for(PxU32 i=0; i<frameCount; i++)
		stepPhysics(false);
	cleanupPhysics(false);
#endif

	return 0;
}
