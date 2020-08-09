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
#include <filesystem>

using namespace std;

/**
 Make the rendering system aware of an object
 @param e the entity to load
 */
void RenderEngine::Spawn(Ref<Entity> e){
    entities.push_back(e);
}

/**
 Remove an entity from the system. This does NOT destroy the entity from the world.
 @param e the entity to remove
 */
void RenderEngine::Destroy(Ref<Entity> e){
    entities.remove_if([&](const Ref<Entity>& item) {
        return e.get() == item.get();
    });
}

int counter = 0;
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
		if (owning->isActive) {
			owning->SetViewTransform();
			break;
		}
	}

    //draw each entity
    for (auto& entity : entities) {
		entity->Draw();
    }
}

const string RenderEngine::currentBackend(){
	/*switch(bgfx::getRendererType()) {
		case bgfx::RendererType::Noop:		 return "No rendering";
		case bgfx::RendererType::Direct3D9:  return "Direct3D 9";
		case bgfx::RendererType::Direct3D11: return "Direct3D 11";
		case bgfx::RendererType::Direct3D12: return "Direct3D 12";
		case bgfx::RendererType::Gnm:        return "GNM";
		case bgfx::RendererType::Metal:      return "Metal";
		case bgfx::RendererType::OpenGL:     return "OpenGL";
		case bgfx::RendererType::OpenGLES:   return "OpenGL ES";
		case bgfx::RendererType::Vulkan:     return "Vulkan";
		case bgfx::RendererType::Nvn: 		 return "NVM";
		case bgfx::RendererType::WebGPU:	 return "WebGPU";
		case bgfx::RendererType::Count:      return "Count";
		default:
			return "Unknown";
	}*/
	return "unknown";
}
