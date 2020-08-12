//
//  PhysicsSolver.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once

#include "PhysXDefines.h"
#include "SharedObject.hpp"
#include "PhysicsCollider.hpp"
#include "PhysicsBodyComponent.hpp"
#include "Entity.hpp"
#include <PxPhysicsAPI.h>
#include <list>
#include <PxFiltering.h>
#include <iostream>
#include <unordered_map>
#include <cstdint>

struct FilterLayers {
    enum Enum {
        L0 = (1 << 0),
        L1 = (1 << 1),
        L2 = (1 << 2),
        L3 = (1 << 3),
    };
};

namespace RavEngine {
    class PhysicsSolver : public SharedObject, public physx::PxSimulationEventCallback {
    protected:
        //static members must exist only once in the application
        static physx::PxDefaultErrorCallback gDefaultErrorCallback;
        static physx::PxDefaultAllocator gDefaultAllocatorCallback;
        static physx::PxFoundation* foundation;
    public:
        static physx::PxPhysics* phys;
        static physx::PxPvd* pvd;
        physx::PxScene* scene;

        void DeallocatePhysx();
    protected:
        std::list<Ref<PhysicsBodyComponent>> objects;

        void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask);

        // PxSimulationEventCallback overrides
        virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {}
        virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override {}
        virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override {}
        virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
        virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
        virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override {}

    public:
        //for sharedobject
        virtual ~PhysicsSolver();
        PhysicsSolver();

        void Spawn(Ref<Entity>);
        void Destroy(Ref<Entity>);

        void Tick(float deltaTime);

        static void ReleaseStatics();
    };
}