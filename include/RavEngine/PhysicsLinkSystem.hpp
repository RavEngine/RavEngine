#pragma once
//
//  PhysicsLinkSystem.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "System.hpp"
#include "PhysXDefines.h"
#include <PxPhysicsAPI.h>
#include "PhysicsBodyComponent.hpp"
#include "ScriptSystem.hpp"
#include "CTTI.hpp"

namespace RavEngine {
	/**
	 This System copies the Entity's transform to the physics simulation transform.
	 It must run after any transform modifications in other systems, ideally at the end of the pipeline.
	 */
	class PhysicsLinkSystemWrite : public AutoCTTI{
	public:
		PhysicsLinkSystemWrite(physx::PxScene* scene ) : dynamicsWorld(scene){}
		
		physx::PxScene* dynamicsWorld = nullptr;
		void operator()(float fpsScale, const std::vector<PhysicsBodyComponent*>&, const std::vector<Transform*>&) const;
	};

	/**
	 This System copies the state of the physics simulation transform to the Entity's transform.
	 It must run before any transform modifications in other systems, ideally at the beginning of the pipeline.
	 */
	class PhysicsLinkSystemRead : public AutoCTTI {
	public:
		PhysicsLinkSystemRead(physx::PxScene* scene ) : dynamicsWorld(scene){}
		
		physx::PxScene* dynamicsWorld = nullptr;
		void operator()(float fpsScale, const RigidBodyDynamicComponent&, Transform&) const;

		//must run before write system
        constexpr const System::list_type& MustRunBefore() const {
			return runbefore;
		}
		
	protected:
		static const System::list_type queries, runbefore;
	};
}
