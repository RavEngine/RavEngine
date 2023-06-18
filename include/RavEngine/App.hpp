#pragma once
#include <chrono>
#include <concurrentqueue.h>
#include "SpinLock.hpp"
#include <taskflow/taskflow.hpp>
#include "NetworkManager.hpp"
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
	#undef SDL_MAIN_NEEDED
	#undef SDL_MAIN_AVAILABLE
	#define _WINRT 1
#endif
#include <SDL_main.h>
#include <optional>
#include "AudioSnapshot.hpp"
#include "GetApp.hpp"

namespace RavEngine {

struct RenderEngine;
struct VirtualFilesystem;
struct FrameData;
struct AudioPlayer;

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
        
        std::unique_ptr<RenderEngine> Renderer;
        std::unique_ptr<VirtualFilesystem> Resources;
	public:
        App();
		virtual ~App();

		// set this to true in app constructor if XR is desired
		bool wantsXR = false;
        
        /**
         Override this method to provide a custom fatal handler
         */
        virtual void OnFatal(const char* msg){}
        
        /**
         Override to be notified if too much audio work was submitted. The default implementation logs a warning.
         */
        virtual void OnDropAudioWorklets(uint32_t ndropped);
		
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
#else
            std::max<size_t>(std::thread::hardware_concurrency() - 2, 1)    // for audio - TODO: make configurable
#endif
        };
		
		//networking interface
		NetworkManager networkManager;
        
        inline VirtualFilesystem& GetResources(){
            return *Resources.get();
        }
        
        inline RenderEngine& GetRenderEngine(){
            return *Renderer.get();
        }
        
        inline auto& GetAudioPlayer(){
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
		void SetRenderedWorld(Ref<World> newWorld);
		
		/**
		 Add a world to be ticked
		 @param world the world to tick
		 */
		void AddWorld(Ref<World> world);

		/**
		Remove a world from the tick list
		@param world the world to tick
		*/
		void RemoveWorld(Ref<World> world);

		/**
		* Unload all worlds
		*/
        void RemoveAllWorlds();
		
		/**
		 Replace a loaded world with a different world, transferring render state if necessary
		 @param oldWorld the world to replace
		 @param newWorld the world to replace with. Cannot be already loaded.
		 */
		void AddReplaceWorld(Ref<World> oldWorld, Ref<World> newWorld);
		
		/**
		 Set the window titlebar text
		 @param title the text for the titlebar
		 @note Do not call this every frame. To update periodically with data such as frame rates, use a scheduled system.
		 */
        void SetWindowTitle(const char* title);
		
		std::optional<Ref<World>> GetWorldByName(const std::string& name);

		auto GetCurrentRenderWorld()  {
			return renderWorld;
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
        
        AudioSnapshot a1, a2, a3, *acurrent = &a1, *ainactive = &a2, *arender = &a3;
        SpinLock audiomtx1, audiomtx2;
	protected:
		virtual AppConfig OnConfigure(int argc, char** argv) { return AppConfig{}; }
		
		//plays the audio generated in worlds
		std::unique_ptr<AudioPlayer> player;
		
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
