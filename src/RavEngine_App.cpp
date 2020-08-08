
#include "RavEngine_App.h"
#include <GameplayStatics.hpp>

#include <OgreArchiveManager.h>
#include <OgreCamera.h>
#include <OgreConfigFile.h>
#include <OgreRoot.h>
#include <OgreWindow.h>

#include <OgreHlmsManager.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>

#include <Compositor/OgreCompositorManager2.h>

#include <OgreWindowEventUtilities.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include "OSX/macUtils.h"
#endif

using namespace Ogre;

int RavEngine_App::run(int argc, char** argv) {

	//invoke startup hook
    setupwindow();
	OnStartup(argc, argv);

	//in loop tick related things
	{
		//window events to feed to input manager

		//process loaded inputs
		GameplayStatics::inputManager->tick();

		//tick the world
		GameplayStatics::currentWorld->tick();

		//render frame
	}

	//invoke shutdown
	return OnShutdown();
}

class MyWindowEventListener : public Ogre::WindowEventListener
{
    bool mQuit;

public:
    MyWindowEventListener() : mQuit(false) {}
    virtual void windowClosed(Ogre::Window* rw) { mQuit = true; }

    bool getQuit(void) const { return mQuit; }
};

void RavEngine_App::setupwindow(){

    const String pluginsFolder = "./";
    const String writeAccessFolder = "./";

#ifndef OGRE_STATIC_LIB
#    if OGRE_DEBUG_MODE && \
        !( ( OGRE_PLATFORM == OGRE_PLATFORM_APPLE ) || ( OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS ) )
    const char* pluginsFile = "plugins_d.cfg";
#    else
    const char* pluginsFile = "plugins.cfg";
#    endif
#endif

    const char* pluginsFile = "plugins.cfg";
    
    Root* root = OGRE_NEW Root(pluginsFolder + pluginsFile,     //
        writeAccessFolder + "ogre.cfg",  //
        writeAccessFolder + "Ogre.log");

    assert(root->showConfigDialog());

    // Initialize Root
    root->getRenderSystem()->setConfigOption("sRGB Gamma Conversion", "Yes");
    Window* window = root->initialise(true, "Tutorial 00: Basic");

    setupwindow();

    // Create SceneManager
    const size_t numThreads = 1u;
    SceneManager* sceneManager = root->createSceneManager(ST_GENERIC, numThreads, "ExampleSMInstance");

    // Create & setup camera
    Camera* camera = sceneManager->createCamera("Main Camera");

    // Position it at 500 in Z direction
    camera->setPosition(Vector3(0, 5, 15));
    // Look back along -Z
    camera->lookAt(Vector3(0, 0, 0));
    camera->setNearClipDistance(0.2f);
    camera->setFarClipDistance(1000.0f);
    camera->setAutoAspectRatio(true);

    // Setup a basic compositor with a blue clear colour
    CompositorManager2* compositorManager = root->getCompositorManager2();
    const String workspaceName("Demo Workspace");
    const ColourValue backgroundColour(0.2f, 0.4f, 0.6f);
    compositorManager->createBasicWorkspaceDef(workspaceName, backgroundColour, IdString());
    compositorManager->addWorkspace(sceneManager, window->getTexture(), camera, workspaceName, true);

    MyWindowEventListener myWindowEventListener;
    WindowEventUtilities::addWindowEventListener(window, &myWindowEventListener);

    bool bQuit = false;

    while (!bQuit)
    {
        WindowEventUtilities::messagePump();
        bQuit |= !root->renderOneFrame();
        bQuit |= myWindowEventListener.getQuit();
    }

    WindowEventUtilities::removeWindowEventListener(window, &myWindowEventListener);

    OGRE_DELETE root;
    root = 0;
}
