#pragma once
#include "Component.hpp"
#include "PhysXDefines.h"
#include <PxMaterial.h>
#include <PxPhysics.h>
#include <PxSimulationEventCallback.h>
#include <iostream>
#include "mathtypes.hpp"
#include "PhysicsMaterial.hpp"
#include "Queryable.hpp"

namespace physx {
	class PxShape;
}

namespace RavEngine {
	class Entity;
	class PhysicsCollider : public Component, public Queryable<PhysicsCollider>
	{
	protected:
		physx::PxShape* collider = nullptr;
		vector3 position = vector3(0,0,0);
		quaternion rotation = quaternion(1.0,0.0,0.0,0.0);
		Ref<PhysicsMaterial> material;
	public:
		PhysicsCollider(const vector3& position, const quaternion& rotation) : position(position), rotation(rotation) {}

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
				
		/**
		 Set PxShape relative transformation
		 @param position the relative location of the shape
		 @param rotation the relative rotation of the shape
		 */
		void SetRelativeTransform(const vector3& position, const quaternion& rotation);

		virtual ~PhysicsCollider();
	};


	class BoxCollider : public PhysicsCollider, public QueryableDelta<PhysicsCollider,BoxCollider> {
	protected:
		vector3 extent;
	public:
		using QueryableDelta<PhysicsCollider,BoxCollider>::GetQueryTypes;

		virtual ~BoxCollider() {}
		BoxCollider(const vector3& position = vector3(0,0,0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0)) : PhysicsCollider(position,rotation) { RegisterAllAlternateTypes(); };

		/**
		 * Create a box collider with an extent and a physics material
		 * @param ext the dimensions of the collider
		 * @param mat the physics material to assign
		 * @note The current scale of the transform is assumed to be the identity size for ResizeToFit.
		 */
		BoxCollider(const vector3& ext, Ref<PhysicsMaterial> mat, const vector3& position = vector3(0, 0, 0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0)) : BoxCollider(position,rotation) {
			extent = ext;
			material = mat;
		}

		void AddHook(const WeakRef<RavEngine::Entity>& e) override;
	};

	class SphereCollider : public PhysicsCollider, public QueryableDelta<PhysicsCollider,SphereCollider>{
	protected:
		decimalType radius;
	public:
		using QueryableDelta<PhysicsCollider,SphereCollider>::GetQueryTypes;
		
		SphereCollider(decimalType r, const vector3& position = vector3(0,0,0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0)) : PhysicsCollider(position,rotation){
			radius = r;
			RegisterAllAlternateTypes();
		};
		
		SphereCollider(decimalType radius, Ref<PhysicsMaterial> mat, const vector3& position = vector3(0,0,0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0)) : SphereCollider(radius, position, rotation){
			material = mat;
		};
		
		void AddHook(const WeakRef<RavEngine::Entity>& e) override;
	};
}
