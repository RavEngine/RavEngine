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

const float RavEngine::App::evalNormal = 60;
Ref<VirtualFilesystem> App::Resources;
NetworkManager App::networkManager;
double App::time;
Ref<RenderEngine> App::Renderer;
static float currentScale = 0;

ConcurrentQueue<function<void(void)>> App::main_tasks;
tf::Executor App::executor;
Ref<InputManager> App::inputManager;
Ref<World> App::renderWorld;
locked_hashset<Ref<World>,SpinLock> App::loadedWorlds;

STATIC(App::swapmtx1);
STATIC(App::swapmtx2);
STATIC(App::f1);
STATIC(App::f2);
STATIC(App::f3);
STATIC(App::current) = &App::f1;
STATIC(App::inactive) = &App::f2;
STATIC(App::render) = &App::f3;

std::chrono::duration<double,std::micro> App::min_tick_time(std::chrono::duration<double,std::milli>(1.0/90 * 1000));

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
	// crash signal handlers
	::signal(SIGSEGV, &crash_signal_handler);
	::signal(SIGABRT, &crash_signal_handler);

	//initialize virtual file system library -- on unix systems this must pass argv[0]
	PHYSFS_init("");
	
	Resources = make_shared<VirtualFilesystem>(resourcesName + ".zip");
	Renderer = make_shared<RenderEngine>();
	SkyBox::Init();

	//setup GUI rendering
	Rml::SetSystemInterface(Renderer.get());
	Rml::SetRenderInterface(Renderer.get());
	Rml::SetFileInterface(new VFSInterface());
	Rml::Initialise();
	
#ifdef _DEBUG
	Renderer->InitDebugger();
#endif
	
#ifdef __APPLE__
    enableSmoothScrolling();
#endif
	
	//load the built-in fonts
	GUIComponent::LoadFont("Roboto-Regular.ttf");
	
	//setup Audio
	player.Init();
	
	//setup networking
	SteamDatagramErrMsg errMsg;
	if ( ! GameNetworkingSockets_Init(nullptr, errMsg) ){
		Debug::Fatal("Networking initialization failed: {}",errMsg);
	}
	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg,DebugOutput);
}

int App::run(int argc, char** argv) {
	
#ifdef _WIN32
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

	{
		//make the default texture white
		uint8_t data[] = {0xFF,0xFF,0xFF,0xFF};
		TextureManager::defaultTexture = make_shared<RuntimeTexture>(1,1,false,1,4,data);
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
		//tick all worlds
		for(const auto world : loadedWorlds){
			world->Tick(scale);
		}
				
		//process main thread tasks
		std::function<void(void)> front;
		while (main_tasks.try_dequeue(front)){
			front();
		}
		if (inputManager){
			inputManager->TickAxes();
		}

		Renderer->DrawNext(renderWorld);
		player.SetWorld(renderWorld);
		SyncVar_base::Swap();
		SyncVar_base::ProcessQueue();
		        
        //make up the difference
		//can't use sleep because sleep is not very accurate
		clocktype::duration work_time;
		do{
			auto workEnd = clocktype::now();
			work_time = workEnd - now;
		}while (work_time < min_tick_time);
        
		lastFrameTime = now;
	}
	
	Renderer->BlockUntilFinishDraw();

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
	player.Shutdown();
	Material::Manager::RemoveAll();
	networkManager.server.reset();
	networkManager.client.reset();
	GameNetworkingSockets_Kill();
	LightManager::Teardown();
	Renderer.reset();
	PHYSFS_deinit();
	f1.Clear();
	f2.Clear();
	f3.Clear();
	SkyBox::Teardown();
	auto fsi = Rml::GetFileInterface();
	Rml::Shutdown();
	delete fsi;
}

void App::SetWindowTitle(const char *title){
	SDL_SetWindowTitle(Renderer->GetWindow(), title);
}
