#include "Transform.h"
#include "Entity.hpp"
#include <list>
#include "mathtypes.h"
#include <cassert>

using namespace std;
using namespace glm;

void Transform::GetParentMatrixStack(list<matrix4>& matrix) {
	WeakRef p = parent;

	//get all the transforms by navigating up the hierarchy
	while (!p.isNull()) {
		Ref<Transform> e(p);
		matrix.push_front(e->GetMatrix());
		p = e->parent;
	}
}

void Transform::AddChild(const WeakRef<Transform>& child)
{
	//must be passing a transform!
	Ref<Transform>(child)->parent = WeakRef<Transform>(this);
	children.insert(child);
}

void Transform::RemoveChild(const WeakRef<Transform>& child)
{
	//must be passing a transform!
	Ref<Transform>(child)->parent = nullptr;
	children.erase(child);
}

vector3 Transform::GetWorldPosition()
{
	if (!HasParent()) {
		return GetLocalPosition();
	}

	//the list of transformations to apply
	list<matrix4> translations;
	GetParentMatrixStack(translations);

	//apply all the transforms
	vector4 finalPos(0,0,0,1);
	for (auto& transform : translations) {
		finalPos = transform * finalPos;
	}

	//finally apply the local matrix
	return GetMatrix() * finalPos;
}

quaternion Transform::GetWorldRotation()
{	
	if (!HasParent()) {
		return GetLocalRotation();
	}

	//the list of transformations to apply
	list<matrix4> rotations;
	GetParentMatrixStack(rotations);

	matrix4 finalRot = matrix4(1.0);	//construct identity matrix
	//apply all the transforms
	for (auto& transform : rotations) {
		finalRot = transform * finalRot;
	}

	//apply local rotation
	finalRot = glm::toMat4(GetLocalRotation()) * finalRot;

	//return the result as a quaternion
	return glm::toQuat(finalRot);
}

bool Transform::HasParent() {
	return !parent.isNull();
}
