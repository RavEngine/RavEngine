#pragma once
#include <chrono>
#include "RenderEngine.hpp"
#include "VirtualFileSystem.hpp"
#include <functional>
#include <concurrentqueue.h>
#include "SpinLock.hpp"
#include <thread>
#include <taskflow/taskflow.hpp>
#include "DataStructures.hpp"
#include "AudioPlayer.hpp"
#include "NetworkManager.hpp"
#include "FrameData.hpp"
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
	#undef SDL_MAIN_NEEDED
	#undef SDL_MAIN_AVAILABLE
	#define _WINRT 1
#endif
#include <SDL_main.h>
#include <bx/platform.h>
#include <optional>
#include "AudioSnapshot.hpp"
#include "GetApp.hpp"

namespace RavEngine {

	struct AppConfig {
		enum class RenderBackend {
			Metal,
			DirectX12,
			Vulkan,
            WebGPU,
			AutoSelect
		} preferredBackend = RenderBackend::AutoSelect;
	};

	typedef std::chrono::high_resolution_clock clocktype;
	typedef std::chrono::duration<double, std::micro> timeDiff;
	typedef std::chrono::seconds deltaSeconds;
	typedef std::chrono::time_point<clocktype> timePoint;
	
	class InputManager;
	class World;

	class App {
		friend class NetworkManager;
        
        std::optional<RenderEngine> Renderer;
        std::optional<VirtualFilesystem> Resources;
	public:
		App(const std::string& resourcesName);
        App();  // for unit tests only, do not use
		virtual ~App();

		// set this to true in app constructor if XR is desired
		bool wantsXR = false;
        
        /**
         Override this method to provide a custom fatal handler
         */
        virtual void OnFatal(const char* msg){}
		
		/**
		 Signal to gracefully shut down the application
		 */
		void Quit();
        
        /**
         Set the minimum tick time. If the work for the tick completes faster than this time interval,
         the main thread will sleep for the remaining time. To unlock the tick rate (not advised), set
         this to 0.
         @param min_ms the minimum amount of time a tick should take.
         */
        template<typename T>
        constexpr void SetMinTickTime(std::chrono::duration<double,T> min_ms){
            min_tick_time = min_ms;
        }

		/**
		Invoked automatically. Passes command line arguments.
		*/
		int run(int argc, char** argv);

		constexpr static float evalNormal = 60;	//normal speed is 60 hz
		
		/**
		 @return the current time, measured in seconds since the application launched
		 */
        inline double GetCurrentTime(){
			return time;
		};
		
		//number of cores on device
		const int numcpus = std::thread::hardware_concurrency();
		
		//global thread pool, threads = logical processors on CPU
        tf::Executor executor{
#ifdef __EMSCRIPTEN__
        1 // use main thread only on emscripten
#endif
        };
		
		//networking interface
		NetworkManager networkManager;
        
        inline VirtualFilesystem& GetResources(){
            return Resources.value();
        }
        
        inline RenderEngine& GetRenderEngine(){
            return Renderer.value();
        }
        
        inline AudioPlayer& GetAudioPlayer(){
            return player;
        }
        
        inline bool HasRenderEngine(){
            return static_cast<bool>(Renderer);
        }
		
		/**
		 Dispatch a task to be executed on the main thread.
		 @param f the block to execute
		 @note To pass parameters, do not reference! Instead, you must explicitly copy the values you want to pass:
		 @code
 int x = 5; int y = 6;
 RavEngine::GetApp()->DispatchMainThread([=]{
	std::cout << x << y << std::endl;
 });
		 @endcode
		 */
        template<typename T>
		constexpr inline void DispatchMainThread(const T& f){
			main_tasks.enqueue(f);
		}

		/**
		@return the current application tick rate
		*/
        float CurrentTPS();
        float GetCurrentFPSScale() const{
            return currentScale;
        }
		
		Ref<InputManager> inputManager;

		/**
		Set the current world to tick automatically
		@param newWorld the new world
		*/
		void SetRenderedWorld(Ref<World> newWorld) {
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
		void AddWorld(Ref<World> world) {
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
		void RemoveWorld(Ref<World> world) {
			loadedWorlds.erase(world);
			if (renderWorld == world) {
				renderWorld->OnDeactivate();
				renderWorld.reset();    //this will cause nothing to render, so set a different world as rendered
			}
		}

		/**
		* Unload all worlds
		*/
        void RemoveAllWorlds() {
			for (const auto& world : loadedWorlds) {
				RemoveWorld(world);
			}
		}
		
		/**
		 Replace a loaded world with a different world, transferring render state if necessary
		 @param oldWorld the world to replace
		 @param newWorld the world to replace with. Cannot be already loaded.
		 */
		void AddReplaceWorld(Ref<World> oldWorld, Ref<World> newWorld){
			AddWorld(newWorld);
			bool updateRender = renderWorld == oldWorld;
			RemoveWorld(oldWorld);
			if (updateRender){
				SetRenderedWorld(newWorld);
			}
		}
		
		/**
		 Set the window titlebar text
		 @param title the text for the titlebar
		 @note Do not call this every frame. To update periodically with data such as frame rates, use a scheduled system.
		 */
        void SetWindowTitle(const char* title);
		
		std::optional<Ref<World>> GetWorldByName(const std::string& name) {
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

		auto GetCurrentRenderWorld()  {
			return renderWorld;
		}
		
        inline FrameData* GetCurrentFramedata(){
			return current;
		}
		
		inline FrameData* GetRenderFramedata(){
			return render;
		}
		
        inline void SwapCurrentFramedata(){
			swapmtx1.lock();
			std::swap(current,inactive);
			swapmtx1.unlock();
		}
        inline void SwapRenderFramedata(){
			swapmtx2.lock();
			std::swap(inactive,render);
			swapmtx2.unlock();
		}
        
        inline void SwapCurrrentAudioSnapshot(){
            audiomtx1.lock();
            std::swap(acurrent,ainactive);
            audiomtx1.unlock();
        }
        inline void SwapRenderAudioSnapshot(){
            audiomtx2.lock();
            std::swap(ainactive,arender);
            audiomtx2.unlock();
        }
        inline AudioSnapshot* GetCurrentAudioSnapshot(){
            return acurrent;
        }
        inline AudioSnapshot* GetRenderAudioSnapshot(){
            return arender;
        }

	private:
        float currentScale = 0.01f;
        
		Ref<World> renderWorld;
	
		ConcurrentQueue<Function<void(void)>> main_tasks;

        //change to adjust the ticking speed of the engine (default 90hz)
		std::chrono::duration<double, std::micro> min_tick_time{ std::chrono::duration<double,std::milli>(1.0 / 90 * 1000)};
		
		locked_hashset<Ref<World>,SpinLock> loadedWorlds;
		
		//triple-buffer framedata
		FrameData f1, f2, f3, *current = &f1, *inactive = &f2, *render = &f3;
		SpinLock swapmtx1, swapmtx2;
        
        AudioSnapshot a1, a2, a3, *acurrent = &a1, *ainactive = &a2, *arender = &a3;
        SpinLock audiomtx1, audiomtx2;
	protected:
		virtual AppConfig OnConfigure(int argc, char** argv) { return AppConfig{}; }
		
		//plays the audio generated in worlds
		AudioPlayer player;
		
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

		//last frame time, frame delta time, framerate scale, maximum frame time
		timePoint lastFrameTime;
		timeDiff deltaTimeMicroseconds{0};
		const timeDiff maxTimeStep = std::chrono::milliseconds((long)1000);
		
		double time = 0;
	};
}
#ifdef _WINRT
// UWP startup requires extra effort
#undef main
#define START_APP(APP) \
int DoProgram(int argc, char** argv){\
auto a = std::make_unique<APP>(); return a->run(argc, argv);\
}\
int main(int argc, char** argv) { \
	return SDL_WinRTRunApp(DoProgram, NULL);\
}
#else
#define START_APP(APP) int main(int argc, char** argv){auto a = std::make_unique<APP>(); return a->run(argc, argv);}
#endif
