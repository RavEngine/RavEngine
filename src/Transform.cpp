#include "Transform.hpp"
#include "Entity.hpp"
#include "mathtypes.hpp"
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

	//figure out the size
	unsigned short depth = 0;
	for(WeakRef<Transform> p = parent; !p.isNull(); p = p.get()->parent){
		depth++;
	}

#if defined __APPLE__ || __STDC_VERSION__ >= 199901L		//Check for C99
	matrix4 transforms[depth];		//prefer C VLA on supported systems
#else
	matrix4* transforms = (matrix4*)alloca(sizeof(matrix4) * depth);	//warning: alloca may not be supported in the future
#endif


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
