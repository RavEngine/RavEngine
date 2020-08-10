
#include "RavEngine_App.hpp"
#include "OgreStatics.hpp"

#include <GameplayStatics.hpp>

#include <OgreArchiveManager.h>
#include <OgreCamera.h>
#include <OgreConfigFile.h>

#include <OgreHlmsManager.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>

//#include <OgreGL3PlusRenderSystem.h>

#include <fstream>

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include "OSX/macUtils.h"
#endif

using namespace Ogre;
using namespace std;

int RavEngine_App::run(int argc, char** argv) {

	//invoke startup hook
    setupwindow();
	OnStartup(argc, argv);

	//in loop tick related things
    bool bQuit = false;

    auto root = GameplayStatics::ogreFactory.GetRoot();
    auto window = GameplayStatics::ogreFactory.GetWindow();

    while (!bQuit)
    {
        WindowEventUtilities::messagePump();

        //window events to feed to input manager

        //process loaded inputs
        GameplayStatics::inputManager->tick();

        //tick the world
        GameplayStatics::currentWorld->tick();

        //render frame
        bQuit |= !root->renderOneFrame();
        bQuit |= windowEventListener.getQuit();
    }

    //teardown
    WindowEventUtilities::removeWindowEventListener(window, &windowEventListener);

	//invoke shutdown
	return OnShutdown();
}

void registerHlms(void)
{
    using namespace Ogre;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
    // Note:  macBundlePath works for iOS too. It's misnamed.
    const String resourcePath = Ogre::macBundlePath() + "/Contents/Resources/";
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    const String resourcePath = Ogre::macBundlePath() + "/";
#else
    String resourcePath = "";
#endif

    ConfigFile cf;
    cf.load(resourcePath + "resources2.cfg");

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    String rootHlmsFolder = macBundlePath() + '/' + cf.getSetting("DoNotUseAsResource", "Hlms", "");
#else
    String rootHlmsFolder = resourcePath + cf.getSetting("DoNotUseAsResource", "Hlms", "");
#endif

    if (rootHlmsFolder.empty())
        rootHlmsFolder = "./";
    else if (*(rootHlmsFolder.end() - 1) != '/')
        rootHlmsFolder += "/";

    // At this point rootHlmsFolder should be a valid path to the Hlms data folder

    HlmsUnlit* hlmsUnlit = 0;
    HlmsPbs* hlmsPbs = 0;

    // For retrieval of the paths to the different folders needed
    String mainFolderPath;
    StringVector libraryFoldersPaths;
    StringVector::const_iterator libraryFolderPathIt;
    StringVector::const_iterator libraryFolderPathEn;

    ArchiveManager& archiveManager = ArchiveManager::getSingleton();

    {
        // Create & Register HlmsUnlit
        // Get the path to all the subdirectories used by HlmsUnlit
        HlmsUnlit::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
        Archive* archiveUnlit =
            archiveManager.load(rootHlmsFolder + mainFolderPath, "FileSystem", true);
        ArchiveVec archiveUnlitLibraryFolders;
        libraryFolderPathIt = libraryFoldersPaths.begin();
        libraryFolderPathEn = libraryFoldersPaths.end();
        while (libraryFolderPathIt != libraryFolderPathEn)
        {
            Archive* archiveLibrary =
                archiveManager.load(rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true);
            archiveUnlitLibraryFolders.push_back(archiveLibrary);
            ++libraryFolderPathIt;
        }

        // Create and register the unlit Hlms
        hlmsUnlit = OGRE_NEW HlmsUnlit(archiveUnlit, &archiveUnlitLibraryFolders);
        Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);
    }

    {
        // Create & Register HlmsPbs
        // Do the same for HlmsPbs:
        HlmsPbs::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
        Archive* archivePbs = archiveManager.load(rootHlmsFolder + mainFolderPath, "FileSystem", true);

        // Get the library archive(s)
        ArchiveVec archivePbsLibraryFolders;
        libraryFolderPathIt = libraryFoldersPaths.begin();
        libraryFolderPathEn = libraryFoldersPaths.end();
        while (libraryFolderPathIt != libraryFolderPathEn)
        {
            Archive* archiveLibrary =
                archiveManager.load(rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true);
            archivePbsLibraryFolders.push_back(archiveLibrary);
            ++libraryFolderPathIt;
        }

        // Create and register
        hlmsPbs = OGRE_NEW HlmsPbs(archivePbs, &archivePbsLibraryFolders);
        Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);
    }

    RenderSystem* renderSystem = Root::getSingletonPtr()->getRenderSystem();
    if (renderSystem->getName() == "Direct3D11 Rendering Subsystem")
    {
        // Set lower limits 512kb instead of the default 4MB per Hlms in D3D 11.0
        // and below to avoid saturating AMD's discard limit (8MB) or
        // saturate the PCIE bus in some low end machines.
        bool supportsNoOverwriteOnTextureBuffers;
        renderSystem->getCustomAttribute("MapNoOverwriteOnDynamicBufferSRV",
            &supportsNoOverwriteOnTextureBuffers);

        if (!supportsNoOverwriteOnTextureBuffers)
        {
            hlmsPbs->setTextureBufferDefaultSize(512 * 1024);
            hlmsUnlit->setTextureBufferDefaultSize(512 * 1024);
        }
    }
}


void RavEngine_App::setupwindow(){

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

     GameplayStatics::ogreFactory.init();

     auto root = GameplayStatics::ogreFactory.GetRoot();

    /*{
        struct stat buf;
        auto cfgpath = pluginsFolder + string(pluginsFile);
        if (stat((cfgpath).c_str(), &buf) != 0) {
            ofstream output(cfgpath);
            output << endl;
            output.close();
        }
    }
    
    if (!root->showConfigDialog()) {

    }*/


    
    
    //registerHlms();

    // Create SceneManager

    // Create & setup camera

    WindowEventUtilities::addWindowEventListener(GameplayStatics::ogreFactory.GetWindow(), &windowEventListener);
}
