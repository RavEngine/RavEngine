#pragma once
//
//  Transform.h
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. 
//

#include "Atomic.hpp"
#include "mathtypes.hpp"
#include "ComponentHandle.hpp"
#include "Queryable.hpp"
#include "Common3D.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "DataStructures.hpp"
#include "ComponentWithOwner.hpp"

namespace RavEngine {
	struct Transform : public ComponentWithOwner, public Queryable<Transform> {
    protected:
        mutable matrix4 matrix;
        quaternion rotation;
        vector3 position, scale;
        UnorderedVector<ComponentHandle<Transform>>  children;        //non-owning
        ComponentHandle<Transform> parent;    //non-owning
        mutable bool isDirty = false;		// used for when the transform hierarchy has been changed

		friend class World;
		mutable bool isTickDirty = false;	// used for when this transform has been updated in the current tick and needs updating in the world's render data
        
        inline void MarkAsDirty(Transform* root) const{
            root->isDirty = true;
			root->isTickDirty = true;
            
            for(auto& t : root->children){
                MarkAsDirty(t.get());
            }
        }

		inline void ClearTickDirty() {
			isTickDirty = false;
		}

	public:
		virtual ~Transform(){}
		Transform(entity_t owner, const vector3& inpos, const quaternion& inrot, const vector3& inscale) : ComponentWithOwner(owner){
            matrix = matrix4(1);
			SetLocalPosition(inpos);
			SetLocalRotation(inrot);
			SetLocalScale(inscale);
		}
		Transform(entity_t owner) : Transform(owner, vector3(0, 0, 0), quaternion(1.0, 0.0, 0.0, 0.0), vector3(1, 1, 1)) {}
        
		Transform& SetLocalPosition(const vector3&);
        Transform& SetWorldPosition(const vector3&);
        Transform& LocalTranslateDelta(const vector3&);
        Transform& WorldTranslateDelta(const vector3&);

        Transform& SetLocalRotation(const quaternion&);
        Transform& SetWorldRotation(const quaternion&);
        Transform& LocalRotateDelta(const quaternion&);

        Transform& SetLocalScale(const vector3&);
        Transform& LocalScaleDelta(const vector3&);
        
        Transform& SetLocalScale(const decimalType& scale){ SetLocalScale(vector3(scale,scale,scale)); return *this;}

		vector3 Forward() const;
		vector3 Right() const;
		vector3 Up() const;
		
		vector3 WorldForward() const;
		vector3 WorldRight() const;
		vector3 WorldUp() const;

		inline bool HasParent() const{
			return parent.IsValid();
		}

		vector3 GetLocalPosition() const;
		vector3 GetWorldPosition() const;

		quaternion GetLocalRotation() const;
		quaternion GetWorldRotation() const;

		vector3 GetLocalScale() const;

		matrix4 GenerateLocalMatrix() const;
		
		matrix4 GetMatrix() const;

		/**
		Get the matrix list of all the parents. 
		@param list the list to add the matrices to
		*/
		matrix4 CalculateWorldMatrix() const;

		/**
		Add a transform as a child object of this transform
		@param child weak reference to the child object
		*/
        Transform& AddChild(ComponentHandle<Transform> child);

		/**
		Remove a transform as a child object of this transform. This does not check if the passed object is actually a child.
		@param child weak reference to the child object
		*/
        Transform& RemoveChild(ComponentHandle<Transform> child);
        
        const decltype(children)& GetChildren() const{
            return children;
        }
        
        // destroy everything parented to this
        void Destroy(){
            for(auto& child : children){
                child.GetOwner().Destroy();
            }
        }
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

	inline vector3 Transform::WorldForward() const{
		return GetWorldRotation() * vector3_forward;
	}

	inline vector3 Transform::WorldRight() const{
		return GetWorldRotation() * vector3_right;
	}

	inline vector3 Transform::WorldUp() const{
		return GetWorldRotation() * vector3_up;
	}

	/**
	Translate the transform in a direction in local (parent) space. This will add to its current position
	@param delta the change to apply
	*/
	inline Transform& Transform::LocalTranslateDelta(const vector3& delta) {
		//set position value
		MarkAsDirty(this);
		position += delta;
        return *this;
	}

	inline Transform& Transform::WorldTranslateDelta(const vector3& delta){
		SetWorldPosition(GetWorldPosition() + delta);
        return *this;
	}

	/**
	Overwrite the position of the transform with a new position in local (parent) space
	@param newPos the new position of this transform
	*/
	inline Transform& Transform::SetLocalPosition(const vector3& newPos) {
		//set position value
		MarkAsDirty(this);
		position = newPos;
        return *this;
	}

	/**
	Move this transform to a new location in world space
	@param newPos the new position in world space.
	*/
	inline Transform& Transform::SetWorldPosition(const vector3& newPos) {
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
        return *this;
	}

	/**
	Overwrite the rotation of this transform in local (parent) space.
	@param newRot the new rotation to set
	*/
	inline Transform& Transform::SetLocalRotation(const quaternion& newRot) {
		MarkAsDirty(this);
		rotation = newRot;
        return *this;
	}

	/**
	Additively apply a rotation to this transform in local (parent) space.
	@param delta the change in rotation to apply
	*/
	inline Transform& Transform::LocalRotateDelta(const quaternion& delta) {
		MarkAsDirty(this);
		//sum two quaternions by multiplying them
		quaternion finalrot;
		vector3 t;
		vector4 p;
		glm::decompose(glm::toMat4((quaternion)rotation) * glm::toMat4(delta), t, finalrot, t, t, p);
		rotation = finalrot;
        return *this;
	}

	/**
	Overwrite the rotation of this transform in world space
	@param newRot the new rotation in world space
	*/
	inline Transform& Transform::SetWorldRotation(const quaternion& newRot) {
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
        return *this;
	}

	/**
	Overwrite the scale of this object with a new scale
	@param newScale the new size of this object in local (parent) space
	*/
	inline Transform& Transform::SetLocalScale(const vector3& newScale) {
		MarkAsDirty(this);
		scale = newScale;
        return *this;
	}

	inline Transform& Transform::LocalScaleDelta(const vector3& delta) {
		MarkAsDirty(this);
		scale += delta;
        return *this;
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
		if (isDirty){
			//figure out the size
			unsigned short depth = 0;
            for(auto p = parent; p.IsValid(); p = p->parent){
				depth++;
			}
			
			stackarray(transforms, matrix4, depth);
			
			int tmp = 0;
			for(auto p = parent; p.IsValid(); p = p->parent){
				transforms[tmp] = p->GenerateLocalMatrix();
				++tmp;
			}
			
			matrix4 mat(1);
			for(int i = depth - 1; i >= 0; --i){
				mat *= transforms[i];
			}
			mat *= GenerateLocalMatrix();
			matrix = mat;
			isDirty = false;
			return mat;
		}
		else{
			return matrix;
		}
	}

	/**
	 @return the current cached matrix, representing the world-space transformation. This may be out-of-date.
	 See CalculateWorldMatrix to calculate if out-of-date.
	 */
	inline matrix4 Transform::GetMatrix() const {
		return matrix;
	}

	inline vector3 Transform::GetWorldPosition() const
	{
		if (!HasParent()) {
			return GetLocalPosition();
		}
		auto finalMatrix = CalculateWorldMatrix();
		
		//finally apply the local matrix
		return finalMatrix * vector4(0,0,0, 1);
	}

	inline quaternion Transform::GetWorldRotation() const
	{
		if (!HasParent()) {
			return GetLocalRotation();
		}
		
		//apply local rotation
		auto finalMatrix = CalculateWorldMatrix();
		
		//decompose the matrix to extract the rotation
		quaternion finalrot;
		
		vector3 t;
		vector4 p;
		glm::decompose(finalMatrix, t, finalrot, t, t, p);
		return finalrot;
	}
}
