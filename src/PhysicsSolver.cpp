//
//  PhysicsSolver.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "PhysicsSolver.hpp"
#include "PhysicsBodyComponent.hpp"
#include "App.hpp"
#include "PhysXDefines.h"
#include "Entity.hpp"
#include <PxPhysicsVersion.h>
#include <snippetcommon/SnippetPVD.h>
#include <extensions/PxDefaultSimulationFilterShader.h>
#define PX_RELEASE(x)    if(x)    { x->release(); x = NULL;    }

#include <thread>

using namespace physx;
using namespace std;
using namespace RavEngine;


STATIC(PhysicsSolver::gDefaultErrorCallback);
STATIC(PhysicsSolver::gDefaultAllocatorCallback);
STATIC(PhysicsSolver::foundation) = nullptr;
STATIC(PhysicsSolver::phys) = nullptr;
STATIC(PhysicsSolver::pvd) = nullptr;
STATIC(PhysicsSolver::cooking) = nullptr;


//see https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Manual/RigidBodyCollision.html#broad-phase-callback
PxFilterFlags FilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags & pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
    // let triggers through
    if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
    {
        pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
        return PxFilterFlag::eDEFAULT;
    }
    // generate contacts for all that were not filtered above
    pairFlags = PxPairFlag::eCONTACT_DEFAULT;

    // trigger the contact callback for pairs (A,B) where
    // the filtermask of A contains the ID of B and vice versa.
    if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
        pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_TOUCH_LOST | PxPairFlag::eNOTIFY_CONTACT_POINTS;

    return PxFilterFlag::eDEFAULT;
}


// Invoked by PhysX after simulation each tick
void PhysicsSolver::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    for (PxU32 i = 0; i < nbPairs; ++i) {
        const PxContactPair& contactpair = pairs[i];
        
        //if these actors do not exist in the scene anymore due to deallocation, do not process
        if(pairHeader.actors[0]->userData == nullptr || pairHeader.actors[1]->userData == nullptr){
            continue;
        }

        Entity actor1_e, actor2_e;
        
        //get the physics body component stored in the user data (non-owning pointer!)
        std::memcpy(&actor1_e, &pairHeader.actors[0]->userData, sizeof(actor1_e));
        std::memcpy(&actor2_e, &pairHeader.actors[1]->userData, sizeof(actor2_e));

        auto& actor1 = actor1_e.GetAllComponentsPolymorphic<PhysicsBodyComponent>()[0];
        auto& actor2 = actor2_e.GetAllComponentsPolymorphic<PhysicsBodyComponent>()[0];

        size_t numContacts = 0;
        stackarray(contactPoints, ContactPairPoint, contactpair.contactCount);
        {
            // do we need contact data?
            if (actor1.GetWantsContactData() || actor2.GetWantsContactData()) {
                stackarray(points, PxContactPairPoint, contactpair.contactCount);
                auto count = contactpair.extractContacts(points, contactpair.contactCount);
                for (int i = 0; i < contactpair.contactCount; i++) {
                    contactPoints[i] = points[i];
                }
                numContacts = count;
            }
        }
		
        //invoke events
        if (contactpair.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) {

            actor1.OnColliderEnter(actor2,contactPoints, numContacts);
            actor2.OnColliderEnter(actor1, contactPoints, numContacts);
        }

        if (contactpair.events & PxPairFlag::eNOTIFY_TOUCH_LOST) {
            actor1.OnColliderExit(actor2, contactPoints, numContacts);
            actor2.OnColliderExit(actor1, contactPoints, numContacts);
        }

        if (contactpair.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS) {
            actor1.OnColliderPersist(actor2, contactPoints, numContacts);
            actor2.OnColliderPersist(actor1, contactPoints, numContacts);
        }

    }
}

void PhysicsSolver::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
    for (PxU32 i = 0; i < count; ++i) {
        // ignore pairs when shapes have been deleted
		const PxTriggerPair& cp = pairs[i];
        if (cp.flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER)) {
            continue;
        }
		
        //if these actors do not exist in the scene anymore due to deallocation, do not process
        if(cp.otherActor->userData == nullptr || cp.triggerActor->userData == nullptr){
            continue;
        }
        
        Entity other_e;
        Entity trigger_e;
        
        memcpy(&other_e, &cp.otherActor->userData, sizeof(other_e));
        memcpy(&trigger_e, &cp.triggerActor->userData, sizeof(trigger_e));
    
        auto& other = other_e.GetAllComponentsPolymorphic<PhysicsBodyComponent>()[0];
        auto& trigger = trigger_e.GetAllComponentsPolymorphic<PhysicsBodyComponent>()[0];

		//process events
		if(cp.status & (PxPairFlag::eNOTIFY_TOUCH_FOUND)){
			other.OnTriggerEnter(trigger);
			trigger.OnTriggerEnter(other);
		}
		
		if(cp.status & (PxPairFlag::eNOTIFY_TOUCH_LOST)){
			other.OnTriggerExit(trigger);
			trigger.OnTriggerExit(other);
		}
    }
}

void PhysicsSolver::DeallocatePhysx() {
    if (scene != nullptr) {
        PX_RELEASE(scene);
    }
}

void PhysicsSolver::ReleaseStatics() {
    PX_RELEASE(phys);
    PX_RELEASE(foundation);
	PX_RELEASE(cooking);
}

bool RavEngine::PhysicsSolver::Raycast(const vector3& origin, const vector3& direction, decimalType maxDistance, RaycastHit& out_hit)
{
    PxRaycastBuffer hit;
    bool result = scene->raycast(PxVec3(origin.x, origin.y, origin.z), PxVec3(direction.x, direction.y, direction.z), maxDistance, hit);

    //construct hit result
    out_hit = RaycastHit(hit);
    return result;
}

bool RavEngine::PhysicsSolver::BoxOverlap(const vector3& origin, const quaternion& r, const vector3& half_ext, OverlapHit& out_hit)
{
    return generic_overlap(PhysicsTransform(origin,r),PxBoxGeometry(half_ext.x, half_ext.y, half_ext.z),out_hit);
}

bool RavEngine::PhysicsSolver::SphereOverlap(const vector3& origin, decimalType radius, OverlapHit& out_hit)
{
    return generic_overlap(PhysicsTransform(origin,quaternion()),PxSphereGeometry(radius),out_hit);
}

bool RavEngine::PhysicsSolver::CapsuleOverlap(const vector3& origin, const quaternion& rotation, decimalType radius, decimalType halfheight, OverlapHit& out_hit)
{
    return generic_overlap(PhysicsTransform(origin,rotation),PxCapsuleGeometry(radius,halfheight),out_hit);
}

bool RavEngine::PhysicsSolver::generic_overlap(const PhysicsTransform& t, const PxGeometry& geo, OverlapHit& out_hit)
{
    PxOverlapBuffer hit;
    bool result = scene->overlap(geo,PxTransform(PxVec3(t.pos.x,t.pos.y,t.pos.y),PxQuat(t.rot.x,t.rot.y,t.rot.z,t.rot.w)),hit);
    out_hit = OverlapHit(hit);
    return result;
}


/**
 Make the physics system aware of an object
 @param e the entity to add
 */
void PhysicsSolver::Spawn(PhysicsBodyComponent& actor){
    auto e = actor.GetOwner();
    if (actor.rigidActor->userData == nullptr){
        memcpy(&actor.rigidActor->userData, &e, sizeof(e)); // store the ID inside the userdata variable (NOT a pointer)
    }
    scene->lockWrite();
    scene->addActor(*(actor.rigidActor));
    scene->unlockWrite();
}

/**
 Remove the entity from the physics system. This does NOT destroy it in the World.
 @param e the entity to remove
 */
void PhysicsSolver::Destroy(PhysicsBodyComponent& body){
    scene->lockWrite();
    scene->removeActor(*(body.rigidActor));
    scene->unlockWrite();
}

/**
 Run the appropriate number of physics time steps given a frame rate scale
 @param deltaTime the scale factor to apply
 */
void PhysicsSolver::Tick(float scaleFactor){

    auto step = scaleFactor / (GetApp()->evalNormal / 2);

    //physics substepping
    int nsteps = ceil(step / max_step_time);
    float step_time = step / nsteps;
	scene->lockWrite();
    for (int i = 0; i < nsteps; i++)
    {
        scene->simulate(step_time);
        scene->fetchResults(true);      //simulate is async, this blocks until the results have been calculated
    }
	scene->unlockWrite();
}

//constructor which configures PhysX
PhysicsSolver::PhysicsSolver(){
    if (foundation == nullptr){
        foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
    }
    if (foundation == nullptr){
		Debug::Fatal("PhysX foundation failed to create");
    }
    bool recordMemoryAllocations = true;
    if (pvd == nullptr) {
        pvd = PxCreatePvd(*foundation);
        PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
        pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
        if (phys == nullptr) {
            phys = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), recordMemoryAllocations, pvd);
        }
    }

    //create PhysX scene
    PxSceneDesc desc(phys->getTolerancesScale());
    desc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

    //set the dispatcher
    desc.cpuDispatcher = &taskDispatcher;

    desc.filterShader = FilterShader;
    desc.simulationEventCallback = this;
	
	// initialize cooking library with defaults
	cooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, PxCookingParams(PxTolerancesScale()));
	if (!cooking){
		Debug::Fatal("PhysX Cooking initialization failed");
	}
	
	// setup cooking parameters (used for all subsequent calls to cooking)
	PxTolerancesScale scale;
	PxCookingParams params(scale);
	// disable mesh cleaning - perform mesh validation on development configurations
	//params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
	// disable edge precompute, edges are set for each triangle, slows contact generation
	//params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
	
	PhysicsSolver::cooking->setParams(params);
	
	// initialize extensions (can be omitted, these are optional components)
	if (!PxInitExtensions(*phys, pvd)){
		Debug::Fatal("Unable to initialize PhysX");
	}
    
    //create the scene
    scene = phys->createScene(desc);
    if (!scene) {
		Debug::Fatal("PhysX Scene failed to create");
    }
}

PhysicsSolver::RaycastHit::RaycastHit(const physx::PxRaycastBuffer& hit) : 
    hasBlocking(hit.hasBlock),
    hitPosition(vector3(hit.block.position.x, hit.block.position.y, hit.block.position.z)),
    hitNormal(vector3(hit.block.normal.x, hit.block.normal.y, hit.block.normal.z)),
    hitDistance(hit.block.distance) {

    if (hit.hasBlock) {
        hitObject = (entity_t(uintptr_t(hit.block.actor->userData)));
    }
}

Entity RavEngine::PhysicsSolver::RaycastHit::getEntity() const
{
    return Entity(hitObject);
}
