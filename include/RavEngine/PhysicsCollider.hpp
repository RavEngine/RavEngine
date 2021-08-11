#pragma once
#include "Component.hpp"
#include "PhysXDefines.h"
#include <PxMaterial.h>
#include <PxPhysics.h>
#include <PxSimulationEventCallback.h>
#include "DebugDraw.hpp"
#include "mathtypes.hpp"
#include "PhysicsMaterial.hpp"
#include "Queryable.hpp"
#include "Common3D.hpp"
#include "Ref.hpp"
#include "MeshAsset.hpp"

namespace physx {
	class PxShape;
}

namespace RavEngine {

	class PhysicsCollider : public Component, public Queryable<PhysicsCollider>
	{
	protected:
		physx::PxShape* collider = nullptr;
		vector3 position = vector3(0,0,0);
		quaternion rotation = quaternion(1.0,0.0,0.0,0.0);
		Ref<PhysicsMaterial> material;
		
		/**
		 @return the world matrix including rotation and position offsets
		 @pre This collider must be attached to an object
		 */
		matrix4 CalculateWorldMatrix() const;
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
		CollisionType GetType() const;

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
		bool GetQueryable() const;
				
		/**
		 Set PxShape relative transformation
		 @param position the relative location of the shape
		 @param rotation the relative rotation of the shape
		 */
		void SetRelativeTransform(const vector3& position, const quaternion& rotation);
		
		/**
		 Draw a wireframe shape representing the boundary of this collider
		 @param color the hex color to use to draw, in format 0xRRGGBBAA
		 */
		virtual void DebugDraw(RavEngine::DebugDraw& dbg, const color_t color = 0xFFFFFFFF) const = 0;

		virtual ~PhysicsCollider();
	};


	class BoxCollider : public PhysicsCollider, public QueryableDelta<PhysicsCollider,BoxCollider> {
	protected:
		vector3 extent;
		void AddHook(const WeakRef<RavEngine::Entity>& e) override;
	public:
		using QueryableDelta<PhysicsCollider,BoxCollider>::GetQueryTypes;

		virtual ~BoxCollider() {}
		BoxCollider(const vector3& position = vector3(0,0,0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0)) : PhysicsCollider(position,rotation) { };

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
		
		/**
		 Draw a wireframe shape representing the boundary of this collider
		 @param color the hex color to use to draw, in format 0xRRGGBBAA
		 */
		void DebugDraw(RavEngine::DebugDraw& dbg, const color_t color = 0xFFFFFFFF) const override;

	};

	class SphereCollider : public PhysicsCollider, public QueryableDelta<PhysicsCollider,SphereCollider>{
	protected:
		decimalType radius;
		void AddHook(const WeakRef<RavEngine::Entity>& e) override;
	public:
		using QueryableDelta<PhysicsCollider,SphereCollider>::GetQueryTypes;
		
		/**
		 Create a sphere collider
		 @param r radius of the collider
		 @param position the relative position of the shape
		 @param rotation the relative rotation of the shape
		 */
		SphereCollider(decimalType r, const vector3& position = vector3(0,0,0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0)) : PhysicsCollider(position,rotation), radius(r){}
		
		/**
		 Create a sphere collider with a material
		 @param r radius of the collider
		 @param mat the physics material of the shape
		 @param position the relative position of the shape
		 @param rotation the relative rotation of the shape
		 */
		SphereCollider(decimalType radius, Ref<PhysicsMaterial> mat, const vector3& position = vector3(0,0,0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0)) : SphereCollider(radius, position, rotation){
			material = mat;
		};
		
		/**
		 Draw a wireframe shape representing the boundary of this collider
		 @param color the hex color to use to draw, in format 0xRRGGBBAA
		 */
		void DebugDraw(RavEngine::DebugDraw& dbg, const color_t color = 0xFFFFFFFF) const override;
	};

	class CapsuleCollider : public PhysicsCollider, public QueryableDelta<PhysicsCollider,CapsuleCollider>{
	protected:
		decimalType radius;
		decimalType halfHeight;
		void AddHook(const WeakRef<RavEngine::Entity>& e) override;
	public:
		using QueryableDelta<PhysicsCollider,CapsuleCollider>::GetQueryTypes;
		
		/**
		 Create a capsule collider
		 @param r radius of the collider
		 @param h the half-height of the collider
		 @param position the relative position of the shape
		 @param rotation the relative rotation of the shape
		 */
		CapsuleCollider(decimalType r, decimalType hh, const vector3& position = vector3(0,0,0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0)) : PhysicsCollider(position,rotation), radius(r), halfHeight(hh){}
		
		/**
		 Create a capsule collider with a material
		 @param r radius of the collider
		 @param h the half-height of the collider
		 @param mat the physics material of the shape
		 @param position the relative position of the shape
		 @param rotation the relative rotation of the shape
		 */
		CapsuleCollider(decimalType r, decimalType hh, Ref<PhysicsMaterial> mat, const vector3& position = vector3(0,0,0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0)) : CapsuleCollider(r,hh,position,rotation){
			material = mat;
		}
		
		/**
		 Draw a wireframe shape representing the boundary of this collider
		 @param color the hex color to use to draw, in format 0xRRGGBBAA
		 */
		void DebugDraw(RavEngine::DebugDraw& dbg, const color_t color = 0xFFFFFFFF) const override;
	};

	class MeshCollider : public PhysicsCollider, public QueryableDelta<PhysicsCollider, MeshCollider>{
	protected:
		Ref<MeshAsset> meshAsset;
		void AddHook(const WeakRef<RavEngine::Entity>& e) override;
	public:
		using QueryableDelta<PhysicsCollider,MeshCollider>::GetQueryTypes;
		
		/**
		 Create a MeshCollider given a MeshAsset
		 @param mesh the MeshAsset to use
		 */
		MeshCollider(Ref<MeshAsset> mesh) : PhysicsCollider(vector3(0,0,0), quaternion(1.0,0,0,0)), meshAsset(mesh){}
		
		/**
		 Create a MeshCollider given a MeshAsset physics material
		 @param mesh the MeshAsset to use
		 @param mat the PhysicsMaterial to use
		 */
		MeshCollider(Ref<MeshAsset> mesh, Ref<PhysicsMaterial> mat) : PhysicsCollider(vector3(0,0,0), quaternion(1.0,0,0,0)), meshAsset(mesh){
			material = mat;
		}
		
		void DebugDraw(RavEngine::DebugDraw& dbg, const color_t color = 0xFFFFFFFF) const override{
			//TODO: debug draw mesh collider
		}
	};

	class ConvexMeshCollider : public PhysicsCollider, public QueryableDelta<PhysicsCollider, ConvexMeshCollider>{
	protected:
		Ref<MeshAsset> meshAsset;
		void AddHook(const WeakRef<RavEngine::Entity>& e) override;
	public:
		using QueryableDelta<PhysicsCollider,ConvexMeshCollider>::GetQueryTypes;
		
		/**
		 Create a Convex Mesh Collider given a MeshAsset
		 @param mesh the MeshAsset to use
		 */
		ConvexMeshCollider(Ref<MeshAsset> mesh) : PhysicsCollider(vector3(0,0,0),quaternion(1.0,0,0,0)), meshAsset(mesh){}
		
		/**
		 Create a Convex Mesh Collider given a MeshAsset physics material
		 @param mesh the MeshAsset to use
		 @param mat the PhysicsMaterial to use
		 */
		ConvexMeshCollider(Ref<MeshAsset> mesh, Ref<PhysicsMaterial> mat) : PhysicsCollider(vector3(0,0,0), quaternion(1.0,0,0,0)), meshAsset(mesh){
			material = mat;
		}
		
		void DebugDraw(RavEngine::DebugDraw& dbg, const color_t color = 0xFFFFFFFF) const override{
			//TODO: debug draw mesh collider
		}
	};
}
