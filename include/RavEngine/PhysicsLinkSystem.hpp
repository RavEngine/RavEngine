//
//  PhysicsLinkSystem.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "System.hpp"
#include "PhysXDefines.h"
#include <PxPhysicsAPI.h>
#include "PhysicsBodyComponent.hpp"
#include "ScriptSystem.hpp"

namespace RavEngine {
    /**
    This System copies the Entity's transform to the physics simulation transform.
    It must run after any transform modifications in other systems, ideally at the end of the pipeline.
    */
    class PhysicsLinkSystemWrite : public System {
    public:
		PhysicsLinkSystemWrite(physx::PxScene* scene ) : dynamicsWorld(scene){}
		
        physx::PxScene* dynamicsWorld = nullptr;
        virtual ~PhysicsLinkSystemWrite() {}
        void Tick(float fpsScale, Ref<Entity> e) const override;

        plf::list<std::type_index> QueryTypes() const override {
            return { typeid(PhysicsBodyComponent) };
        }
		//must run before write system
		bool MustRunBefore(const std::type_index& other) const override{
			return other == std::type_index(typeid(PhysicsLinkSystemWrite));
		}
       
    };

    /**
     This System copies the state of the physics simulation transform to the Entity's transform.
     It must run before any transform modifications in other systems, ideally at the beginning of the pipeline.
     */
    class PhysicsLinkSystemRead : public System {
    public:
		PhysicsLinkSystemRead(physx::PxScene* scene ) : dynamicsWorld(scene){}
		
        physx::PxScene* dynamicsWorld = nullptr;
        virtual ~PhysicsLinkSystemRead() {}
        void Tick(float fpsScale, Ref<Entity> e) const override;

        plf::list<std::type_index> QueryTypes() const override {
            return {typeid(RigidBodyDynamicComponent)};
        }

		//must run before write system
		bool MustRunBefore(const std::type_index& other) const override {
			return other == std::type_index(typeid(ScriptSystem));
		}
        
    };
}
