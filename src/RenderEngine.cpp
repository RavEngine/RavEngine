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
#include <Compositor/OgreCompositorManager2.h>
#include <OgreWindow.h>

using namespace std;
using namespace Ogre;

constexpr char mainWorkspaceName[] = "MainWorkspace";

/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine(const WeakRef<World>& w) : world(w) {
	ogrescene = GameplayStatics::ogreFactory.createSceneManager(to_string(uuids::uuid_system_generator{}()));
}

/**
 Make the rendering system aware of an object
 @param e the entity to load
 */
void RenderEngine::Spawn(Ref<Entity> e){
	ogrescene->getRootSceneNode()->addChild(e->transform()->getNode());		//add to ogre scene

	//is this a camera?
	if (e->Components().HasComponentOfType<CameraComponent>()) {
		auto camcomp = e->Components().GetComponent<CameraComponent>();
		auto workspace = camcomp->GetCompositor();
		//does the camera have a compositormanager?
		if (workspace == nullptr) {
			auto cam = camcomp->getCamera();
			// Setup a compositor for this camera
			auto window = GameplayStatics::ogreFactory.GetWindow();
			auto root = GameplayStatics::ogreFactory.GetRoot();
			CompositorManager2* compositorManager = root->getCompositorManager2();
			const ColourValue backgroundColour(0.2f, 0.4f, 0.6f);
			//workspaces are id'ed to the owning camera
			compositorManager->createBasicWorkspaceDef(cam->getName(), backgroundColour, IdString());
			workspace = compositorManager->addWorkspace(ogrescene, window->getTexture(), cam, cam->getName(), camcomp->isActive());
		}
	}
}

/**
 Remove an entity from the system. This does NOT destroy the entity from the world.
 @param e the entity to remove
 */
void RenderEngine::Destroy(Ref<Entity> e){
	ogrescene->getRootSceneNode()->removeChild(e->transform()->getNode());	//remove from ogre scene
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
		entity->transform()->ApplyToSceneNode();
		//entity->Draw();
    }
}

/**
@return the name of the current rendering API
*/
const string RenderEngine::currentBackend(){
	return GameplayStatics::ogreFactory.GetRoot()->getRenderSystem()->getName();
}
