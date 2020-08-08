//
//  GameplayStatics.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "GameplayStatics.hpp"

#include <SDL.h>
#include <SDL_syswm.h>

//startup world
Ref<World> GameplayStatics::currentWorld;
GameplayStatics::vs GameplayStatics::VideoSettings;
Ref<InputSystem> GameplayStatics::inputManager;
int GameplayStatics::width = VideoSettings.width, GameplayStatics::height = VideoSettings.height;
GameplayStatics::static_constructor GameplayStatics::statcons;
//SDL_Window* GameplayStatics::mainWindow = nullptr;


using namespace std;

/**
 Create an SDL window for different platforms, and reference it to bgfx
 @param _window the SDL window pointer (modified)
 @note supported platforms: Linux, BSD, OSX, Windows, SteamLink
 */
//inline bool sdlSetWindow(SDL_Window* _window)
//{
//	SDL_SysWMinfo wmi;
//	SDL_VERSION(&wmi.version);
//	if (!SDL_GetWindowWMInfo(_window, &wmi)) {
//		return false;
//	}
//	
//	bgfx::PlatformData pd;
//#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
//	pd.ndt = wmi.info.x11.display;
//	pd.nwh = (void*)(uintptr_t)wmi.info.x11.window;
//#elif BX_PLATFORM_OSX
//	pd.ndt = NULL;
//	pd.nwh = wmi.info.cocoa.window;
//#elif BX_PLATFORM_WINDOWS
//	pd.ndt = NULL;
//	pd.nwh = wmi.info.win.window;
//#elif BX_PLATFORM_STEAMLINK
//	pd.ndt = wmi.info.vivante.display;
//	pd.nwh = wmi.info.vivante.window;
//#endif // BX_PLATFORM_
//	pd.context = NULL;
//	pd.backBuffer = NULL;
//	pd.backBufferDS = NULL;
//	bgfx::setPlatformData(pd);
//	
//	return true;
//}

///**
// Call BGFX to reset the screen
// */
//inline void reset_screen(int width, int height) {
//	bgfx::reset(width, height, GameplayStatics::VideoSettings.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
//	bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
//}


GameplayStatics::static_constructor::static_constructor()
{
	//SDL_Init(0);
	//SDL_Init(SDL_INIT_GAMECONTROLLER);

	//mainWindow = SDL_CreateWindow("RavEngine Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	//sdlSetWindow(mainWindow);
	//bgfx::renderFrame();

	////setup BGFX
	//{
	//	bgfx::init();

	//	bgfx::reset(width, height, VideoSettings.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);		//add BGFX_RESET_VSYNC as an argument to enable vsync. Other flags: https://bkaradzic.github.io/bgfx/bgfx.html#reset

	//	// Enable debug text.
	//	bgfx::setDebug(BGFX_DEBUG_TEXT /*| BGFX_DEBUG_STATS*/);

	//	// Set view 0 clear state.
	//	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	//}

	//SDL_GetWindowSize(mainWindow, &width, &height);
	//reset_screen(width, height);
}

///**
// Start up the game
// @param startupWorld the world object to use as the initial world
// @note Uses values defined in GameplayStatics::VideoSettings when starting
// */
//void GameplayStatics::StartGame(Ref<World> startupWorld, const string& titlebarText){
//	
//	currentWorld = startupWorld;
//	inputManager->InitGameControllers();
//	
//#ifdef _WIN32
//	{
//		//convert to a wide string (note: only works if characters are a single byte)
//		string temp = titlebarText + " Output Window";
//		wstring wide(temp.begin(), temp.end());
//		//SetConsoleTitle(wide.c_str());
//	}
//#endif
//
//	SDL_SetWindowTitle(mainWindow, titlebarText.c_str());
//	
//	const auto title = string("RavEngine 0.0.1a - ") + RenderEngine::currentBackend();
//			
//	//process events first
//	bool exit = false;
//	SDL_Event event;
//	while (!exit) {
//		while (SDL_PollEvent(&event)) {
//			
//			switch (event.type) {
//				case SDL_QUIT:
//					exit = true;
//					break;
//					
//				case SDL_WINDOWEVENT: {
//					const SDL_WindowEvent& wev = event.window;
//					switch (wev.event) {
//						case SDL_WINDOWEVENT_RESIZED:
//						case SDL_WINDOWEVENT_SIZE_CHANGED:
//							SDL_GetWindowSize(mainWindow, &width, &height);
//							reset_screen(width, height);
//							break;
//							
//						case SDL_WINDOWEVENT_CLOSE:
//							exit = true;
//							break;
//					}
//				} break;
//				
//				case SDL_KEYDOWN:
//				case SDL_KEYUP:
//					inputManager->SDL_key(event.key.state, event.key.keysym.scancode);
//					break;
//				case SDL_MOUSEMOTION:
//					inputManager->SDL_mousemove((float)event.motion.x / width, (float)event.motion.y / height, event.motion.xrel, event.motion.yrel);
//					break;
//				case SDL_MOUSEBUTTONDOWN:
//				case SDL_MOUSEBUTTONUP:
//					inputManager->SDL_mousekey(event.button.state, event.button.button);
//					break;
//				case SDL_CONTROLLERAXISMOTION:
//				case SDL_CONTROLLER_AXIS_LEFTX:
//				case SDL_CONTROLLER_AXIS_LEFTY:
//					inputManager->SDL_ControllerAxis(event.caxis.axis + Special::CONTROLLER_AXIS_OFFSET, (event.caxis.value ) / ((float)SHRT_MAX));
//					break;
//				case SDL_CONTROLLERBUTTONDOWN:
//				case SDL_CONTROLLERBUTTONUP:
//					inputManager->SDL_mousekey(event.cbutton.state + Special::CONTROLLER_BUTTON_OFFSET, event.cbutton.button);
//					break;
//				case SDL_CONTROLLERDEVICEADDED:
//				case SDL_CONTROLLERDEVICEREMOVED:
//					break;
//				}
//		}
//
//		//blank screen to prepare for frame
//		bgfx::touch(0);
//		bgfx::dbgTextClear();
//		bgfx::dbgTextPrintf(0, 0, 0x4f,title.c_str());
//
//		//run the world
//		GameplayStatics::currentWorld->tick();
//
//		//render and advance to next frame
//		//bgfx::renderFrame();		//for multithreaded draw
//		bgfx::frame();
//	}
//	
//	bgfx::shutdown();
//	
//	//for multithreaded draw
//	//	while (bgfx::RenderFrame::NoContext != bgfx::renderFrame()) {
//	//	};
//	
//	SDL_DestroyWindow(mainWindow);
//	SDL_Quit();
//    
//    //unload physx
//    PhysicsSolver::ReleaseStatics();
//}
