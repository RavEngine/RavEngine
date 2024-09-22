#include <RavEngine/App.hpp>
#include <RavEngine/StaticMesh.hpp>
#include <RavEngine/World.hpp>
#include <RavEngine/Dialogs.hpp>
#include <RavEngine/StartApp.hpp>
#include <RavEngine/MeshCollection.hpp>

using namespace RavEngine;
using namespace std;

struct DummyApp : public RavEngine::App {
	void OnStartup(int argc, char** argv) final;
	void OnFatal(const std::string_view msg) final {
		RavEngine::Dialog::ShowBasic("Fatal Error", msg, Dialog::MessageBoxType::Error);
	}
};

struct DummyWorld : public RavEngine::World {
};

// We've defined a world, but now we need to load it. OnStartup is a good place to load your initial world.
void DummyApp::OnStartup(int argc, char** argv) {

	// You can rename the window via this App method.
	SetWindowTitle("Hello RavEngine!");

	// Make an instance of the world. RavEngine provides the New<T> helper to allocate
	// resources which the engine does not direclty reference. The return value is an owning pointer,
	// so be careful about storing references to worlds in Components, to avoid reference cycles. 
	auto level = RavEngine::New<DummyWorld>();

	// Tell the engine to switch to this world.
	// If the engine has no worlds active, it will automatically set the first one as the active (rendered) world.
	// You can have multiple worlds loaded and ticking at once, but only one world can be the active world. 
	AddWorld(level);
}

// Last thing - we need to launch our application. RavEngine supplies a convenience macro for this,
// which simply inlines a main function that allocates and launches your app, then invokes its OnStartup method.
// You do not need to use this macro if you don't want to.
START_APP(DummyApp)
