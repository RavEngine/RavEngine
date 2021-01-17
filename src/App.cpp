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

#ifdef _WIN32
	#include <Windows.h>
	#undef min
#endif

using namespace std;
using namespace RavEngine;
using namespace std::chrono;

const float RavEngine::App::evalNormal = 60;
Ref<VirtualFilesystem> App::Resources;
double App::time;
Ref<RenderEngine> App::Renderer;
static float currentScale = 0;

ConcurrentQueue<function<void(void)>> App::main_tasks;
tf::Executor App::executor;
Ref<InputManager> App::inputManager;
Ref<World> App::currentWorld;

App::App(const std::string& resourcesName){
	//initialize virtual file system library -- on unix systems this must pass argv[0]
	PHYSFS_init("");
	
	Resources = make_shared<VirtualFilesystem>(resourcesName + ".zip");
	Renderer = make_shared<RenderEngine>();

	//setup GUI rendering
	Rml::SetSystemInterface(Renderer.get());
	Rml::SetRenderInterface(Renderer.get());
	Rml::SetFileInterface(new VFSInterface());
	Rml::Initialise();
	
#ifdef _DEBUG
	Renderer->InitDebugger();
#endif
	
	
	//load the built-in fonts
	GUIComponent::LoadFont("Roboto-Regular.ttf");
}

int App::run(int argc, char** argv) {
	
#ifdef _WIN32
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

	{
		//make the default texture white
		uint8_t data[] = {0xCC,0xCC,0xCC,0xCC};
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
		deltaTimeMicroseconds = std::min(duration_cast<timeDiff>((now - lastFrameTime)), maxTimeStep);
		float deltaSeconds = deltaTimeMicroseconds.count() / 1000.0 / 1000;
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
		currentWorld->Tick(scale);
		
		//process main thread tasks
		std::function<void(void)> front;
		while (main_tasks.try_dequeue(front)){
			front();
		}
		if (inputManager){
			inputManager->TickAxes();
		}

		Renderer->DrawNext(currentWorld);

#ifdef LIMIT_TICK
		this_thread::sleep_for(tickrate);
#endif
		lastFrameTime = now;
	}
	
	//RenderEngine::BlockUntilFinishDraw();

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
	currentWorld = nullptr;
	Material::Manager::RemoveAll();
	PHYSFS_deinit();
	auto fsi = Rml::GetFileInterface();
	Rml::Shutdown();
	delete fsi;
}
