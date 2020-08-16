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

#include <filament/Engine.h>
#include <filament/SwapChain.h>
#include <filament/Camera.h>
#include <filament/Renderer.h>
#include <filament/Viewport.h>
#include <filament/View.h>
#include <filament/Scene.h>
#include <utils/Entity.h>
#include <filament/Material.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <utils/EntityManager.h>
#include <filament/TransformManager.h>
#include <filament/Skybox.h>
#include <filamentapp/FilamentApp.h>
#include <filamentapp/Config.h>

#include <SDL_syswm.h>
#include <SDL.h>
#include <SDL_video.h>

#ifdef __APPLE_
#include <Cocoa/Cocoa.h>
#endif
#include <filamat/MaterialBuilder.h>

using namespace std;
using namespace RavEngine;
//using namespace utils;
using namespace filament;

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
	
	resize();

	filamentView->setScene(filamentScene);
	
	auto skybox = Skybox::Builder().color({ 0.1, 0.125, 0.25, 1.0 }).build(*filamentEngine);
	filamentScene->setSkybox(skybox);
}


RavEngine::RenderEngine::~RenderEngine()
{
	filamentEngine->destroy(filamentView);
	filamentEngine->destroy(filamentScene);
}

/**
 Make the rendering system aware of an object
 @param e the entity to load
 */
void RenderEngine::Spawn(Ref<Entity> e){
	auto& tm = RenderEngine::getEngine()->getTransformManager();
	auto entity = e->transform()->getEntity();
	filamentScene->addEntity(entity);

	//also add children
	auto instance = tm.getInstance(entity);
	const auto count = tm.getChildCount(instance);
	if (count == 0) {
		return;
	}
	vector<utils::Entity> children(count);
	tm.getChildren(instance,&children.front(),count);
	filamentScene->addEntities(&children.front(), count);
}

/**
 Remove an entity from the system. This does NOT destroy the entity from the world.
 @param e the entity to remove
 */
void RenderEngine::Destroy(Ref<Entity> e){
	auto& tm = RenderEngine::getEngine()->getTransformManager();
	auto entity = e->transform()->getEntity();
	filamentScene->remove(entity);

	//also remove children
	auto instance = tm.getInstance(entity);
	const auto count = tm.getChildCount(instance);
	if (count == 0) {
		return;
	}
	vector<utils::Entity> children(count);
	tm.getChildren(instance, &children.front(), count);
	filamentScene->removeEntities(&children.front(), count);
}

/**
 Render one frame using the current state of every object in the world
 */
void RenderEngine::Draw(){
	//get the active camera
	auto components = Ref<World>(world.get())->Components();
	auto allcams = components.GetAllComponentsOfType<CameraComponent>();

	//set the view transform - all entities drawn will use this matrix
	for (auto& cam : allcams) {
		auto owning = Ref<CameraComponent>(cam);
		if (owning->isActive()) {
			filamentView->setCamera(owning->getCamera());
			auto size = GetDrawableArea();
			owning->SetTargetSize(size.width, size.height);
			break;
		}
	}

    //draw each entity (TODO: multithread, skip statics)
	auto worldOwning = Ref<World>(world);
	auto entitylist = worldOwning->getEntities();
	for (auto& entity : entitylist) {
		entity->transform()->Apply();
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
	switch(filamentEngine->getBackend()){
		case Engine::Backend::METAL: return "Metal"; break;
		case Engine::Backend::OPENGL: return "OpenGL"; break;
		case filament::backend::Backend::DEFAULT: return "Default";break;
		case filament::backend::Backend::VULKAN: return "Vulkan"; break;
		case filament::backend::Backend::NOOP: return "Null"; break;
	}
	return "Unknown";
}

RenderEngine::WindowSize RenderEngine::GetDrawableArea() {
	int width; int height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	return WindowSize{ static_cast<unsigned int>(width),static_cast<unsigned int>(height) };
}

/**
Update the viewport to the correct size of the container window
*/
void RenderEngine::resize() {
	//fix the window size
	auto size = GetDrawableArea();

	filamentView->setViewport({ 0,0,size.width,size.height });
#ifdef __APPLE__
	resizeMetalLayer(getNativeWindow(window));
#endif
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
	uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;

	window = SDL_CreateWindow("RavEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 480, windowFlags);

	//get the native window 
    void* nativeWindow = getNativeWindow(window);
	
#ifdef __APPLE__
    //need to make a metal layer on Mac
    nativeWindow = setUpMetalLayer(nativeWindow);
	auto backend = filament::Engine::Backend::METAL;
#else
	auto backend = filament::Engine::Backend::OPENGL;
#endif

	filamentEngine = filament::Engine::create(backend);	
	filamentSwapChain = filamentEngine->createSwapChain((void*)nativeWindow);
	filamentRenderer = filamentEngine->createRenderer();
	
	SDL_SetWindowTitle(window, (string("RavEngine - ") + currentBackend()).c_str());
}
