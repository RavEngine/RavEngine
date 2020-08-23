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
		@pre This component must be added to an entity with a PhysicsBodyComponent before using this call.
		*/
		void SetType(CollisionType);

		/**
		@returns if the current collider is a trigger or not
		@pre This component must be added to an entity with a PhysicsBodyComponent before using this call.
		*/
		CollisionType GetType();

		/**
		Set whether the collider participates in scene queries (raycasts, overlaps, etc)
		@param the new state
		@pre This component must be added to an entity with a PhysicsBodyComponent before using this call.
		*/
		void SetQueryable(bool);

		/**
		@return if the scene is queryable (see SetQueryable)
		@pre This component must be added to an entity with a PhysicsBodyComponent before using this call.
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
		 * @note The current scale of the transform is assumed to be the identity size for ResizeToFit.
		 */
		BoxCollider(const vector3& ext, Ref<PhysicsMaterial> mat) : BoxCollider() {
			extent = ext;
			material = mat;
		}

		void AddHook(const WeakRef<RavEngine::Entity>& e) override;	

		virtual void RegisterAllAlternateTypes() override {
			RegisterAlternateQueryType<PhysicsCollider>();
		}
	};
}