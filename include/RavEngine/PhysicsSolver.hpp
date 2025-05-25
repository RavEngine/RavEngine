#pragma once
//
//  PhysicsSolver.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//


#include "PhysXDefines.h"
#include <PxQueryReport.h>
#include "Ref.hpp"
#include "PhysicsCollider.hpp"
#include "PhysicsTaskDispatcher.hpp"
#include <PxPhysicsAPI.h>
#include <PxFiltering.h>
#include <cstdint>
#include "Types.hpp"
#include "Entity.hpp"

struct FilterLayers {
    enum Enum {
        L0 = (1 << 0),
        L1 = (1 << 1),
        L2 = (1 << 2),
        L3 = (1 << 3),
    };
};

namespace RavEngine {
    struct Entity;
    struct PhysicsBodyComponent;
    struct World;
    class PhysicsSolver : public physx::PxSimulationEventCallback {
        friend class World;
        PhysicsTaskDispatcher taskDispatcher;
    protected:
        //static members must exist only once in the application
        static physx::PxDefaultErrorCallback gDefaultErrorCallback;
        static physx::PxDefaultAllocator gDefaultAllocatorCallback;
        static physx::PxFoundation* foundation;
    public:
        static physx::PxPhysics* phys;
        static physx::PxPvd* pvd;
        World* owner = nullptr;
        physx::PxScene* scene;

        void DeallocatePhysx();
    protected:

        void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask);

        // PxSimulationEventCallback overrides
        virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {}
        virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override {}
        virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override {}
        virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
        virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
        virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override {}

        // If deltatime > this value, the system will substep
        constexpr static float max_step_time = 1.f/30;
        
        friend class PhysicsBodyComponent;

    public:
        PhysicsSolver(World* world);
		~PhysicsSolver(){
			DeallocatePhysx();
		}

        void Spawn( PhysicsBodyComponent&);
        void Destroy( PhysicsBodyComponent&);

        void Tick(float deltaTime);

        static void ReleaseStatics();

        //scene query methods
        struct RaycastHit {
            RaycastHit() {}
            RaycastHit(const physx::PxRaycastBuffer& hit, World* owner);
            
            bool hasBlocking{};
            vector3 hitPosition{};
            vector3 hitNormal{};
            decimalType hitDistance{};
            Entity getEntity() const;
        private:
            Entity hitObject;
        };

        /**
        Perform a raycast in the scene.
        @param origin the starting point of the vector
        @param direction the direction to continue the vector
        @param maxDistance the distance to continue the cast
        @param out_hit the RaycastHit to write the data into
        @returns true if the Raycast hit, false otherwise
        */
        bool Raycast(const vector3& origin, const vector3& direction, decimalType maxDistance, RaycastHit& out_hit, uint32_t layermask = ALL_LAYERS);


        struct OverlapHit {
            OverlapHit() {}
            //TODO: FIX
            OverlapHit(const physx::PxOverlapBuffer& hit)/* : overlapObject( ((PhysicsBodyComponent*)hit.block.actor->userData)->GetOwner())*/{}
            Ref<Entity> overlapObject;
        };

        /**
        Perform a box overlap
        @param origin the center point of the box overlap
        @param rotation the rotation of the box
        @param half_ext the distance from the origin to the corner for each axis
        @param out_hit the cast results to write the data into
        @return true if the box overlap found data, false otherwise
        */
        bool BoxOverlap(const vector3& origin, const quaternion& rotation, const vector3& half_ext, OverlapHit& out_hit);

        /**
        Perform a sphere overlap
        @param origin the center point of the sphere overlap
        @param radius radius of the sphere
        @param out_hit where to write the results
        @return true if the overlap found data
        */
        bool SphereOverlap(const vector3& origin, decimalType radius, OverlapHit& out_hit);

        /**
        Perform a capsule overlap
        @param origin the center of the capsule
        @param rotation the rotation of the capsule
        @param halfHeight the distance from the center of the capsule to a pole
        @param out_hit the destination to write the results into
        @return true if the capsule overlap found data
        */
        bool CapsuleOverlap(const vector3& origin, const quaternion& rotation, decimalType radius, decimalType halfheight, OverlapHit& out_hit);


    protected:
        struct PhysicsTransform {
            vector3 pos;
            quaternion rot;
            PhysicsTransform(const vector3& p, const quaternion& q) : pos(p), rot(q) {}
        };

        /**
        Generic version of overlap
        @param transform the transform to pass
        @param geo the geometry for the it
        @param out_hit the destination to write the results
        */
        bool generic_overlap(const PhysicsTransform& transform, const physx::PxGeometry& geo, OverlapHit& out_hit);
    };
}
