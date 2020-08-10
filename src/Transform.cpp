#include "Transform.hpp"
#include "Entity.hpp"
#include <list>
#include "mathtypes.hpp"
#include "GameplayStatics.hpp"
#include <cassert>
#include "OgreStatics.hpp"

using namespace std;
using namespace glm;

TransformComponent::TransformComponent(const vector3& inpos, const quaternion& inrot, const vector3& inscale, bool inStatic) {
	sceneNode = GameplayStatics::ogreFactory.createSceneNode();
	matrix.store(matrix4(1.0));
	LocalTranslateDelta(inpos);
	LocalRotateDelta(inrot);
	LocalScaleDelta(inscale);
	isStatic = inStatic;
}

void TransformComponent::GetParentMatrixStack(list<matrix4>& matrix) const{
	WeakRef p = parent;

	//get all the transforms by navigating up the hierarchy
	while (!p.isNull()) {
		Ref<TransformComponent> e(p);
		matrix.push_front(e->GetMatrix());
		p = e->parent;
	}
}

void TransformComponent::AddChild(const WeakRef<TransformComponent>& child)
{
	//must be passing a transform!
	Ref<TransformComponent>(child)->parent = WeakRef<TransformComponent>(this);
	children.insert(child);
}

void TransformComponent::RemoveChild(const WeakRef<TransformComponent>& child)
{
	//must be passing a transform!
	Ref<TransformComponent>(child)->parent = nullptr;
	children.erase(child);
}

vector3 TransformComponent::GetWorldPosition()
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

quaternion TransformComponent::GetWorldRotation()
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

vector3 TransformComponent::GetWorldScale()
{
	if (!HasParent()) {
		return GetLocalScale();
	}
	//list of transforms
	list<matrix4> scales;
	GetParentMatrixStack(scales);

	//apply all the transforms
	vector4 finalPos(0, 0, 0, 1);
	for (auto& transform : scales) {
		finalPos = transform * finalPos;
	}

	//finally apply the local matrix
	return GetMatrix() * finalPos;
}

bool TransformComponent::HasParent() {
	return !parent.isNull();
}
