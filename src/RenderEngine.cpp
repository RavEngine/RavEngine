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
#include "SDLSurface.hpp"
#include <memory>
#include "LLGL/RenderSystem.h"
#include "LLGL/LLGL.h"
#include "Gauss/Gauss.h"
#include "Material.hpp"

#include "RenderableComponent.hpp"
#include <SDL_syswm.h>
#include <SDL.h>
#include <SDL_video.h>

#ifdef __APPLE_
#include <Cocoa/Cocoa.h>
#endif

using namespace std;
using namespace RavEngine;

std::shared_ptr<SDLSurface> RenderEngine::surface;
std::unique_ptr<LLGL::RenderSystem> RenderEngine::renderer;

/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine(const WeakRef<World>& w) : world(w) {
	//call Init()
	Init();

    // Get command queue to record and submit command buffers
    queue = renderer->GetCommandQueue();

    // Create command buffer to submit subsequent graphics commands to the GPU
    commands = renderer->CreateCommandBuffer();
}


RavEngine::RenderEngine::~RenderEngine()
{
    delete queue; queue = nullptr;
    delete commands; commands = nullptr;
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
			//TODO: set projection
			auto size = surface->GetDrawableArea();
			owning->SetTargetSize(size.width, size.height);
            MaterialManager::SetProjectionMatrix(cam->GenerateCameraMatrix());
			break;
		}
	}

    //apply transforms for only entities that need to be rendered
    auto toDraw = components.GetAllComponentsOfSubclass<RenderableComponent>();

    // Begin recording commands
    commands->Begin();

    // Clear buffers
    commands->Clear(LLGL::ClearFlags::Color | LLGL::ClearFlags::Depth);

    // Set the render context as the initial render target
    commands->BeginRenderPass(*surface->GetContext());

    //iterate through renderables and call Draw
    for (auto& e : toDraw) {
        e->Draw(commands);
    }

    commands->EndRenderPass();

    commands->End();
    queue->Submit(*commands);

    // Present the result on the screen
    surface->GetContext()->Present();

	//iterate through materials to draw each one
}

/**
@return the name of the current rendering API
*/
const string RenderEngine::currentBackend(){
	
	return "Unknown";
}

SDL_Window* const RavEngine::RenderEngine::GetWindow()
{
	return surface->getWindowPtr();
}

/**
Update the viewport to the correct size of the container window
*/
void RenderEngine::resize() {
	//fix the window size
	auto size = surface->GetDrawableArea();

	//TOOD: update renderer size

#ifdef __APPLE__
	//resizeMetalLayer(getNativeWindow(window));
#endif
}

/**
Initialize static singletons. Invoked automatically if needed.
*/
void RenderEngine::Init()
{
	// don't initialize again if already initialized
	if (surface != nullptr)
	{
		return;
	}

#ifdef __APPLE__
	//need to make a metal layer on Mac
	//nativeWindow = setUpMetalLayer(nativeWindow);
#else
#endif

	//create instance of surface
	surface = std::make_shared<RavEngine::SDLSurface>(LLGL::Extent2D{ 800, 480 }, "RavEngine");

#ifdef _WIN32
	const string backend = "Direct3D11";
#elif defined __APPLE__
	const string backend = "Metal";
#endif

    // Load render system module
    renderer = LLGL::RenderSystem::Load(backend);

    // Create render context
    LLGL::RenderContextDescriptor contextDesc;
    
    contextDesc.videoMode.resolution = surface->GetContentSize();
    contextDesc.vsync.enabled = true;
#ifdef ENABLE_MULTISAMPLING
    contextDesc.samples = 8; // check if LLGL adapts sample count that is too high
#endif
    
    surface->SetContext(renderer->CreateRenderContext(contextDesc,surface));

    // Print renderer information
    const auto& info = renderer->GetRendererInfo();

    std::cout << "Renderer:         " << info.rendererName << std::endl;
    std::cout << "Device:           " << info.deviceName << std::endl;
    std::cout << "Vendor:           " << info.vendorName << std::endl;
    std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;
}