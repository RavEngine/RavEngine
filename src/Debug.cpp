#include "Debug.hpp"
#include <im3d.h>
#include <glm/gtc/type_ptr.hpp>

using namespace RavEngine;
using namespace std;


SpinLock DebugDraw::mtx;


/**
 Matrix class converison
 @param m matrix input
 @return Im3d::Mat4 representation
 */
static inline Im3d::Mat4 matrix4ToMat4(const matrix4& m){
	//need to transpose
	auto transposed = glm::transpose(m);
	auto p = glm::value_ptr(transposed);
	
	return Im3d::Mat4(p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14]);
}


void DebugDraw::DrawRectangularPrism(const matrix4 &transform, const vector3 &c, const vector3& d){
	DrawHelper(transform, [&]{
		Im3d::SetColor(Im3d::Color(c.x,c.y,c.z));
		Im3d::DrawAlignedBox(Im3d::Vec3(-d.x/2,-d.y/2,-d.z/2), Im3d::Vec3(d.x/2,d.y/2,d.z/2));
	});
}

void DebugDraw::DrawCylinder(const matrix4 &transform, const vector3 &c,decimalType radius, decimalType height){
	DrawHelper(transform, [&]{
		Im3d::SetColor(Im3d::Color(c.x,c.y,c.z));
		Im3d::DrawCylinder(Im3d::Vec3(0,0,0), Im3d::Vec3(0,height,0), radius);
	});
}

void DebugDraw::DrawSphere(const matrix4 &transform, const vector3 &c, decimalType radius){
	DrawHelper(transform, [&]{
		Im3d::SetColor(Im3d::Color(c.x,c.y,c.z));
		Im3d::DrawSphere(Im3d::Vec3(0,0,0), radius);
	});
}

void DebugDraw::DrawHelper(const matrix4 &transform, std::function<void()> impl){
	mtx.lock();
	Im3d::PushMatrix(matrix4ToMat4(transform));
	impl();
	Im3d::PopMatrix();
	mtx.unlock();
}
