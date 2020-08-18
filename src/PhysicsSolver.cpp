//
//  PhysicsSolver.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "PhysicsSolver.hpp"
#include "PhysicsBodyComponent.hpp"
#ifndef _WIN32
#define _DEBUG              //physx required macro
#endif
#undef NDEBUG

#include <PxPhysicsVersion.h>
#include <snippetcommon/SnippetPVD.h>
#define PX_RELEASE(x)    if(x)    { x->release(); x = NULL;    }

#include <thread>
#include "GameplayStatics.hpp"

using namespace physx;
using namespace std;
using namespace RavEngine;


PxDefaultErrorCallback PhysicsSolver::gDefaultErrorCallback;
PxDefaultAllocator PhysicsSolver::gDefaultAllocatorCallback;
PxFoundation* PhysicsSolver::foundation = nullptr;
PxPhysics* PhysicsSolver::phys = nullptr;
PxPvd* PhysicsSolver::pvd = nullptr;


//see https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Manual/RigidBodyCollision.html#broad-phase-callback
PxFilterFlags SampleFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags & pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
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
        pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;

    return PxFilterFlag::eDEFAULT;
}

/**
 * Given an actor, set its filter layer group
 * @param actor the rigid actor to add
 * @param filterGroup which layer to assign the object (use the enum)
 * @param filterMask a bitmask of which layers the object should collide with
 */
void PhysicsSolver::setupFiltering(PxRigidActor* actor, PxU32 filterGroup, PxU32 filterMask)
{
    PxFilterData filterData;
    filterData.word0 = filterGroup; // word0 = own ID
    filterData.word1 = filterMask;  // word1 = ID mask to filter pairs that trigger a
                                    // contact callback;
    const PxU32 numShapes = actor->getNbShapes();
    //PxShape** shapes = (PxShape**)SAMPLE_ALLOC(sizeof(PxShape*) * numShapes);
    PxShape** shapes = new PxShape*[sizeof(PxShape*) * numShapes];
    actor->getShapes(shapes, numShapes);
    for (PxU32 i = 0; i < numShapes; i++)
    {
        PxShape* shape = shapes[i];
        shape->setSimulationFilterData(filterData);
    }
    delete[] shapes;
    //SAMPLE_FREE(shapes);
}

// Invoked by PhysX after simulation each tick
void PhysicsSolver::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    for (PxU32 i = 0; i < nbPairs; ++i) {
        const PxContactPair& cp = pairs[i];

        //get the physics body component stored in the user data (non-owning pointer!)
        auto actor1 = (PhysicsBodyComponent*)pairHeader.actors[0]->userData;
        auto actor2 = (PhysicsBodyComponent*)pairHeader.actors[1]->userData;

        //invoke events
        if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
            actor1->OnColliderEnter(actor2);
            actor2->OnColliderEnter(actor1);
        }

        if (cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST) {   //one must be a trigger
            actor1->OnColliderExit(actor2);
            actor2->OnColliderExit(actor1);
        }

        if (cp.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS) {   //none must be a trigger
            actor1->OnColliderPersist(actor2);
            actor2->OnColliderPersist(actor1);
        }

    }
}

void PhysicsSolver::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
    for (PxU32 i = 0; i < count; ++i) {
        // ignore pairs when shapes have been deleted
        if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER)) {
            continue;
        }

        //TODO: invoke callbacks
    }
}

void PhysicsSolver::DeallocatePhysx() {
    if (scene != nullptr) {
        PX_RELEASE(scene);
    }
    //null out all of the physics objects
    for (auto& body : objects) {
        body->rigidActor = nullptr;
    }
}

PhysicsSolver::~PhysicsSolver() {
    DeallocatePhysx();
}

void PhysicsSolver::ReleaseStatics() {
    GameplayStatics::currentWorld->DeallocatePhysics();
    PX_RELEASE(phys);
    PX_RELEASE(foundation);
}


/**
 Make the physics system aware of an object
 @param e the entity to add
 */
void PhysicsSolver::Spawn(Ref<Entity> e){
    if (e->Components().HasComponentOfSubclass<PhysicsBodyComponent>()) {
        auto actor = e->Components().GetComponentOfSubclass<PhysicsBodyComponent>();
        actor->rigidActor->userData = actor.get();
        scene->addActor(*(actor->rigidActor));

        //set filtering on the actor if its filtering is not disabled
        if (actor->filterGroup != -1 && actor->filterMask != -1) {
            setupFiltering(actor->rigidActor, actor->filterGroup, actor->filterMask);
        }

        //add to the scene list
        objects.push_back(actor);
    }
}

/**
 Remove the entity from the physics system. This does NOT destroy it in the World.
 @param e the entity to remove
 */
void PhysicsSolver::Destroy(Ref<Entity> e){
    auto body = e->Components().GetComponent<PhysicsBodyComponent>();
    objects.remove(body);
    scene->removeActor(*(body->rigidActor));
}

/**
 Run the appropriate number of physics time steps given a frame rate scale
 @param deltaTime the scale factor to apply
 */
void PhysicsSolver::Tick(float deltaTime){
    scene->simulate(deltaTime / 20);
    scene->fetchResults(true);      //simulate is async, this blocks until the results have been calculated
}

//constructor which configures PhysX
PhysicsSolver::PhysicsSolver(){
    if (foundation == nullptr){
        foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
    }
    if (foundation == nullptr){
        cerr << "PhysX foundation failed!" << endl;
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

    //set the dispatcher (can be CPU or Multithreaded GPU)
    auto cpuDispatcher = PxDefaultCpuDispatcherCreate(std::thread::hardware_concurrency());
    if (!cpuDispatcher) {
        throw std::runtime_error("PhysX dispatcher failed to create ");
    }
    desc.cpuDispatcher = cpuDispatcher;

    desc.filterShader = SampleFilterShader;
    desc.simulationEventCallback = this;
    
    //create the scene
    scene = phys->createScene(desc);
    if (!scene) {
        throw std::runtime_error("PhysX scene failed to create");
    }
}
