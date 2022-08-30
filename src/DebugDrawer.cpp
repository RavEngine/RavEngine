#include "DebugDrawer.hpp"
#include <im3d.h>
#include "Function.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "PhysXDefines.h"
#include "Debug.hpp"
#include <bgfx/bgfx.h>
#include <MeshAsset.hpp>
#include <RenderEngine.hpp>

using namespace RavEngine;
using namespace std;

// defined in RenderEngine.cpp
extern bgfx::ProgramHandle rve_debugShaderHandle;

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

void DebugDrawer::DrawRectangularPrism(const matrix4 &transform, const color_t c, const vector3& d){
#ifndef NDEBUG
	DrawHelper(transform, [=]{
		Im3d::SetColor(c);
		Im3d::DrawAlignedBox(Im3d::Vec3(-d.x/2,-d.y/2,-d.z/2), Im3d::Vec3(d.x/2,d.y/2,d.z/2));
	});
#endif
}

void DebugDrawer::DrawCylinder(const matrix4 &transform, const color_t c,decimalType radius, decimalType height){
#ifndef NDEBUG
	DrawHelper(transform, [=]{
		Im3d::SetColor(c);
		Im3d::DrawCylinder(Im3d::Vec3(0,0,0), Im3d::Vec3(0,height,0), radius);
	});
#endif
}

void DebugDrawer::DrawSphere(const matrix4 &transform, const color_t c, decimalType radius){
#ifndef NDEBUG
	DrawHelper(transform, [=]{
		Im3d::SetColor(c);
		Im3d::DrawSphere(Im3d::Vec3(0,0,0), radius);
	});
#endif
}

void DebugDrawer::DrawCapsule(const matrix4 &transform, const color_t color, decimalType radius, decimalType height){
#ifndef NDEBUG
	DrawHelper(transform, [=]{
        Im3d::SetColor(color);
        Im3d::DrawCapsule(Im3d::Vec3(0,0,0), Im3d::Vec3(0,height,0), radius);
    });
#endif
}

void DebugDrawer::DrawPrism(const matrix4 &transform, const color_t color, decimalType radius, decimalType height, decimalType sides){
#ifndef NDEBUG
	DrawHelper(transform, [=] {
		Im3d::SetColor(color);
		Im3d::DrawPrism(Im3d::Vec3(0, 0, 0), Im3d::Vec3(0, height, 0), radius, sides);
	});
#endif

}

void DebugDrawer::DrawArrow(const vector3 &start, const vector3 &end, const color_t color){
#ifndef NDEBUG
	mtx.lock();
    Im3d::SetColor(color);
    Im3d::DrawArrow(Im3d::Vec3(start.x,start.y,start.z), Im3d::Vec3(end.x,end.y,end.z));
	mtx.unlock();
#endif
}


void DebugDrawer::DrawHelper(const matrix4 &transform, Function<void()> impl){
#ifndef NDEBUG
	mtx.lock();
	Im3d::PushMatrix(matrix4ToMat4(transform));
	impl();
	Im3d::PopMatrix();
	mtx.unlock();
#endif
}

void DebugDrawer::DrawWireframeMesh(const matrix4& transform, const Ref<MeshAsset>& mesh) {
#ifndef NDEBUG
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA | BGFX_STATE_PT_LINES);
	float mtx[16];
	copyMat4(glm::value_ptr(transform), mtx);
	bgfx::setTransform(mtx);
	bgfx::setVertexBuffer(0,mesh->getVertexBuffer());
	bgfx::setIndexBuffer(mesh->getIndexBuffer());
	bgfx::submit(RenderEngine::Views::FinalBlit, rve_debugShaderHandle);
	bgfx::discard();
#endif
}
