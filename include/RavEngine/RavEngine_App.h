#include <OgreRoot.h>
#include <OgreWindow.h>
#include <OgreWindowEventUtilities.h>

#ifdef __APPLE__
#include <OgreMetalRenderSystem.h>
#elif defined _WIN32
#include <OgreD3D11RenderSystem.h>
#endif


class RavEngine_App {
public:
	virtual ~RavEngine_App() {}

	/**
	Invoked automatically. Passes command line arguments.
	*/
	int run(int argc, char** argv);
	
protected:
	class AppWindowEventListener : public Ogre::WindowEventListener
	{
		bool mQuit;

	public:
		AppWindowEventListener() : mQuit(false) {}
		virtual void windowClosed(Ogre::Window* rw) { mQuit = true; }

		bool getQuit(void) const { return mQuit; }
	};

	AppWindowEventListener windowEventListener;
	Ogre::Root* root = nullptr;
	Ogre::Window* window = nullptr;
	Ogre::SceneManager* sceneManager;

#ifdef _WIN32
	typedef Ogre::D3D11RenderSystem NativeRenderSystem;
#elif defined __APPLE__
	typedef Ogre::MetalRenderSystem NativeRenderSystem;
#endif
	NativeRenderSystem* renderSystem;

	/**
	The startup hook.
	@param argc the number of command line arguments
	@param argv the command line arguments
	*/
	virtual void OnStartup(int argc, char** argv) {}
	/**
	Invoked before destructor when the application is expected to shut down. You can return exit codes from here. 
	*/
	virtual int OnShutdown() { return 0; };

	void setupwindow();

};
#define START_APP(APP) int main(int argc, char** argv){APP a; return a.run(argc, argv);}