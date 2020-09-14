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

#ifdef __APPLE_
#include <Cocoa/Cocoa.h>
#endif

using namespace std;
using namespace RavEngine;

SDL_Window* RenderEngine::window = nullptr;

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

/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine(const WeakRef<World>& w) : world(w) {
	//call Init()
	Init();
}


RavEngine::RenderEngine::~RenderEngine()
{
}

/**
 Make the rendering system aware of an object
 @param e the entity to load
 */
void RenderEngine::Spawn(Ref<Entity> e){
	
}

/**
 Remove an entity from the system. This does NOT destroy the entity from the world.
 @param e the entity to remove
 */
void RenderEngine::Destroy(Ref<Entity> e){
	
}

/**
 Render one frame using the current state of every object in the world
 */
void RenderEngine::Draw(){
	//get the active camera
    auto worldOwning = Ref<World>(world);
	auto components = worldOwning->Components();
	auto allcams = components.GetAllComponentsOfType<CameraComponent>();

	//set the view transform - all entities drawn will use this matrix
	for (auto& cam : allcams) {
		auto owning = Ref<CameraComponent>(cam);
		if (owning->isActive()) {
			int width,height;
			SDL_GL_GetDrawableSize(window, &width, &height);
			owning->SetTargetSize(width, height);
            MaterialManager::SetProjectionMatrix(cam->GenerateProjectionMatrix());
            MaterialManager::SetViewMatrix(cam->GenerateViewMatrix());
			break;
		}
	}

    //apply transforms for only entities that need to be rendered
    auto toDraw = components.GetAllComponentsOfSubclass<RenderableComponent>();
	
	//clear buffers

	bgfx::touch(0);
	bgfx::dbgTextClear();
	bgfx::dbgTextPrintf(0, 0, 0x4f, "RavEngine");

    //update viewport size if applicable

    //iterate through renderables and call Draw
    for (auto& e : toDraw) {
        e->Draw();
    }
	bgfx::frame();
}

void RenderEngine::resize(){
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	bgfx::reset(width, height, GameplayStatics::VideoSettings.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
	bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
}

/**
@return the name of the current rendering API
*/
const string RenderEngine::currentBackend(){
	
	return "Unknown";
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
	window = SDL_CreateWindow("RavEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 480, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	
	//must be in this order
	sdlSetWindow(window);
	bgfx::renderFrame();
	bgfx::init();
	
	//TODO: refactor
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	bgfx::reset(width, height, GameplayStatics::VideoSettings.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
	
	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_TEXT /*| BGFX_DEBUG_STATS*/);
	
	// Set view 0 clear state.
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	
	bgfx::reset(width, height);
	
	bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
}
