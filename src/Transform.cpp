#include "Transform.hpp"
#include "Entity.hpp"
#include <list>
#include "mathtypes.hpp"
#include "GameplayStatics.hpp"
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <RenderEngine.hpp>
#include <filament/Engine.h>
#include <filament/TransformManager.h>
#include <glm/gtc/type_ptr.hpp>


using namespace std;
using namespace glm;
using namespace RavEngine;
using namespace utils;
using namespace filament::math;

Transform::Transform(const vector3& inpos, const quaternion& inrot, const vector3& inscale, bool inStatic) {
	filamentEntity = EntityManager::get().create();

	LocalTranslateDelta(inpos);
	LocalRotateDelta(inrot);
	LocalScaleDelta(inscale);
	isStatic = inStatic;

	Apply();
}

void Transform::GetParentMatrixStack(list<matrix4>& matrix) const{
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
	matrix4 finalMatrix(1);
	for (auto& transform : translations) {
		finalMatrix *= transform;
	}
	finalMatrix *= GetMatrix();

	//finally apply the local matrix
	return finalMatrix * vector4(GetLocalPosition(), 1);
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
		finalRot *= transform;
	}

	//apply local rotation
	finalRot *= glm::toMat4(GetLocalRotation());

	//return the result as a quaternion
	return glm::toQuat(finalRot);
}

void RavEngine::Transform::Apply()
{
	auto& tcm = RenderEngine::getEngine()->getTransformManager();
	auto instance = tcm.getInstance(filamentEntity);

	//the list of transformations to apply
	list<matrix4> translations;
	GetParentMatrixStack(translations);

	//apply all the transforms
	matrix4 finalMatrix(1);
	for (auto& transform : translations) {
		finalMatrix *= transform;
	}
	finalMatrix *= GetMatrix();

	decimalType dArray[16] = { 0.0 };

	const decimalType* pSource = (const decimalType*)glm::value_ptr(finalMatrix);
	for (int i = 0; i < 16; ++i)
		dArray[i] = pSource[i];

	//copy glm matrix to filament matrix
	tcm.setTransform(instance, filmat4(dArray[0], dArray[1], dArray[2], dArray[3], dArray[4], dArray[5], dArray[6], dArray[7], dArray[8], dArray[9], dArray[10], dArray[11], dArray[12], dArray[13], dArray[14], dArray[15]));

}

bool Transform::HasParent() {
	return !parent.isNull();
}
