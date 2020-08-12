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
#include "GameplayStatics.hpp"
#include "CameraComponent.hpp"
#include "World.hpp"

#include <filament/Engine.h>
#include <filament/SwapChain.h>
#include <filament/Renderer.h>
#include <filament/View.h>
#include <filament/Scene.h>
#include <utils/Entity.h>
#include <filament/RenderableManager.h>
#include <utils/EntityManager.h>

#include <SDL_syswm.h>
#include <SDL.h>

#ifdef __APPLE_
#include <Cocoa/Cocoa.h>
#endif

using namespace std;
//using namespace utils;

SDL_Window* RenderEngine::window = nullptr;
filament::SwapChain* RenderEngine::filamentSwapChain = nullptr;
filament::Engine* RenderEngine::filamentEngine = nullptr;
filament::Renderer* RenderEngine::filamentRenderer = nullptr;

/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine(const WeakRef<World>& w) : world(w) {
	if (filamentEngine == nullptr) {
		Init();
	}
	filamentView = filamentEngine->createView();
	filamentScene = filamentEngine->createScene();

	filament::Camera* camera = filamentEngine->createCamera(utils::EntityManager::get().create());

	filamentView->setCamera(camera);
	filamentView->setScene(filamentScene);

	utils::Entity renderable = utils::EntityManager::get().create();
	// build a quad
	/*filament::RenderableManager::Builder(1)
		.boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
		.material(0, materialInstance)
		.geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer, indexBuffer, 0, 6)
		.culling(false)
		.build(*engine, renderable);
	scene->addEntity(renderable);*/
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

int counter = 0;
/**
 Render one frame using the current state of every object in the world
 */
void RenderEngine::Draw(){
	//get the active camera
	/*auto components = Ref<World>(world.get())->Components();
	auto allcams = components.GetAllComponentsOfType<CameraComponent>();*/

	//set the view transform - all entities drawn will use this matrix
	/*for (auto& cam : allcams) {
		auto owning = Ref<CameraComponent>(cam);
		if (owning->isActive()) {
			activeCamera = owning->getCamera();
			break;
		}
	}*/

    //draw each entity
	auto worldOwning = Ref<World>(world);
	auto entitylist = worldOwning->getEntities();
	for (auto& entity : entitylist) {
		//entity->transform()->ApplyToSceneNode();
		//entity->Draw();
    }

	if (filamentRenderer->beginFrame(filamentSwapChain)) {
		// for each View
		filamentRenderer->render(filamentView);
		filamentRenderer->endFrame();
	}
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
	//if already initialized, don't do anything
	if (filamentEngine != nullptr) {
		return;
	}
	//create SDL window

	SDL_Init(SDL_INIT_EVENTS);
	uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;

	window = SDL_CreateWindow("RavEngine - Filament", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 480, windowFlags);

	//get the native window 
#ifdef _WIN32
	//Windows implementation
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	assert(SDL_GetWindowWMInfo(window, &wmi));
	//HDC nativeWindow = (HDC)wmi.info.win.hdc;
	HWND nativeWindow = wmi.info.win.window;
#elif defined __APPLE__
	//mac implementation
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	NSWindow* win = (NSWindow*)wmi.info.cocoa.window;
	NSView* view = [win contentView];
	void* nativeWindow = view;
#endif

#ifdef __APPLE__
	auto backend = filament::Engine::Backend::Metal;
	//also create a metal view and set as the nativeWindow
	void* metalLayer = nullptr;	//this is passed to createSwapChain
#else
	auto backend = filament::Engine::Backend::VULKAN;
#endif

	filamentEngine = filament::Engine::create(backend);	//backend?
	filamentSwapChain = filamentEngine->createSwapChain((void*)nativeWindow);
	filamentRenderer = filamentEngine->createRenderer();
}
