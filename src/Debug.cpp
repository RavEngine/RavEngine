#include "Debug.hpp"
#include <im3d.h>
#include <glm/gtc/type_ptr.hpp>

using namespace RavEngine;
using namespace std;


SpinLock DebugDraw::mtx;
vector<DebugDraw::drawinst> DebugDraw::instances;

/**
 Matrix class converison
 @param m matrix input
 @return Im3d::Mat4 representation
 */
static inline Im3d::Mat4 matrix4ToMat4(const matrix4& m){
	auto p = glm::value_ptr(m);
	
	return Im3d::Mat4(p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14]);
}


void DebugDraw::DrawRectangularPrism(const matrix4 &transform, const vector3 &color){
	//create matrix from transform
	
	//bind color uniform and set value
	
	//execute draw call
}

void DebugDraw::DrawCylinder(const matrix4 &transform, const vector3 &c,decimalType radius, decimalType height){
	mtx.lock();
	instances.emplace_back(transform, c);
	Im3d::DrawCylinder(Im3d::Vec3(0,0,0), Im3d::Vec3(0,height,0), radius);
	mtx.unlock();
}

void DebugDraw::DrawSphere(const matrix4 &transform, const vector3 &c, decimalType radius){
	mtx.lock();
	instances.emplace_back(transform,c);
	Im3d::DrawSphere(Im3d::Vec3(0,0,0), radius);
	mtx.unlock();
}

void DebugDraw::Reset(){
	instances.clear();
}

const DebugDraw::drawinst DebugDraw::InstanceAt(size_t index){
	return instances.at(index);
}
