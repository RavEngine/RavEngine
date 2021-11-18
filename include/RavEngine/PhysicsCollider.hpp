#pragma once
#include "PhysXDefines.h"
#include <PxMaterial.h>
#include <PxPhysics.h>
#include <PxSimulationEventCallback.h>
#include "DebugDrawer.hpp"
#include "mathtypes.hpp"
#include "PhysicsMaterial.hpp"
#include "Common3D.hpp"
#include "Ref.hpp"
#include "MeshAsset.hpp"

namespace physx {
	class PxShape;
}

namespace RavEngine {
    struct PhysicsBodyComponent;
    class PhysicsCollider
	{
        friend class PhysicsBodyComponent;
	protected:
		physx::PxShape* collider = nullptr;
		Ref<PhysicsMaterial> material;
		
		/**
		 @return the world matrix including rotation and position offsets
		 @pre This collider must be attached to an object
		 */
		matrix4 CalculateWorldMatrix() const;
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
        
        
        virtual void DebugDraw(RavEngine::DebugDrawer& dbg) const;

		virtual ~PhysicsCollider();
	};


	class BoxCollider : public PhysicsCollider {
	protected:
		vector3 extent;
	public:

		virtual ~BoxCollider() {}

		/**
		 * Create a box collider with an extent and a physics material
		 * @param ext the dimensions of the collider
		 * @param mat the physics material to assign
		 * @note The current scale of the transform is assumed to be the identity size for ResizeToFit.
		 */
        BoxCollider(PhysicsBodyComponent* owner, const vector3& ext, Ref<PhysicsMaterial> mat, const vector3& position = vector3(0, 0, 0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0));
		
		/**
		 Draw a wireframe shape representing the boundary of this collider
		 @param color the hex color to use to draw, in format 0xRRGGBBAA
		 */
		void DebugDraw(RavEngine::DebugDrawer& dbg) const override;

	};

	class SphereCollider : public PhysicsCollider {
	protected:
		decimalType radius;
	public:
		
		/**
		 Create a sphere collider with a material
		 @param r radius of the collider
		 @param mat the physics material of the shape
		 @param position the relative position of the shape
		 @param rotation the relative rotation of the shape
		 */
        SphereCollider(PhysicsBodyComponent* owner, decimalType radius, Ref<PhysicsMaterial> mat, const vector3& position = vector3(0,0,0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0));
		
		/**
		 Draw a wireframe shape representing the boundary of this collider
		 @param color the hex color to use to draw, in format 0xRRGGBBAA
		 */
		void DebugDraw(RavEngine::DebugDrawer& dbg) const override;
	};

	class CapsuleCollider : public PhysicsCollider {
	protected:
		decimalType radius;
		decimalType halfHeight;
	public:
		/**
		 Create a capsule collider with a material
		 @param r radius of the collider
		 @param h the half-height of the collider
		 @param mat the physics material of the shape
		 @param position the relative position of the shape
		 @param rotation the relative rotation of the shape
		 */
        CapsuleCollider(PhysicsBodyComponent* owner, decimalType r, decimalType hh, Ref<PhysicsMaterial> mat, const vector3& position = vector3(0,0,0), const quaternion& rotation = quaternion(1.0, 0.0, 0.0, 0.0));
		
		/**
		 Draw a wireframe shape representing the boundary of this collider
		 @param color the hex color to use to draw, in format 0xRRGGBBAA
		 */
		void DebugDraw(RavEngine::DebugDrawer& dbg) const override;
	};

	struct MeshCollider : public PhysicsCollider {
		/**
		 Create a MeshCollider given a MeshAsset physics material
		 @param mesh the MeshAsset to use
		 @param mat the PhysicsMaterial to use
		 */
        MeshCollider(PhysicsBodyComponent* owner, Ref<MeshAsset> mesh, Ref<PhysicsMaterial> mat);
		
		void DebugDraw(RavEngine::DebugDrawer& dbg) const override{
			//TODO: debug draw mesh collider
		}
	};

	struct ConvexMeshCollider : public PhysicsCollider {
		
		/**
		 Create a Convex Mesh Collider given a MeshAsset physics material
		 @param mesh the MeshAsset to use
		 @param mat the PhysicsMaterial to use
		 */
        ConvexMeshCollider(PhysicsBodyComponent*, Ref<MeshAsset> mesh, Ref<PhysicsMaterial> mat);
		
		void DebugDraw(RavEngine::DebugDrawer& dbg) const override{
			//TODO: debug draw mesh collider
		}
	};
}
