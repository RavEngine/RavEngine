#pragma once
//
//  PhysicsLinkSystem.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "CTTI.hpp"

namespace phsyx{
    class PxScene;
}

namespace RavEngine {
	struct RigidBodyStaticComponent;
	struct RigidBodyDynamicComponent;
	struct Transform;

	/**
	 This System copies the Entity's transform to the physics simulation transform.
	 It must run after any transform modifications in other systems, ideally at the end of the pipeline.
	 */
	class PhysicsLinkSystemWrite : public AutoCTTI{
	public:		
		void operator()(const RigidBodyStaticComponent&, const Transform&) const;
	};

	/**
	 This System copies the state of the physics simulation transform to the Entity's transform.
	 It must run before any transform modifications in other systems, ideally at the beginning of the pipeline.
	 */
	class PhysicsLinkSystemRead : public AutoCTTI {
	public:
		void operator()(const RigidBodyDynamicComponent&, Transform&) const;
	};
}
