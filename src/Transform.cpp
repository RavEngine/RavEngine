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
	SetLocalPosition(inpos);
	SetLocalRotation(inrot);
	SetLocalScale(inscale);
	isStatic = inStatic;

	Apply();
}

matrix4 Transform::CalculateWorldMatrix() const{	

	list<matrix4> matrix;
	
	WeakRef p = parent;
	
	//get all the transforms by navigating up the hierarchy
	while (!p.isNull()) {
		Ref<Transform> e(p);
		matrix.push_front(e->GenerateLocalMatrix());
		p = e->parent;
	}
	
	matrix4 finalMatrix(1);
	for (auto& transform : matrix) {
		finalMatrix *= transform;
	}
	finalMatrix *= GenerateLocalMatrix();
	
	return finalMatrix;
}

void Transform::AddChild(const WeakRef<Transform>& child)
{
	Ref<Transform> ctrans(child);
	auto worldPos = ctrans->GetWorldPosition();
	auto worldRot = ctrans->GetWorldRotation();
	
	ctrans->parent = WeakRef<Transform>(this);
	children.insert(child);
	
	ctrans->SetWorldPosition(worldPos);
	ctrans->SetWorldRotation(worldRot);
}

void Transform::RemoveChild(const WeakRef<Transform>& child)
{
	Ref<Transform> ctrans(child);
	auto worldPos = ctrans->GetWorldPosition();
	auto worldRot = ctrans->GetWorldRotation();
	ctrans->parent = nullptr;
	children.erase(child);
	ctrans->SetWorldPosition(worldPos);
	ctrans->SetWorldRotation(worldRot);
}

vector3 Transform::GetWorldPosition()
{
	if (!HasParent()) {
		return GetLocalPosition();
	}
	auto finalMatrix = CalculateWorldMatrix();
	
	//finally apply the local matrix
	return finalMatrix * vector4(GetLocalPosition(), 1);
}

quaternion Transform::GetWorldRotation()
{	
	if (!HasParent()) {
		return GetLocalRotation();
	}
	
	//apply local rotation
	auto finalRot = CalculateWorldMatrix();

	//return the result as a quaternion
	return glm::toQuat(finalRot);
}

void RavEngine::Transform::Apply()
{
	matrix = CalculateWorldMatrix();
}

bool Transform::HasParent() {
	return !parent.isNull();
}
