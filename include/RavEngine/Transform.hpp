//
//  Transform.h
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. 
//

#pragma once
#include "Component.hpp"
#include "Atomic.hpp"
#include <array>
#include <phmap.h>
#include "mathtypes.hpp"
#include "WeakRef.hpp"
#include "Queryable.hpp"
#include "Common3D.hpp"

namespace RavEngine {
	/**
	 A thread-safe transform component
	 */
	class Transform : public Component, public Queryable<Transform> {
	public:
		typedef phmap::flat_hash_set<WeakRef<Transform>> childStore;
		virtual ~Transform(){}
		Transform(const vector3& inpos, const quaternion& inrot, const vector3& inscale, bool inStatic = false){
			matrix = matrix4(1);
			SetLocalPosition(inpos);
			SetLocalRotation(inrot);
			SetLocalScale(inscale);
			isStatic = inStatic;
			
			Apply();
		}
		Transform() : Transform(vector3(0, 0, 0), quaternion(1.0, 0.0, 0.0, 0.0), vector3(1, 1, 1)) {}

		void SetLocalPosition(const vector3&);
		void SetWorldPosition(const vector3&);
		void LocalTranslateDelta(const vector3&);

		void SetLocalRotation(const quaternion&);
		void SetWorldRotation(const quaternion&);
		void LocalRotateDelta(const quaternion&);

		void SetLocalScale(const vector3&);
		void LocalScaleDelta(const vector3&);

		vector3 Forward() const;
		vector3 Right() const;
		vector3 Up() const;

		inline bool HasParent() const{
			return !parent.isNull();
		}

		vector3 GetLocalPosition() const;
		vector3 GetWorldPosition() const;

		quaternion GetLocalRotation() const;
		quaternion GetWorldRotation() const;

		vector3 GetLocalScale() const;

		matrix4 GenerateLocalMatrix() const;

		/**
		Apply cached transformations to matrix - for internal use only
		*/
		inline void Apply(){
			matrix = CalculateWorldMatrix();
		}

		/**
		Get the matrix list of all the parents. This does NOT include the current transform.
		@param list the list to add the matrices to
		*/
		matrix4 CalculateWorldMatrix() const;

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
		@returns the currently stored world matrix. This is not guarenteed to be up-to-date. 
		To update it, call Apply()
		*/
		inline matrix4 GetCurrentWorldMatrix() const{
			return matrix;
		}

	protected:
		Atomic<vector3> position;
		Atomic<quaternion> rotation;
		Atomic<vector3> scale;
		Atomic<matrix4> matrix;

		bool isStatic = false;

		childStore children;		//non-owning
		WeakRef<Transform> parent;	//non-owning
};

	/**
	Construct a transformation matrix out of this transform
	@return glm matrix representing this transform
	*/
	inline matrix4 Transform::GenerateLocalMatrix() const{
		return glm::translate(matrix4(1), (vector3)position) * glm::toMat4((quaternion)rotation) * glm::scale(matrix4(1), (vector3)scale);
	}

	/**
	@return the vector pointing in the forward direction of this transform
	*/
	inline vector3 Transform::Forward() const{
		return (quaternion)rotation * vector3_forward;
	}

	/**
	@return the vector pointing in the up direction of this transform
	*/
	inline vector3 Transform::Up() const{
		return (quaternion)rotation * vector3_up;
	}

	/**
	@return the vector pointing in the right direction of this transform
	*/
	inline vector3 Transform::Right() const{
		return (quaternion)rotation * vector3_right;
	}

	/**
	Translate the transform in a direction in local (parent) space. This will add to its current position
	@param delta the change to apply
	*/
	inline void Transform::LocalTranslateDelta(const vector3& delta) {
		//set position value
		position = (vector3)position + delta;
	}

	/**
	Overwrite the position of the transform with a new position in local (parent) space
	@param newPos the new position of this transform
	*/
	inline void Transform::SetLocalPosition(const vector3& newPos) {
		//set position value
		position = newPos;
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
		rotation = newRot;
	}

	/**
	Additively apply a rotation to this transform in local (parent) space.
	@param delta the change in rotation to apply
	*/
	inline void Transform::LocalRotateDelta(const quaternion& delta) {
		//sum two quaternions by multiplying them
		rotation = glm::toQuat(glm::toMat4((quaternion)rotation) * glm::toMat4(delta));
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
		scale = newScale;
	}

	inline void Transform::LocalScaleDelta(const vector3& delta) {
		scale = (vector3)scale + delta;
	}

	inline vector3 Transform::GetLocalPosition() const
	{
		return position;
	}

	inline quaternion Transform::GetLocalRotation() const
	{
		return rotation;
	}

	inline vector3 Transform::GetLocalScale() const
	{
		return scale;
	}

	inline matrix4 Transform::CalculateWorldMatrix() const{
		//figure out the size
		unsigned short depth = 0;
		for(WeakRef<Transform> p = parent; !p.isNull(); p = p.get()->parent){
			depth++;
		}
		
		stackarray(transforms, matrix4, depth);
		
		int tmp = 0;
		for(WeakRef<Transform> p = parent; !p.isNull(); p = p.get()->parent){
			transforms[tmp] = p.get()->GenerateLocalMatrix();
			++tmp;
		}
		
		matrix4 mat(1);
		for(int i = depth - 1; i >= 0; --i){
			mat *= transforms[i];
		}
		mat *= GenerateLocalMatrix();
		
		return mat;
	}

	inline vector3 Transform::GetWorldPosition() const
	{
		if (!HasParent()) {
			return GetLocalPosition();
		}
		auto finalMatrix = CalculateWorldMatrix();
		
		//finally apply the local matrix
		return finalMatrix * vector4(GetLocalPosition(), 1);
	}

	inline quaternion Transform::GetWorldRotation() const
	{
		if (!HasParent()) {
			return GetLocalRotation();
		}
		
		//apply local rotation
		auto finalRot = CalculateWorldMatrix();
		
		//return the result as a quaternion
		return glm::toQuat(finalRot);
	}
}
