//
//  Transform.h
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "Component.hpp"
#include <atomic>
#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_set>
#include "mathtypes.hpp"
#include "WeakRef.hpp"
#include <utils/Entity.h>


namespace RavEngine {
	/**
	 A thread-safe transform component
	 */
	class Transform : public Component {
	public:
		typedef std::unordered_set<WeakRef<Transform>> childStore;
		virtual ~Transform() {}
		Transform(const vector3& inpos, const quaternion& inrot, const vector3& inscale, bool inStatic = false);
		Transform() : Transform(vector3(0, 0, 0), quaternion(1.0, 0.0, 0.0, 0.0), vector3(1, 1, 1)) {}

		void SetLocalPosition(const vector3&);
		void SetWorldPosition(const vector3&);
		void LocalTranslateDelta(const vector3&);

		void SetLocalRotation(const quaternion&);
		void SetWorldRotation(const quaternion&);
		void LocalRotateDelta(const quaternion&);

		void SetLocalScale(const vector3&);
		void LocalScaleDelta(const vector3&);

		vector3 Forward();
		vector3 Right();
		vector3 Up();

		bool HasParent();

		vector3 GetLocalPosition();
		vector3 GetWorldPosition();

		quaternion GetLocalRotation();
		quaternion GetWorldRotation();

		vector3 GetLocalScale();
		vector3 GetWorldScale();

		matrix4 GetMatrix() {
			return matrix.load();
		}

		/**
		Apply cached transformations to filament entity - for internal use only
		*/
		void Apply();

		/**
		Get the matrix list of all the parents. This does NOT include the current transform.
		@param list the list to add the matrices to
		*/
		void GetParentMatrixStack(std::list<matrix4>& list) const;

		/**
		Add a transform as a child object of this transform
		@param child weak reference to the child object
		*/
		void AddChild(const WeakRef<Transform>& child);

		/**
		Remove a transform as a child object of this transform. This does not check if the passed object is actually a child.
		@param child weak reference to the child object
		*/
		void RemoveChild(const WeakRef<Transform>& child);

		/**
		@returns Filament type, for internal use only
		*/
		const utils::Entity& getEntity() {
			return filamentEntity;
		}

	protected:
		std::atomic<vector3> position;
		std::atomic<quaternion> rotation;
		std::atomic<vector3> scale;
		std::atomic<matrix4> matrix;

		utils::Entity filamentEntity;

		bool isStatic = false;

		childStore children;		//non-owning
		WeakRef<Transform> parent;	//non-owning
	};

	/**
	@return the vector pointing in the forward direction of this transform
	*/
	inline vector3 Transform::Forward() {
		return rotation.load() * vector3_forward;
	}

	/**
	@return the vector pointing in the up direction of this transform
	*/
	inline vector3 Transform::Up() {
		return rotation.load() * vector3_up;
	}

	/**
	@return the vector pointing in the right direction of this transform
	*/
	inline vector3 Transform::Right() {
		return rotation.load() * vector3_right;
	}

	/**
	Translate the transform in a direction in local (parent) space. This will add to its current position
	@param delta the change to apply
	*/
	inline void Transform::LocalTranslateDelta(const vector3& delta) {
		//set position value
		position.store(position.load() + delta);

		//apply transform
		matrix.store(glm::translate(matrix.load(), delta));
	}

	/**
	Overwrite the position of the transform with a new position in local (parent) space
	@param newPos the new position of this transform
	*/
	inline void Transform::SetLocalPosition(const vector3& newPos) {
		//apply transform (must translate to origin and then to new position)
		//auto newMat = glm::translate(matrix.load(), vector3(3, 9, 5));
		auto newMat = glm::translate(glm::translate(matrix.load(), -position.load()), newPos);	//undo the old move, then do the new move

		//set position value
		position.store(newPos);
		matrix.store(newMat);
	}

	/**
	Move this transform to a new location in world space
	@param newPos the new position in world space.
	*/
	inline void Transform::SetWorldPosition(const vector3& newPos) {
		if (!HasParent()) {
			SetLocalPosition(newPos);
		}
		else {
			//get the world position 
			auto worldpos = GetWorldPosition();
			//get the displacement to the new world coords from the current local position
			auto displacement = newPos - worldpos;
			//update the local pos to that displacement
			LocalTranslateDelta(displacement);
		}
	}

	/**
	Overwrite the rotation of this transform in local (parent) space.
	@param newRot the new rotation to set
	*/
	inline void Transform::SetLocalRotation(const quaternion& newRot) {
		//create transformation matrix back to origin
		auto pos = GetLocalPosition();
		matrix4 toOrigin(1), toPos(1);
		toOrigin = glm::translate(toOrigin, -pos);
		matrix4 undorotate = glm::toMat4(glm::conjugate(rotation.load()));
		matrix4 rotate = glm::toMat4(newRot);
		toPos = glm::translate(toPos, pos);

		//undo the current rotation, then apply the new rotation
		auto allMatrix = toOrigin * undorotate * rotate * toPos;

		matrix.store(matrix.load() * allMatrix);

		rotation.store(newRot);
	}

	/**
	Additively apply a rotation to this transform in local (parent) space.
	@param delta the change in rotation to apply
	*/
	inline void Transform::LocalRotateDelta(const quaternion& delta) {
		//create transformation matrix back to origin
		auto pos = GetLocalPosition();
		matrix4 toOrigin(1), toPos(1);
		toOrigin = glm::translate(toOrigin, -pos);
		matrix4 rotate = glm::toMat4(delta);
		toPos = glm::translate(toPos, pos);

		auto allMatrix = toOrigin * rotate * toPos;

		auto appliedRot = matrix.load() * rotate;
		matrix.store(matrix.load() * allMatrix);
		rotation.store(glm::toQuat(appliedRot));
	}

	/**
	Overwrite the rotation of this transform in world space
	@param newRot the new rotation in world space
	*/
	inline void Transform::SetWorldRotation(const quaternion& newRot) {
		if (!HasParent()) {
			SetLocalRotation(newRot);
		}
		else {
			//get world rotation
			auto worldRot = GetWorldRotation();
			//get the relative world offset
			auto relative = glm::inverse(worldRot) * newRot;
			//apply the rotation
			LocalRotateDelta(relative);
		}
	}

	/**
	Overwrite the scale of this object with a new scale
	@param newScale the new size of this object in local (parent) space
	*/
	inline void Transform::SetLocalScale(const vector3& newScale) {
		//must undo current scale then scale to new size
		matrix.store(glm::scale(glm::scale(matrix.load(), -scale.load()), newScale));
		scale.store(newScale);
	}

	inline void Transform::LocalScaleDelta(const vector3& delta) {
		scale.store(scale.load() + delta);
		matrix.store(glm::scale(matrix.load(), delta));
	}

	inline vector3 Transform::GetLocalPosition()
	{
		return position.load();
	}

	inline quaternion Transform::GetLocalRotation()
	{
		return rotation.load();
	}

	inline vector3 Transform::GetLocalScale()
	{
		return scale.load();
	}
}