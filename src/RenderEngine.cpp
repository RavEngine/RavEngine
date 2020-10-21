//
//  RenderEngine.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "RenderEngine.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "GameplayStatics.hpp"
#include "CameraComponent.hpp"
#include "World.hpp"
#include <memory>
#include "Material.hpp"
#include <bgfx/platform.h>
#include <bx/bx.h>

#include "RenderableComponent.hpp"
#include <SDL_syswm.h>
#include <SDL.h>
#include <SDL_video.h>
#include <im3d.h>
#include <BuiltinMaterials.hpp>
#include "Common3D.hpp"
#include "Debug.hpp"
#include <glm/gtc/type_ptr.hpp>

#ifdef __APPLE_
#include <Cocoa/Cocoa.h>
#endif

using namespace std;
using namespace RavEngine;

SDL_Window* RenderEngine::window = nullptr;
RenderEngine::vs RenderEngine::VideoSettings;

static Ref<DebugMaterialInstance> mat;

static bgfx::VertexLayout pcvDecl;

/**
 Create an SDL window for different platforms, and reference it to bgfx
 @param _window the SDL window pointer (modified)
 @note supported platforms: Linux, BSD, OSX, Windows, SteamLink
 */
inline bool sdlSetWindow(SDL_Window* _window)
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(_window, &wmi)) {
		return false;
	}
	
	bgfx::PlatformData pd;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	pd.ndt = wmi.info.x11.display;
	pd.nwh = (void*)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_OSX
	pd.ndt = NULL;
	pd.nwh = wmi.info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
	pd.ndt = NULL;
	pd.nwh = wmi.info.win.window;
#elif BX_PLATFORM_STEAMLINK
	pd.ndt = wmi.info.vivante.display;
	pd.nwh = wmi.info.vivante.window;
#endif // BX_PLATFORM_
	pd.context = NULL;
	pd.backBuffer = NULL;
	pd.backBufferDS = NULL;
	bgfx::setPlatformData(pd);
	
	return true;
}

void DebugRender(const Im3d::DrawList& drawList){
	switch(drawList.m_primType){
		case Im3d::DrawPrimitive_Triangles:
			//Set BGFX state to triangles
			bgfx::setState(BGFX_STATE_DEFAULT);
			break;
		case Im3d::DrawPrimitive_Lines:
			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA | BGFX_STATE_PT_LINES);
			//set BGFX state to lines
			break;
		case Im3d::DrawPrimitive_Points:
			//set BGFX state to points
			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA | BGFX_STATE_PT_POINTS);
			break;
		default:
			throw runtime_error("Invalid Im3d state");
			break;
	}
	//perform drawing here
	const Im3d::VertexData* vertexdata = drawList.m_vertexData;
	const size_t verts = drawList.m_vertexCount;
	
	vector<VertexColor> converted;
	converted.reserve(verts);
	
	for(int x = 0; x < verts; x++){
		Im3d::VertexData d = vertexdata[x];
		converted.push_back({d.m_positionSize.x,d.m_positionSize.y,d.m_positionSize.z,d.m_color});
	}
	vector<uint16_t> indices;
	for(int i = 0; i < verts; i++){
		indices.push_back(i);
	}
	
	bgfx::VertexBufferHandle vbuf = bgfx::createVertexBuffer(bgfx::copy(&converted[0], converted.size() * sizeof(VertexColor)), pcvDecl);
	bgfx::IndexBufferHandle ibuf = bgfx::createIndexBuffer(bgfx::copy(&indices[0], indices.size() * sizeof(uint16_t)));
	
	//auto data = DebugDraw::InstanceAt(i);
		
	auto& context = Im3d::GetContext();
		
	auto& drawmatrix = context.getMatrix();
	
	//matrix4 rvm = glm::transpose(glm::make_mat4(drawmatrix.m));
	
	mat->Draw(vbuf,ibuf,matrix4(1));
	bgfx::destroy(vbuf);
	bgfx::destroy(ibuf);
}

/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine() {
	//call Init()
	Init();
	mat = new DebugMaterialInstance(Material::Manager::AccessMaterialOfType<DebugMaterial>());
	auto& data = Im3d::GetAppData();
	data.drawCallback = &DebugRender;
	
	//vertex format
	pcvDecl.begin()
	.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
	.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
	.end();
	
}

RavEngine::RenderEngine::~RenderEngine()
{
}


/**
 Render one frame using the current state of every object in the world
 */
void RenderEngine::Draw(Ref<World> worldOwning){
	//debug draw
	//TODO: compile-out in release build
	//Im3d::Context& ctx = Im3d::GetContext();
	//Im3d::AppData& ad = Im3d::GetAppData();
	
	//Im3d::SetMatrix(Im3d::Mat4(1.0f));
	Im3d::SetSize(8.0f);
	DebugDraw::DrawSphere(Transformation({vector3(1,5,4)}), vector3(0,1,0), 3);
	DebugDraw::DrawCylinder(Transformation({vector3(-1,-5,-3)}), vector3(1,1,1), 5,10);

	//get the active camera
	auto components = worldOwning->Components();
	auto allcams = components.GetAllComponentsOfType<CameraComponent>();

	//set the view transform - all entities drawn will use this matrix
	for (auto& cam : allcams) {
		auto owning = Ref<CameraComponent>(cam);
		if (owning->isActive()) {
			int width,height;
			SDL_GL_GetDrawableSize(window, &width, &height);
			owning->SetTargetSize(width, height);
            Material::Manager::SetProjectionMatrix(cam->GenerateProjectionMatrix());
            Material::Manager::SetViewMatrix(cam->GenerateViewMatrix());
			break;
		}
	}

    //apply transforms for only entities that need to be rendered
    auto toDraw = components.GetAllComponentsOfSubclass<RenderableComponent>();
	
	//clear buffers

	bgfx::touch(0);
	bgfx::dbgTextClear();
	bgfx::dbgTextPrintf(0, 0, 0x4f, (string("RavEngine - ") + currentBackend()).c_str());

    //iterate through renderables and call Draw
    for (auto& e : toDraw) {
        e->Draw();
    }
	
	//TODO: compile-out in release build
	Im3d::EndFrame();
	Im3d::GetContext().draw();
	bgfx::frame();

	
	for(int i = 0, n = Im3d::GetDrawListCount(); i < n; ++i){
		
	}
	DebugDraw::Reset();
	Im3d::NewFrame();
	Im3d::AppData& data = Im3d::GetAppData();
	data.drawCallback = &DebugRender;

}

void RenderEngine::resize(){
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	bgfx::reset(width, height, GetResetFlags());
	bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
}

/**
@return the name of the current rendering API
*/
const string RenderEngine::currentBackend(){
	
	switch (bgfx::getRendererType()) {
		case bgfx::RendererType::Noop:			return "Disabled";
		case bgfx::RendererType::Direct3D9:		return "DirectX9"; 
		case bgfx::RendererType::Direct3D11:	return "DirectX11";
		case bgfx::RendererType::Direct3D12:	return "DirectX12";
		case bgfx::RendererType::Gnm:			return "GNM";
		case bgfx::RendererType::Metal:			return "Metal";
		case bgfx::RendererType::OpenGL:		return "OpenGL";
		case bgfx::RendererType::OpenGLES:		return "OpenGL ES";
		case bgfx::RendererType::Vulkan:		return "Vulkan";
		case bgfx::RendererType::Nvn:			return "NVN";
		case bgfx::RendererType::WebGPU:		return "WebGPU";
	}
}

uint32_t RenderEngine::GetResetFlags(){
	return (VideoSettings.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE) | BGFX_RESET_HIDPI | BGFX_RESET_MSAA_X8;
}

/**
Initialize static singletons. Invoked automatically if needed.
*/
void RenderEngine::Init()
{
	//setup bgfx if it is not already setup
	if (window != nullptr)
	{
		return;
	}
	SDL_Init(0);
	SDL_Init(SDL_INIT_GAMECONTROLLER);
	window = SDL_CreateWindow("RavEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 480, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	
	//must be in this order
	sdlSetWindow(window);
	bgfx::renderFrame();
	bgfx::init();
	
	//TODO: refactor
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	bgfx::reset(width, height, GetResetFlags());
	
	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_TEXT /*| BGFX_DEBUG_STATS*/);
	
	// Set view 0 clear state.
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	
	bgfx::reset(width, height, GetResetFlags());
	
	bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

	bgfx::setState(BGFX_STATE_DEFAULT);
}
