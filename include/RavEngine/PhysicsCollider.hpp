#pragma once
#include "Component.hpp"
#include "PhysXDefines.h"
#include <PxMaterial.h>
#include <PxPhysics.h>
#include <PxSimulationEventCallback.h>
#include <iostream>
#include "mathtypes.hpp"
#include "PhysicsMaterial.hpp"

namespace physx {
	class PxShape;
}

namespace RavEngine {
	class Entity;
	class PhysicsCollider : public Component
	{
	protected:
		physx::PxShape* collider = nullptr;
	public:
		enum class CollisionType { Trigger, Collider };

		/**
		Set the state to collider or trigger.
		@param the new state
		*/
		void SetType(CollisionType);

		/**
		@returns if the current collider is a trigger or not
		*/
		CollisionType GetType();

		/**
		Set whether the collider participates in scene queries (raycasts, overlaps, etc)
		@param the new state
		*/
		void SetQueryable(bool);

		/**
		@return if the scene is queryable (see SetQueryable)
		*/
		bool GetQueryable();

		virtual ~PhysicsCollider();
	};


	class BoxCollider : public PhysicsCollider {
	protected:
		vector3 extent;
		Ref<PhysicsMaterial> material;
	public:

		virtual ~BoxCollider() {}
		BoxCollider() : PhysicsCollider() { RegisterAllAlternateTypes(); };

		/**
		 * Create a box collider with an extent and a physics material
		 * @param ext the dimensions of the collider
		 * @param mat the physics material to assign
		 */
		BoxCollider(const vector3& ext, Ref<PhysicsMaterial> mat, CollisionType contact = CollisionType::Collider, bool trigger = false) : BoxCollider() {
			extent = ext;
			material = mat;
			Type = contact;
			eventsEnabled = trigger;
		}

		void AddHook(const WeakRef<RavEngine::Entity>& e) override;

		virtual void RegisterAllAlternateTypes() override {
			RegisterAlternateQueryType<PhysicsCollider>();
		}
	};
}