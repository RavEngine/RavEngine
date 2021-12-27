#include "App.hpp"
#include "RenderEngine.hpp"
#include <SDL_events.h>
#include <bgfx/bgfx.h>
#include <algorithm>
#include "MeshAsset.hpp"
#include "InputManager.hpp"
#include "Material.hpp"
#include <physfs.h>
#include "Texture.hpp"
#include <RmlUi/Core.h>
#include "GUI.hpp"
#include "RMLFileInterface.hpp"
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include "SyncVar.hpp"
#include "Skybox.hpp"
#include <SDL.h>
#include <filesystem>
#include "Function.hpp"
#include "World.hpp"
#include "GetApp.hpp"

#ifdef _WIN32
	#include <Windows.h>
	#undef min
#endif

#ifdef __APPLE__
    #include "AppleUtilities.h"
#endif

using namespace std;
using namespace RavEngine;
using namespace std::chrono;

// pointer to the current app instance
static App* currentApp = nullptr;

static float currentScale = 0;

// on crash, call this
void crash_signal_handler(int signum) {
	::signal(signum, SIG_DFL);
	Debug::PrintStacktraceHere();
	::raise(SIGABRT);
}

/**
 GameNetworkingSockets debug log function
 */
static void DebugOutput( ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg )
{
	if ( eType == k_ESteamNetworkingSocketsDebugOutputType_Bug )
	{
		Debug::Fatal("{}",pszMsg);
	}
	else{
		Debug::Log("{}",pszMsg);
	}
}

App::App(const std::string& resourcesName){
	currentApp = this;
	// crash signal handlers
	::signal(SIGSEGV, &crash_signal_handler);
	::signal(SIGABRT, &crash_signal_handler);

	//initialize virtual file system library 
	PHYSFS_init("");
	
	Resources.emplace(resourcesName + ".zip");
}

int App::run(int argc, char** argv) {

	// initialize SDL2
	if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_HAPTIC | SDL_INIT_VIDEO) != 0) {
		Debug::Fatal("Unable to initialize SDL2: {}", SDL_GetError());
	}
	{
		auto config = OnConfigure(argc, argv);

		Renderer.emplace(config);
	}
	
	Skybox::Init();

	//setup GUI rendering
	Rml::SetSystemInterface(&GetRenderEngine());
	Rml::SetRenderInterface(&GetRenderEngine());
	Rml::SetFileInterface(new VFSInterface());
	Rml::Initialise();

#ifdef _DEBUG
	Renderer->InitDebugger();
#endif

#ifdef __APPLE__
	enableSmoothScrolling();
#endif

	//load the built-in fonts
	App::Resources->IterateDirectory("fonts", [](const std::string& filename) {
		auto p = std::filesystem::path(filename);
		if (p.extension() == ".ttf") {
			GUIComponent::LoadFont(p.filename().string());
		}
		});

	//setup Audio
	player.Init();

	//setup networking
	SteamDatagramErrMsg errMsg;
	if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
		Debug::Fatal("Networking initialization failed: {}", errMsg);
	}
	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
	
	// if built in non-UWP for Windows, need to manually set DPI awareness
#if defined _WIN32 && !_WINRT
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

	{
		//make the default texture white
		uint8_t data[] = {0xFF,0xFF,0xFF,0xFF};
		TextureManager::defaultTexture = make_shared<RuntimeTexture>(1,1,false,1,data);
	}

	//invoke startup hook
	OnStartup(argc, argv);
	
	lastFrameTime = clocktype::now();
	
	bool exit = false;
	SDL_Event event;
	while (!exit) {
		//setup framerate scaling for next frame
		auto now = clocktype::now();
		//will cause engine to run in slow motion if the frame rate is <= 1fps
		deltaTimeMicroseconds = std::min(duration_cast<timeDiff>(now - lastFrameTime), maxTimeStep);
        float deltaSeconds = std::chrono::duration<decltype(deltaSeconds)>(deltaTimeMicroseconds).count();
		time += deltaSeconds;
		float scale = deltaSeconds * evalNormal;
		currentScale = scale;

		auto windowflags = SDL_GetWindowFlags(RenderEngine::GetWindow());
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					exit = true;
					break;
					
				case SDL_WINDOWEVENT: {
					const SDL_WindowEvent& wev = event.window;
					switch (wev.event) {
						case SDL_WINDOWEVENT_RESIZED:
						case SDL_WINDOWEVENT_SIZE_CHANGED:
							Renderer->resize();
							break;
							
						case SDL_WINDOWEVENT_CLOSE:
							exit = true;
							break;
					}
				} break;
			}
			//process others
			if (inputManager){
				inputManager->ProcessInput(event,windowflags,scale);
#ifdef _DEBUG
				RenderEngine::debuggerInput->ProcessInput(event,windowflags,scale);
#endif
			}
		}
	
#ifdef _DEBUG
		RenderEngine::debuggerInput->TickAxes();
#endif
		if (inputManager) {
			inputManager->TickAxes();
		}

		//tick all worlds
		for(const auto world : loadedWorlds){
			world->Tick(scale);
		}
				
		//process main thread tasks
		Function<void(void)> front;
		while (main_tasks.try_dequeue(front)){
			front();
		}

		// update GUIs
		renderWorld->Filter<GUIComponent>([](float, auto& gui) {
			gui.Render();	//bgfx state is set in renderer before actual draw calls
		});

		Renderer->Draw(renderWorld);
		player.SetWorld(renderWorld);
		SyncVar_base::Swap();
		SyncVar_base::ProcessQueue();
		        
        //make up the difference
		//can't use sleep because sleep is not very accurate
		/*clocktype::duration work_time;
		do{
			auto workEnd = clocktype::now();
			work_time = workEnd - now;
			auto delta = min_tick_time - work_time;
			if (delta > std::chrono::duration<double, std::milli>(3)) {
				auto dc = std::chrono::duration_cast<std::chrono::milliseconds>(delta);
				std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(dc.count()-1));
			}
		}while (work_time < min_tick_time);*/
        
		lastFrameTime = now;
	}
	
    return OnShutdown();
}

float App::CurrentTPS() {
	return App::evalNormal / currentScale;
}

void App::Quit(){
	SDL_Event event;
	event.type = SDL_QUIT;
	SDL_PushEvent(&event);
}

App::~App(){
	inputManager = nullptr;
	renderWorld = nullptr;
	loadedWorlds.clear();
#ifdef _DEBUG
	Renderer->DeactivateDebugger();
#endif
    MeshAsset::Manager::Clear();
    MeshAssetSkinned::Manager::Clear();
    Texture::Manager::Clear();
	player.Shutdown();
	networkManager.server.reset();
	networkManager.client.reset();
	GameNetworkingSockets_Kill();
	LightManager::Teardown();
	PHYSFS_deinit();
	f1.Reset();
	f2.Reset();
	f3.Reset();
	Skybox::Teardown();
	auto fsi = Rml::GetFileInterface();
	Rml::Shutdown();
    Renderer.reset();
	delete fsi;
}

void App::SetWindowTitle(const char *title){
	SDL_SetWindowTitle(Renderer->GetWindow(), title);
}

void RavEngine::App::SetRenderedWorld(Ref<World> newWorld){
   if (!loadedWorlds.contains(newWorld)){
       Debug::Fatal("Cannot render an inactive world");
   }
   if (renderWorld) {
       renderWorld->OnDeactivate();
       renderWorld->isRendering = false;
   }
   renderWorld = newWorld;
   renderWorld->isRendering = true;
   renderWorld->OnActivate();
}

void RavEngine::App::RemoveWorld(Ref<World> world){
    loadedWorlds.erase(world);
    if (renderWorld == world){
        renderWorld->OnDeactivate();
        renderWorld.reset();    //this will cause nothing to render, so set a different world as rendered
    }
}

std::optional<Ref<World>> RavEngine::App::GetWorldByName(const std::string &name){
    std::optional<Ref<World>> value;
    for(const auto& world : loadedWorlds){
        // because std::string "world\0\0" != "world", we need to use strncmp
        if (std::strncmp(world->worldID.data(),name.data(), World::id_size) == 0){
            value.emplace(world);
            break;
        }
    }
    return value;
}

void App::AddWorld(Ref<World> world) {
	loadedWorlds.insert(world);
	if (!renderWorld) {
		SetRenderedWorld(world);
	}

	// synchronize network if necessary
	if (networkManager.IsClient() && !networkManager.IsServer()) {
		networkManager.client->SendSyncWorldRequest(world);
	}
}

App* RavEngine::GetApp()
{
	return currentApp;
}
