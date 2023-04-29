//
//  RenderEngine.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//	Some code adapted from: https://github.com/pezcode/Cluster/blob/master/src/Renderer/DeferredRenderer.cpp
//

#include "RenderEngine.hpp"
#include <string>
#include "CameraComponent.hpp"
#include "World.hpp"
#include <memory>
#include "Material.hpp"
#include "StaticMesh.hpp"
#include "App.hpp"
#include "GUI.hpp"
#include <RmlUi/Debugger.h>
#include "Utilities.hpp"
#include "InputManager.hpp"
#include "AnimatorComponent.hpp"
#include "Skybox.hpp"

	#if defined __linux__ && !defined(__ANDROID__)
	#define SDL_VIDEO_DRIVER_X11 1		//Without this X11 support doesn't work
	#define SDL_VIDEO_DRIVER_WAYLAND 1
#endif
#include <SDL_syswm.h>
#include <SDL.h>
#include <SDL_video.h>
#include <im3d.h>
#include <BuiltinMaterials.hpp>
#include "Common3D.hpp"
#include "DebugDrawer.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <fmt/core.h>
#include <iostream>
#include "Debug.hpp"
#include <chrono>
#include <cstdio>
#include <RGL/RGL.hpp>
#include <RGL/Device.hpp>
#include <RGL/Synchronization.hpp>

#ifdef __APPLE__
	#include "AppleUtilities.h"
#endif

#if BX_PLATFORM_WINDOWS
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")
#elif BX_PLATFORM_WINRT
#include <winrt/Windows.Graphics.Display.h>
using namespace winrt;
#endif

using namespace std;
using namespace RavEngine;

SDL_Window* RenderEngine::window = nullptr;
RenderEngine::vs RenderEngine::VideoSettings;


#ifndef NDEBUG
static std::optional<GUIComponent> debuggerContext;
STATIC(RenderEngine::debuggerInput);
#endif

static constexpr uint16_t shadowMapSize = 2048;


#ifndef NDEBUG
static DebugDrawer dbgdraw;	//for rendering debug primitives
#endif


void DebugRender(const Im3d::DrawList& drawList){
#if 0
#ifndef NDEBUG
	switch(drawList.m_primType){
		case Im3d::DrawPrimitive_Triangles:
			//Set BGFX state to triangles
			bgfx::setState(BGFX_STATE_DEFAULT);
			break;
		case Im3d::DrawPrimitive_Lines:
			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z /*| BGFX_STATE_DEPTH_TEST_LESS*/ | BGFX_STATE_CULL_CW | BGFX_STATE_LINEAA | BGFX_STATE_PT_LINES);
			//set BGFX state to lines
			break;
		case Im3d::DrawPrimitive_Points:
			//set BGFX state to points
			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA | BGFX_STATE_PT_POINTS);
			break;
		default:
			Debug::Fatal("Invalid Im3d state");
			break;
	}
	//perform drawing here
	const Im3d::VertexData* vertexdata = drawList.m_vertexData;
	const auto verts = drawList.m_vertexCount;
	
	bgfx::VertexBufferHandle vbuf;
	{
		maybestackarray(converted, VertexColor, verts)
		for (uint32_t x = 0; x < verts; x++) {
			Im3d::VertexData d = vertexdata[x];
			converted[x] = { d.m_positionSize.x,d.m_positionSize.y,d.m_positionSize.z,d.m_color };
		}
		vbuf = bgfx::createVertexBuffer(bgfx::copy(&converted[0], verts * sizeof(converted[0])), debuglayout);
	}
	
	bgfx::IndexBufferHandle ibuf;
	{
		maybestackarray(indices, uint16_t, verts)
		for (uint32_t i = 0; i < verts; i++) {
			indices[i] = i;
		}
		ibuf = bgfx::createIndexBuffer(bgfx::copy(&indices[0], verts * sizeof(indices[0])));
	}
	
	mat->Draw(vbuf,ibuf,matrix4(1),RenderEngine::Views::FinalBlit);
	bgfx::destroy(vbuf);
	bgfx::destroy(ibuf);
#endif
#endif
}

/**
Initialize static singletons. Invoked automatically if needed.
*/
void RenderEngine::Init(const AppConfig& config)
{
	//setup RGL if it is not already setup
	if (window != nullptr)
	{
		return;
	}
	
	window = SDL_CreateWindow("RavEngine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, VideoSettings.width, VideoSettings.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	
	if (window == NULL){
		Debug::Fatal("Unable to create main window: {}",SDL_GetError());
	}

	RGL::API api = RGL::API::PlatformDefault;
	{
#if _UWP
		size_t n_elt;
		char* envv;
		_dupenv_s(&envv, &n_elt, "RGL_BACKEND");
#else
		auto envv = std::getenv("RGL_BACKEND");
#endif
		if (envv == nullptr) {
			goto cont;
		}
		auto backend = std::string_view(envv);

		const std::unordered_map<std::string_view, RGL::API> apis{
			{"metal", decltype(apis)::value_type::second_type::Metal},
			{"d3d12", decltype(apis)::value_type::second_type::Direct3D12},
			{"vulkan", decltype(apis)::value_type::second_type::Vulkan},
		};

		auto it = apis.find(backend);
		if (it != apis.end()) {
			api = (*it).second;
		}
		else {
			std::cerr << "No backend \"" << backend << "\", expected one of:\n";
			for (const auto& api : apis) {
				std::cout << "\t - " << RGL::APIToString(api.second) << "\n";
			}
		}
	}
cont:

	RGL::InitOptions opt{
		.api = api,
		.engineName = "RavEngine",
	};
	RGL::Init(opt);

	// for debug wireframes
	auto& data = Im3d::GetAppData();
	data.drawCallback = &DebugRender;

#if __APPLE__
	if (!AppleGPUMeetsMinSpec()) {
		char buf[30]{ 0 };
		AppleGPUName(buf, 30);
		Debug::Fatal("Cannot proceed: device \"{}\" is under the minimum spec!", buf);
	}
#endif
	
}


/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine(const AppConfig& config) {
	Init(config);

	SDL_GetWindowSize(window, &windowdims.width, &windowdims.height);

	device = RGL::IDevice::CreateSystemDefaultDevice();
}

RavEngine::RenderEngine::~RenderEngine()
{
	// TODO: wait for the fence
	DestroyUnusedResources();
}

void RenderEngine::DestroyUnusedResources() {
	// deallocate the resources that have been freed
	RGLBufferPtr gcBuffer;
	while (gcBuffers.try_dequeue(gcBuffer)) {}
}


/**
 Render one frame using the current state of every object in the world
 */
void RenderEngine::Draw(Ref<RavEngine::World> worldOwning){

	// execute when render fence says its ok
	DestroyUnusedResources();
	

/*
	worldOwning->Filter([](GUIComponent& gui) {
		gui.Render();	// kicks off commands for rendering UI
	});
	*/
#ifndef NDEBUG
	// process debug shapes
	worldOwning->FilterPolymorphic([](PolymorphicGetResult<IDebugRenderable, World::PolymorphicIndirection> dbg, const PolymorphicGetResult<Transform, World::PolymorphicIndirection> transform) {
		for (int i = 0; i < dbg.size(); i++) {
			auto& ptr = dbg[i];
			if (ptr.debugEnabled) {
				ptr.DebugDraw(dbgdraw, transform[0]);
			}
		}
		});
	Im3d::GetContext().draw();
	/*
	if (debuggerContext) {
		auto& dbg = *debuggerContext;
		dbg.SetDimensions(bufferdims.width, bufferdims.height);
		dbg.SetDPIScale(GetDPIScale());
		dbg.Update();
		dbg.Render();
	}
	*/
	Im3d::NewFrame();
#endif

}

void RenderEngine::resize(){
	UpdateBufferDims();
#if BX_PLATFORM_IOS
	//view must be manually sized on iOS
	//also this API takes screen points not pixels
	resizeMetalLayer(metalLayer,windowdims.width, windowdims.height);
#endif

}

void RenderEngine::SyncVideoSettings(){
	SDL_SetWindowSize(window, VideoSettings.width, VideoSettings.height);
}

/**
@return the name of the current rendering API
*/
const string_view RenderEngine::GetCurrentBackendName(){
	
	return RGL::APIToString(RGL::CurrentAPI());
}

float RavEngine::RenderEngine::GetCurrentFPS()
{
	return 1.0f / currentFrameTime;
}

float RavEngine::RenderEngine::GetLastFrameTime()
{
	return currentFrameTime * 1000;
}


void RenderEngine::UpdateBufferDims(){
	// on non-apple platforms this is in pixels, on apple platforms it is in "screen points"
	// which will be dealt with later
	SDL_GetWindowSize(window, &windowdims.width, &windowdims.height);

	// update bufferdims
	bufferdims.width = windowdims.width;
	bufferdims.height = windowdims.height;

	// get the canvas size in pixels
# if BX_PLATFORM_WINDOWS
	
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(window, &wmi)) {
		Debug::Fatal("Cannot get native window information");
	}
	auto monitor = MonitorFromWindow(wmi.info.win.window, MONITOR_DEFAULTTONEAREST);
	DEVICE_SCALE_FACTOR fac;
	if (GetScaleFactorForMonitor(monitor,&fac) == S_OK) {
		win_scalefactor = (static_cast<int>(fac) / 100.0);
	}
	else {
		Debug::Fatal("GetScaleFactorForMonitor failed");
	}
#elif BX_PLATFORM_WINRT
	auto dinf = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
	win_scalefactor = static_cast<int32_t>(dinf.ResolutionScale()) / 100.0;
#elif BX_PLATFORM_IOS || BX_PLATFORM_OSX
	// since iOS and macOS do not use OpenGL we cannot use the GL call here
	// instead we derive it by querying display data
	float scale = GetWindowScaleFactor(window);
	bufferdims.width = windowdims.width * scale;
	bufferdims.height = windowdims.height * scale;
#endif
}

#ifndef NDEBUG
void RenderEngine::InitDebugger() const{
	if (!debuggerContext){
		Im3d::AppData& data = Im3d::GetAppData();
		data.drawCallback = &DebugRender;
		
		debuggerContext.emplace(10,10);
		auto ctxd = (*debuggerContext).GetData();
		bool status = Rml::Debugger::Initialise(ctxd->context);
		
		debuggerInput = make_unique<InputManager>();
		
		debuggerInput->BindAnyAction(ctxd);
		debuggerInput->AddAxisMap("MouseX", Special::MOUSEMOVE_X);
		debuggerInput->AddAxisMap("MouseY", Special::MOUSEMOVE_Y);
		
		debuggerInput->AddAxisMap("ScrollY", Special::MOUSEWHEEL_Y);

		auto dbg = PointerInputBinder<GUIComponent>(&*debuggerContext);
		debuggerInput->BindAxis("MouseX", dbg, &GUIComponent::MouseX, CID::ANY, 0);	//no deadzone
		debuggerInput->BindAxis("MouseY", dbg, &GUIComponent::MouseY, CID::ANY, 0);
		debuggerInput->BindAxis("ScrollY", dbg, &GUIComponent::ScrollY, CID::ANY, 0);
	}
}

void RenderEngine::DeactivateDebugger() const{
	debuggerContext.reset();
	debuggerInput = nullptr;
}
#endif

void RenderEngine::SetWindowMode(WindowMode mode){
    int flag;
    switch(mode){
        case WindowMode::Windowed:
            flag = 0;
            break;
        case WindowMode::BorderlessFullscreen:
            flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
            break;
        case WindowMode::Fullscreen:
            flag = SDL_WINDOW_FULLSCREEN;
            break;
    }
    SDL_SetWindowFullscreen(window, flag);
}
