#include "OgreStatics.hpp"

#ifdef __APPLE__
#include <OgreMetalRenderSystem.h>
#elif defined _WIN32
#include <OgreD3D11RenderSystem.h>
#endif


using namespace Ogre;
using namespace std;

#ifdef _WIN32
typedef D3D11RenderSystem NativeRenderSystem;
#elif defined __APPLE__
typedef MetalRenderSystem NativeRenderSystem;
#endif


void OgreStatics::init() {
	const std::string pluginsFolder = "./";
	const std::string writeAccessFolder = "./";

#ifndef OGRE_STATIC_LIB
#    if OGRE_DEBUG_MODE && \
        !( ( OGRE_PLATFORM == OGRE_PLATFORM_APPLE ) || ( OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS ) )
	const char* pluginsFile = "plugins_d.cfg";
#    else
	const char* pluginsFile = "plugins.cfg";
#    endif
#else
	const char* pluginsFile = "plugins.cfg";
#endif

	root = OGRE_NEW Root(pluginsFolder + pluginsFile, writeAccessFolder + "ogre.cfg", writeAccessFolder + "Ogre.log");

	NativeRenderSystem* rendersystem = new NativeRenderSystem;
	root->setRenderSystem(rendersystem);	//root now owns pointer

	// Initialize Root
	//auto opt = root->getRenderSystem()->getConfigOptions();
	root->getRenderSystem()->setConfigOption("sRGB Gamma Conversion", "Yes");
	root->getRenderSystem()->setConfigOption("Full Screen", "No");
	root->getRenderSystem()->setConfigOption("VSync", "Yes");
	
	window = root->initialise(true, string("RavEngine 0.0.2a - ") + root->getRenderSystem()->getName());

	const size_t numThreads = 1u;
	manager = root->createSceneManager(Ogre::SceneType::ST_GENERIC, numThreads, "FactoryManager");

}