#include "App.hpp"
#if !RVE_SERVER
    #include "RenderEngine.hpp"
    #include <SDL3/SDL_events.h>
    #include "Texture.hpp"
    #include <RmlUi/Core.h>
    #include "GUI.hpp"
    #include "Material.hpp"
    #include "RMLFileInterface.hpp"
    #include "InputManager.hpp"
    #include "Skybox.hpp"
    #include <SDL3/SDL.h>
    #include "AudioPlayer.hpp"
    #include "Window.hpp"
    #include <RGL/Swapchain.hpp>
    #include <RGL/CommandBuffer.hpp>
    #include "OpenXRIntegration.hpp"
	#include "BuiltinTonemap.hpp"
#endif
#include <algorithm>
#include "MeshAsset.hpp"
#include <physfs.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <filesystem>
#include "Function.hpp"
#include "World.hpp"
#include "GetApp.hpp"
#include "Defines.hpp"
#include "VirtualFileSystem.hpp"
#include "MeshAssetSkinned.hpp"

#include "CameraComponent.hpp"
#include <csignal>
#include "Debug.hpp"
#include "Profile.hpp"

#ifdef _WIN32
	#include <Windows.h>
	#include <winuser.h>
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

#if !RVE_SERVER
void RGLFatalCallback(const std::string& msg, void* userData) {
	Debug::Fatal(msg);
};

void RGLmsgCallback(RGL::MessageSeverity severity, const std::string& msg, void* userData) {
	switch (severity) {
	case RGL::MessageSeverity::Info:
		Debug::Log(msg);
		break;
	case RGL::MessageSeverity::Warning:
		Debug::Warning(msg);
		break;
	case RGL::MessageSeverity::Error:
		Debug::Error(msg);
		break;
	}
	// fatal errors are handled by the fatal callback
};
#endif

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

App::App()
{
    currentApp = this;
	// crash signal handlers
	::signal(SIGSEGV, &crash_signal_handler);
	::signal(SIGABRT, &crash_signal_handler);

	//initialize virtual file system library
#if __ANDROID__
    PHYSFS_AndroidInit androidInit{
        .jnienv = SDL_GetAndroidJNIEnv(),
        .context = SDL_GetAndroidActivity()
    };
	if (PHYSFS_init(reinterpret_cast<const char*>(&androidInit)) == 0){
        Debug::Fatal("PhysFS failed to init: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }
#else
    PHYSFS_init("");
#endif
#if !RVE_SERVER
	Resources = std::make_unique<VirtualFilesystem>();
#endif
}

int App::run(int argc, char** argv) {
#if !RVE_SERVER
	// initialize SDL2
	if (not SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_EVENTS | SDL_INIT_HAPTIC | SDL_INIT_VIDEO)) {
		Debug::Fatal("Unable to initialize SDL: {}", SDL_GetError());
	}
	{
		window = std::make_unique<Window>(960, 540, "RavEngine");

		auto config = OnConfigure(argc, argv);

		// initialize RGL and the global Device
		RGL::API api = RGL::API::PlatformDefault;
		{

			auto envv = std::getenv("RGL_BACKEND");

			if (envv == nullptr) {
				goto cont;
			}
			auto backend = std::string_view(envv);

			const std::unordered_map<std::string_view, RGL::API> apis{
				{"metal", decltype(apis)::value_type::second_type::Metal},
				{ "d3d12", decltype(apis)::value_type::second_type::Direct3D12 },
				{ "vulkan", decltype(apis)::value_type::second_type::Vulkan },
			};

			auto it = apis.find(backend);
			if (it != apis.end()) {
				api = (*it).second;
			}
			else {
				std::cerr << "No backend \"" << backend << "\", expected one of:\n";
				for (const auto& api : apis) {
					std::cout << "\t - " << RGL::APIToString(api.second) << "\n";
				}
			}
		}
	cont:

		RGL::InitOptions opt{
			.api = api,
			.callback = RGLmsgCallback,
			.fatal_callback = RGLFatalCallback,
			.engineName = "RavEngine",
		};
		RGL::Init(opt);

		device = RGL::IDevice::CreateSystemDefaultDevice();

		Renderer = std::make_unique<RenderEngine>(config, device);
		Renderer->dummyTonemap = New<DummyTonemapInstance>(New<DummyTonemap>());


		window->InitSwapchain(device, Renderer->mainCommandQueue);

		auto size = window->GetSizeInPixels();
		mainWindowView = { Renderer->CreateRenderTargetCollection({ static_cast<unsigned int>(size.width), static_cast<unsigned int>(size.height) }) };

#ifdef RVE_XR_AVAILABLE
		if (wantsXR) {
			OpenXRIntegration::init_openxr({
				.device = device,
				.commandQueue = Renderer->mainCommandQueue
				});
			xrRenderViewCollections = OpenXRIntegration::CreateRenderTargetCollections();
		}
#endif
	}

	//setup GUI rendering
	Rml::SetSystemInterface(&GetRenderEngine());
	Rml::SetRenderInterface(&GetRenderEngine());
	Rml::SetFileInterface(new VFSInterface());
	Rml::Initialise();

#ifndef NDEBUG
	Renderer->InitDebugger();
#endif

#ifdef __APPLE__
	enableSmoothScrolling();
#endif

	//load the built-in fonts
	App::Resources->IterateDirectory("fonts", [](const std::string& filename) {
		auto p = Filesystem::Path(filename);
		if (p.extension() == ".ttf") {
			GUIComponent::LoadFont(p.filename().string());
		}
		});

	//setup Audio
	if (NeedsAudio()) {
		player = std::make_unique<AudioPlayer>();
		player->Init();
	}
#endif
	//setup networking
	if (NeedsNetworking()) {
		SteamDatagramErrMsg errMsg;
		if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
			Debug::Fatal("Networking initialization failed: {}", errMsg);
		}
		SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
	}
	
	// if built in non-UWP for Windows, need to manually set DPI awareness
	// for some weird reason, it's not present on ARM
#if defined _WIN32 && !defined(_M_ARM64)
	SetProcessDPIAware();
	//SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif
#if !RVE_SERVER

	{
		//make the default texture white
		uint8_t data[] = {0xFF,0xFF,0xFF,0xFF};
		Texture::Manager::defaultTexture = make_shared<RuntimeTexture>(1, 1, Texture::Config{
			.mipLevels = 1,
			.numLayers = 1,
			.initialData = {{reinterpret_cast<std::byte*>(data),sizeof(data)}}
			});
		
		uint8_t normalData[] = {256/2,256/2, 0xFF,0xFF};
		Texture::Manager::defaultNormalTexture = New<RuntimeTexture>(1,1, Texture::Config{
			.mipLevels = 1,
			.numLayers = 1,
			.initialData = {{reinterpret_cast<std::byte*>(normalData),sizeof(normalData)}}
		});

		uint8_t zeroData[] = { 0,0,0,0 };
		Texture::Manager::zeroTexture = New<RuntimeTexture>(1, 1, Texture::Config{
			.mipLevels = 1,
			.numLayers = 1,
			.initialData = {{reinterpret_cast<std::byte*>(zeroData), sizeof(zeroData)}}
		});
	}
#endif

	//invoke startup hook
	OnStartup(argc, argv);
	
	lastFrameTime = clocktype::now();
   
#if !RVE_SERVER
    float windowScaleFactor = GetMainWindow()->GetDPIScale();
    SDL_Event event;
#endif
	bool exit = false;
	
	while (!exit) {
#if __APPLE__
		@autoreleasepool{
#endif

		//setup framerate scaling for next frame
		auto now = clocktype::now();
		//will cause engine to run in slow motion if the frame rate is <= 1fps
		deltaTimeMicroseconds = std::min(duration_cast<timeDiff>(now - lastFrameTime), maxTimeStep);
		float deltaSeconds = std::chrono::duration<decltype(deltaSeconds)>(deltaTimeMicroseconds).count();
		time += deltaSeconds;
		currentScale = deltaSeconds * evalNormal;
#if !RVE_SERVER
		RVE_PROFILE_SECTION(events, "Process all Events");
		auto windowflags = SDL_GetWindowFlags(window->window);
		while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    exit = true;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                    Renderer->mainCommandQueue->WaitUntilCompleted();
                    window->NotifySizeChanged(event.window.data1, event.window.data2);
                    windowScaleFactor = GetMainWindow()->GetDPIScale();
                    {
                        auto size = window->GetSizeInPixels();
                        Renderer->ResizeRenderTargetCollection(mainWindowView.collection, {uint32_t(size.width), uint32_t(size.height)});
                    }
                    break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    exit = true;
                    break;
            }
			//process others
			if (inputManager) {
				inputManager->ProcessInput(event,windowflags,currentScale, window->windowdims.width, window->windowdims.height, windowScaleFactor);
#ifndef NDEBUG
				RenderEngine::debuggerInput->ProcessInput(event,windowflags,currentScale, window->windowdims.width, window->windowdims.height, windowScaleFactor);
#endif
			}
		}
        RVE_PROFILE_SECTION_END(events);
#endif // !RVE_SERVER
        Tick();
#if RVE_SERVER
        //make up the difference
        // because there's no vsync on server builds, we need to add delay
        //can't just call sleep because sleep is not very accurate
        clocktype::duration work_time;
        do{
            auto workEnd = clocktype::now();
            work_time = workEnd - now;
            auto delta = min_tick_time - work_time;
            if (delta > std::chrono::duration<double, std::milli>(3)) {
                auto dc = std::chrono::duration_cast<std::chrono::milliseconds>(delta);
                std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(dc.count()-1));
            }
        }while (work_time < min_tick_time);
#endif
            lastFrameTime = now;
#if __APPLE__
		}	// end of @autoreleasepool
#endif
	}
	
    return OnShutdown();
}

void App::Tick(){
#if __APPLE__
    @autoreleasepool{
#endif
        
#if !RVE_SERVER
        RVE_PROFILE_SECTION(getSwapchain, "Acquire Swapchain Image");
        RGL::SwapchainPresentConfig swapchainPresentConfig;
        window->QueueGetNextSwapchainImage(swapchainPresentConfig);
#ifndef NDEBUG
        RenderEngine::debuggerInput->TickAxes();
#endif
        if (inputManager) {
            inputManager->TickAxes();
        }
#endif

#if !RVE_SERVER
        auto windowSize = window->GetSizeInPixels();
        auto scale = window->GetDPIScale();
#endif
        RVE_PROFILE_SECTION(tickallworlds, "Tick All Worlds");
        //tick all worlds
        for (const auto world : loadedWorlds) {
            world->Tick(currentScale);
#if !RVE_SERVER
            world->Filter([=](GUIComponent& gui) {
                if (gui.Mode == GUIComponent::RenderMode::Screenspace) {
                    gui.SetDimensions(windowSize.width, windowSize.height);
                    gui.SetDPIScale(scale);
                }
                gui.Update();
            });
#endif
        }

        //process main thread tasks
        {
            Function<void(void)> front;
            while (main_tasks.try_dequeue(front)) {
                front();
            }
        }
        RVE_PROFILE_SECTION_END(tickallworlds);
#if !RVE_SERVER

        // get the cameras to render
        auto allCameras = renderWorld->GetAllComponentsOfType<CameraComponent>();

        if (!allCameras)
        {
            Debug::Fatal("Cannot render: World does not have a camera!");
        }
        mainWindowView.camDatas.clear();
        
        constexpr auto MakeCamData = [](const auto& camera, uint32_t width, uint32_t height){
            auto projOnly = camera.GenerateProjectionMatrix(width, height);
            auto viewOnly = camera.GenerateViewMatrix();
            auto viewProj = projOnly * viewOnly;
            auto camPos = camera.GetOwner().GetTransform().GetWorldPosition();
            
            auto viewportOverride = camera.viewportOverride;
            
            return RenderViewCollection::camData{ viewProj, projOnly, viewOnly, camPos,{camera.nearClip, camera.farClip} ,viewportOverride, camera.renderLayers, camera.FOV, width, height, &camera.postProcessingEffects};
        };
        std::vector<RenderViewCollection> allViews;
        for(const auto& camera : *allCameras){
            if (!camera.IsActive()) {
                continue;
            }
            if (!camera.target){
                continue;   // only want render texture cameras
            }
            
            auto& collection = camera.target->GetCollection();
            auto size = collection.depthStencil->GetSize();
            allViews.push_back({collection, {MakeCamData(camera, size.width, size.height)}, {static_cast<int>(size.width), static_cast<int>(size.height)}});
        }
        
        for (const auto& camera : *allCameras) {
            if (!camera.IsActive()) {
                continue;
            }
            if (camera.target){
                continue;   // no render texture cameras
            }
           
            mainWindowView.camDatas.push_back(MakeCamData(camera, windowSize.width, windowSize.height));
        }

        mainWindowView.pixelDimensions = window->GetSizeInPixels();

#ifdef RVE_XR_AVAILABLE
        // update OpenXR data if it is requested
        std::pair<std::vector<XrView>, XrFrameState> xrBeginData;
        if (wantsXR) {
            xrBeginData = OpenXRIntegration::BeginXRFrame();
            OpenXRIntegration::UpdateXRTargetCollections(xrRenderViewCollections, xrBeginData.first);
            allViews.insert(allViews.end(), xrRenderViewCollections.begin(), xrRenderViewCollections.end());
        }
#endif

        auto nextTexture = window->BlockGetNextSwapchainImage(swapchainPresentConfig);
        RVE_PROFILE_SECTION_END(getSwapchain);
        mainWindowView.collection.finalFramebuffer = nextTexture.texture;
        allViews.push_back(mainWindowView);
        auto mainCommandBuffer = Renderer->Draw(renderWorld, allViews, scale);


        // show the results to the user
        RGL::CommitConfig commitconfig{
            .signalFence = window->swapchainFence,
        };
        mainCommandBuffer->Commit(commitconfig);
        
        window->swapchain->Present(nextTexture.presentConfig);
        Profile::EndTick();

#ifdef RVE_XR_AVAILABLE
        if (wantsXR) {
            OpenXRIntegration::EndXRFrame(xrBeginData.second);
        }
#endif
        if (GetAudioActive()) {
            player->SetWorld(renderWorld);
        }
	skip_xr_frame:
		;	// dummy statement for the label
#endif
        
#if __APPLE__
    }   // end of autoreleasepool
#endif
}

float App::CurrentTPS() {
	return App::evalNormal / currentScale;
}

/**
Set the current world to tick automatically
@param newWorld the new world
*/

void RavEngine::App::SetRenderedWorld(Ref<World> newWorld) {
	if (!loadedWorlds.contains(newWorld)) {
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

/**
Add a world to be ticked
@param world the world to tick
*/

void RavEngine::App::AddWorld(Ref<World> world) {
	loadedWorlds.insert(world);
	if (!renderWorld) {
		SetRenderedWorld(world);
	}

	// synchronize network if necessary
	if (networkManager.IsClient() && !networkManager.IsServer()) {
		networkManager.client->SendSyncWorldRequest(world);
	}
}

/**
Remove a world from the tick list
@param world the world to tick
*/

void RavEngine::App::RemoveWorld(Ref<World> world) {
	loadedWorlds.erase(world);
	if (renderWorld == world) {
		renderWorld->OnDeactivate();
		renderWorld.reset();    //this will cause nothing to render, so set a different world as rendered
	}
}

/**
* Unload all worlds
*/

void RavEngine::App::RemoveAllWorlds() {
	for (const auto& world : loadedWorlds) {
		RemoveWorld(world);
	}
}

/**
Replace a loaded world with a different world, transferring render state if necessary
@param oldWorld the world to replace
@param newWorld the world to replace with. Cannot be already loaded.
*/

void RavEngine::App::AddReplaceWorld(Ref<World> oldWorld, Ref<World> newWorld) {
	AddWorld(newWorld);
	bool updateRender = renderWorld == oldWorld;
	RemoveWorld(oldWorld);
	if (updateRender) {
		SetRenderedWorld(newWorld);
	}
}

void App::Quit(){
#if !RVE_SERVER
	SDL_Event event;
	event.type = SDL_EVENT_QUIT;
	SDL_PushEvent(&event);
#else
    Debug::Fatal("Quit is not implemented on the server (TODO)");
#endif
}

App::~App(){
    if (!PHYSFS_isInit()){  // unit tests do not initialize the vfs, so we don't want to procede here
        return;
    }

#if !RVE_SERVER
	// ensure the GPU is done doing work
	window->BlockGetNextSwapchainImage({});
#ifndef NDEBUG
	Renderer->DeactivateDebugger();
#endif
#endif
    MeshAsset::Manager::Clear();
    MeshAssetSkinned::Manager::Clear();
    
#if !RVE_SERVER

	Texture::Manager::defaultTexture.reset();
	Texture::Manager::defaultNormalTexture.reset();
	Texture::Manager::zeroTexture.reset();
    Texture::Manager::Clear();
	if (GetAudioActive()) {
		player->Shutdown();
	}
#endif
	networkManager.server.reset();
	networkManager.client.reset();

#if !RVE_SERVER
	inputManager = nullptr;
#endif
	renderWorld = nullptr;
	loadedWorlds.clear();

	GameNetworkingSockets_Kill();
	PHYSFS_deinit();
#if !RVE_SERVER

	auto fsi = Rml::GetFileInterface();
	Rml::Shutdown();
    Renderer.reset();
    delete fsi;
#endif

	currentApp = nullptr;
}

#if !RVE_SERVER
void App::SetWindowTitle(const char *title){
	SDL_SetWindowTitle(window->window, title);
}
#endif

std::optional<Ref<World>> RavEngine::App::GetWorldByName(const std::string& name) {
	std::optional<Ref<World>> value;
	for (const auto& world : loadedWorlds) {
		// because std::string "world\0\0" != "world", we need to use strncmp
		if (std::strncmp(world->worldID.data(), name.data(), World::id_size) == 0) {
			value.emplace(world);
			break;
		}
	}
	return value;
}

void App::OnDropAudioWorklets(uint32_t nDropped){
    Debug::Warning("Dropped {} audio tasks.", nDropped);
}

bool RavEngine::App::GetAudioActive() const
{
#if !RVE_SERVER
	return player.operator bool();
#else
	return false;
#endif
}


App* RavEngine::GetApp()
{
	return currentApp;
}
