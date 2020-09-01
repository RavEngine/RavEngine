#include "Transform.hpp"
#include "Entity.hpp"
#include <list>
#include "mathtypes.hpp"
#include "GameplayStatics.hpp"
#include <RenderEngine.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace std;
using namespace glm;
using namespace RavEngine;
using namespace utils;

RavEngine::Transform::~Transform()
{
}

Transform::Transform(const vector3& inpos, const quaternion& inrot, const vector3& inscale, bool inStatic) {

	matrix = matrix4(1);
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
		matrix.push_front(e->GenerateLocalMatrix());
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
	finalMatrix *= GenerateLocalMatrix();

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

	//the list of transformations to apply
	list<matrix4> translations;
	GetParentMatrixStack(translations);

	//apply all the transforms
	matrix4 finalMatrix(1);
	for (auto& transform : translations) {
		finalMatrix *= transform;
	}
	finalMatrix *= GenerateLocalMatrix();
	matrix = finalMatrix;
}

bool Transform::HasParent() {
	return !parent.isNull();
}
