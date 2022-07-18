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
#include "Skybox.hpp"
#include <SDL.h>
#include <filesystem>
#include "Function.hpp"
#include "World.hpp"
#include "GetApp.hpp"
#include "Defines.hpp"

#if XR_AVAILABLE
#include <openxr/openxr.h>
static XrSessionState xr_session_state = XR_SESSION_STATE_UNKNOWN;
extern XrSession rve_xr_session;	//TODO: defined in RenderEngine_XR.cpp, but probably shouldn't be
extern XrSpace rve_xr_app_space;
extern XrInstance rve_xr_instance;
extern XrDebugUtilsMessengerEXT rve_xr_debug;
extern XrViewConfigurationType rve_app_config_view;
static struct input_state_t {
	XrActionSet actionSet;
	XrAction    poseAction;
	XrAction    selectAction;
	XrPath   handSubactionPath[2];
	XrSpace  handSpace[2];
	XrPosef  handPose[2];
	XrBool32 renderHand[2];
	XrBool32 handSelect[2];
} xr_input_state;

constexpr static XrPosef xr_pose_identity = { {0,0,0,1}, {0,0,0} };

static void openxr_make_actions() {
	XrActionSetCreateInfo actionset_info = { XR_TYPE_ACTION_SET_CREATE_INFO };
	strcpy(actionset_info.actionSetName, "gameplay");
	strcpy(actionset_info.localizedActionSetName, "Gameplay");
	XR_CHECK(xrCreateActionSet(rve_xr_instance, &actionset_info, &xr_input_state.actionSet));
	XR_CHECK(xrStringToPath(rve_xr_instance, "/user/hand/left", &xr_input_state.handSubactionPath[0]));
	XR_CHECK(xrStringToPath(rve_xr_instance, "/user/hand/right", &xr_input_state.handSubactionPath[1]));

	// Create an action to track the position and orientation of the hands! This is
	// the controller location, or the center of the palms for actual hands.
	XrActionCreateInfo action_info = { XR_TYPE_ACTION_CREATE_INFO };
	action_info.countSubactionPaths = BX_COUNTOF(xr_input_state.handSubactionPath);
	action_info.subactionPaths = xr_input_state.handSubactionPath;
	action_info.actionType = XR_ACTION_TYPE_POSE_INPUT;
	strcpy(action_info.actionName, "hand_pose");
	strcpy(action_info.localizedActionName, "Hand Pose");
	XR_CHECK(xrCreateAction(xr_input_state.actionSet, &action_info, &xr_input_state.poseAction));

	// Create an action for listening to the select action! This is primary trigger
	// on controllers, and an airtap on HoloLens
	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	strcpy(action_info.actionName, "select");
	strcpy(action_info.localizedActionName, "Select");
	XR_CHECK(xrCreateAction(xr_input_state.actionSet, &action_info, &xr_input_state.selectAction));

	// Bind the actions we just created to specific locations on the Khronos simple_controller
	// definition! These are labeled as 'suggested' because they may be overridden by the runtime
	// preferences. For example, if the runtime allows you to remap buttons, or provides input
	// accessibility settings.
	XrPath profile_path;
	XrPath pose_path[2];
	XrPath select_path[2];
	XR_CHECK(xrStringToPath(rve_xr_instance, "/user/hand/left/input/grip/pose", &pose_path[0]));
	XR_CHECK(xrStringToPath(rve_xr_instance, "/user/hand/right/input/grip/pose", &pose_path[1]));
	XR_CHECK(xrStringToPath(rve_xr_instance, "/user/hand/left/input/select/click", &select_path[0]));
	XR_CHECK(xrStringToPath(rve_xr_instance, "/user/hand/right/input/select/click", &select_path[1]));
	XR_CHECK(xrStringToPath(rve_xr_instance, "/interaction_profiles/khr/simple_controller", &profile_path));
	XrActionSuggestedBinding bindings[] = {
		{ xr_input_state.poseAction,   pose_path[0]   },
		{ xr_input_state.poseAction,   pose_path[1]   },
		{ xr_input_state.selectAction, select_path[0] },
		{ xr_input_state.selectAction, select_path[1] }, };
	XrInteractionProfileSuggestedBinding suggested_binds = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	suggested_binds.interactionProfile = profile_path;
	suggested_binds.suggestedBindings = &bindings[0];
	suggested_binds.countSuggestedBindings = BX_COUNTOF(bindings);
	XR_CHECK(xrSuggestInteractionProfileBindings(rve_xr_instance, &suggested_binds));

	// Create frames of reference for the pose actions
	for (int32_t i = 0; i < 2; i++) {
		XrActionSpaceCreateInfo action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.action = xr_input_state.poseAction;
		action_space_info.poseInActionSpace = xr_pose_identity;
		action_space_info.subactionPath = xr_input_state.handSubactionPath[i];
		XR_CHECK(xrCreateActionSpace(rve_xr_session, &action_space_info, &xr_input_state.handSpace[i]));
	}

	// Attach the action set we just made to the session
	XrSessionActionSetsAttachInfo attach_info = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attach_info.countActionSets = 1;
	attach_info.actionSets = &xr_input_state.actionSet;
	XR_CHECK(xrAttachSessionActionSets(rve_xr_session, &attach_info));
}

static void XrShutdown() {
	if (xr_input_state.actionSet != XR_NULL_HANDLE) {
		if (xr_input_state.handSpace[0] != XR_NULL_HANDLE) XR_CHECK(xrDestroySpace(xr_input_state.handSpace[0]));
		if (xr_input_state.handSpace[1] != XR_NULL_HANDLE) XR_CHECK(xrDestroySpace(xr_input_state.handSpace[1]));
		XR_CHECK(xrDestroyActionSet(xr_input_state.actionSet));

		if (rve_xr_app_space != XR_NULL_HANDLE) XR_CHECK(xrDestroySpace(rve_xr_app_space));
		if (rve_xr_session != XR_NULL_HANDLE) XR_CHECK(xrDestroySession(rve_xr_session));
		
		if (rve_xr_instance != XR_NULL_HANDLE) XR_CHECK(xrDestroyInstance(rve_xr_instance));
	}
}
#endif

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

App::App(const std::string& resourcesName) : App(){
	// crash signal handlers
	::signal(SIGSEGV, &crash_signal_handler);
	::signal(SIGABRT, &crash_signal_handler);

	//initialize virtual file system library 
	PHYSFS_init("");
	
	Resources.emplace(StrFormat("{}.rvedata",resourcesName));
}

App::App(){
    currentApp = this;
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
#if XR_AVAILABLE
	if (wantsXR) {
		// setup actions
		openxr_make_actions();
	}
#endif
	
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
		auto p = Filesystem::Path(filename);
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
	// for some weird reason, it's not present on ARM
#if defined _WIN32 && !_WINRT && !defined(_M_ARM64)
	SetProcessDPIAware();
	//SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
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

		// process OpenXR actions and events
#if XR_AVAILABLE
		if (wantsXR) {
			if (xr_session_state == XR_SESSION_STATE_FOCUSED) {
				// process actions
				{
					// update action set with new inputs
					XrActiveActionSet action_set{ };
					action_set.actionSet = xr_input_state.actionSet;
					action_set.subactionPath = XR_NULL_PATH;

					XrActionsSyncInfo sync_info{ XR_TYPE_ACTIONS_SYNC_INFO };
					sync_info.countActiveActionSets = 1;
					sync_info.activeActionSets = &action_set;

					XR_CHECK(xrSyncActions(rve_xr_session, &sync_info));


					for (uint32_t hand = 0; hand < 2; hand++) {
						XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
						get_info.subactionPath = xr_input_state.handSubactionPath[hand];

						XrActionStatePose pose_state = { XR_TYPE_ACTION_STATE_POSE };
						get_info.action = xr_input_state.poseAction;
						XR_CHECK(xrGetActionStatePose(rve_xr_session, &get_info, &pose_state));
						xr_input_state.renderHand[hand] = pose_state.isActive;

						// Events come with a timestamp
						XrActionStateBoolean select_state = { XR_TYPE_ACTION_STATE_BOOLEAN };
						get_info.action = xr_input_state.selectAction;
						XR_CHECK(xrGetActionStateBoolean(rve_xr_session, &get_info, &select_state));
						xr_input_state.handSelect[hand] = select_state.currentState && select_state.changedSinceLastSync;

						// If we have a select event, update the hand pose to match the event's timestamp
						if (xr_input_state.handSelect[hand]) {
							XrSpaceLocation space_location = { XR_TYPE_SPACE_LOCATION };
							XrResult res = xrLocateSpace(xr_input_state.handSpace[hand], rve_xr_app_space, select_state.lastChangeTime, &space_location);
							if (XR_UNQUALIFIED_SUCCESS(res) &&
								(space_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
								(space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {
								xr_input_state.handPose[hand] = space_location.pose;
							}
						}
					}
				}
			}
			// process these regardless of focused state
			{
				XrEventDataBuffer event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
				while (xrPollEvent(rve_xr_instance, &event_buffer) == XR_SUCCESS) {
					switch (event_buffer.type) {
					case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
						XrEventDataSessionStateChanged* changed = (XrEventDataSessionStateChanged*)&event_buffer;
						xr_session_state = changed->state;

						// Session state change is where we can begin and end sessions, as well as find quit messages!
						switch (xr_session_state) {
						case XR_SESSION_STATE_READY: {
							XrSessionBeginInfo begin_info = { XR_TYPE_SESSION_BEGIN_INFO };
							begin_info.primaryViewConfigurationType = rve_app_config_view;
							XR_CHECK(xrBeginSession(rve_xr_session, &begin_info));
						} break;
						case XR_SESSION_STATE_STOPPING: {
							exit = true;
							XR_CHECK(xrEndSession(rve_xr_session));
						}
						break;
						case XR_SESSION_STATE_EXITING:
						case XR_SESSION_STATE_LOSS_PENDING:
							exit = true;
							break;
						}
					}
					break;
					case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
						exit = true;
						break;
					}
					event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
				}
			}
		}
#endif
	
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

#if XR_AVAILABLE
		XrFrameState state{ XR_TYPE_FRAME_STATE };
		if (wantsXR) {
			XrFrameWaitInfo fwinfo{ XR_TYPE_FRAME_WAIT_INFO };
			fwinfo.next = nullptr;
			state.next = nullptr;
			XR_CHECK(xrWaitFrame(rve_xr_session,&fwinfo,&state));

			XrFrameBeginInfo fbinfo{ XR_TYPE_FRAME_BEGIN_INFO };
			fbinfo.next = nullptr;
			XR_CHECK(xrBeginFrame(rve_xr_session, &fbinfo));
			if (state.shouldRender == XR_FALSE) {
				goto skip_xr_frame;
			}
		}
#endif
		Renderer->Draw(renderWorld);		// in XR, this must be synchronous, to ensure xrEndFrame is called properly
#if XR_AVAILABLE
		if (wantsXR) {
			skip_xr_frame:
			Renderer->SignalXRFrameEnd(state.predictedDisplayTime);	// calls xrEndFrame
		}
		
#endif
		player.SetWorld(renderWorld);
		
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
    if (!PHYSFS_isInit()){  // unit tests do not initialize the vfs, so we don't want to procede here
        return;
    }
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
#if XR_AVAILABLE
	Renderer->ShutdownXR();
	XrShutdown();
#endif
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
