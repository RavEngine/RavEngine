#pragma once
#include <chrono>
#include "RenderEngine.hpp"
#include "VirtualFileSystem.hpp"
#include <functional>
#include <concurrentqueue.h>
#include "SpinLock.hpp"
#include <thread>
#include <taskflow/taskflow.hpp>
#include "World.hpp"
#include "DataStructures.hpp"
#include "AudioPlayer.hpp"
#include "NetworkManager.hpp"

namespace RavEngine {
	typedef std::chrono::high_resolution_clock clocktype;
	typedef std::chrono::duration<double, std::micro> timeDiff;
	typedef std::chrono::seconds deltaSeconds;
	typedef std::chrono::time_point<clocktype> timePoint;
	
	class InputManager;
	class World;

	class App {
	public:
		App(const std::string& resourcesName);
		virtual ~App();
		
		/**
		 Signal to gracefully shut down the application
		 */
		static void Quit();
        
        /**
         Set the minimum tick time. If the work for the tick completes faster than this time interval,
         the main thread will sleep for the remaining time. To unlock the tick rate (not advised), set
         this to 0.
         @param min_ms the minimum amount of time a tick should take.
         */
        template<typename T>
        static void SetMinTickTime(std::chrono::duration<double,T> min_ms){
            min_tick_time = min_ms;
        }

		/**
		Invoked automatically. Passes command line arguments.
		*/
		int run(int argc, char** argv);

		static const float evalNormal;	//normal speed is 60 hz

		static Ref<VirtualFilesystem> Resources;
		
		/**
		 @return the current time, measured in seconds since the application launched
		 */
		static double currentTime(){
			return time;
		};
		
		//number of cores on device
		const int numcpus = std::thread::hardware_concurrency();
		
		//global thread pool, threads = logical processors on CPU
		static tf::Executor executor;
		
		//Render engine
		static Ref<RenderEngine> Renderer;
		
		//networking interface
		static NetworkManager networkManager;
		
		/**
		 Dispatch a task to be executed on the main thread.
		 @param f the block to execute
		 @note To pass parameters, do not reference! Instead, you must explicitly copy the values you want to pass:
		 @code
 int x = 5; int y = 6;
 RavEngine::App::DispatchMainThread([=]{
	std::cout << x << y << std::endl;
 });
		 @endcode
		 */
		static inline void DispatchMainThread(const std::function<void(void)>& f){
			main_tasks.enqueue(f);
		}

		/**
		@return the current application tick rate
		*/
		static float CurrentTPS();
		
		static Ref<InputManager> inputManager;

		/**
		Set the current world to tick automatically
		@param newWorld the new world
		*/
		static void SetRenderedWorld(Ref<World> newWorld) {
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
		
		/**
		 Add a world to be ticked
		 @param world the world to tick
		 */
		static void AddWorld(Ref<World> world){
			loadedWorlds.insert(world);
			if (!renderWorld){
				SetRenderedWorld(world);
			}
		}
		/**
		Remove a world from the tick list
		@param world the world to tick
		*/
		static void RemoveWorld(Ref<World> world){
			loadedWorlds.erase(world);
			if (renderWorld == world){
				renderWorld->OnDeactivate();
				renderWorld.reset();	//this will cause nothing to render, so set a different world as rendered
			}
		}
		
		/**
		 Replace a loaded world with a different world, transferring render state if necessary
		 @param oldWorld the world to replace
		 @param newWorld the world to replace with. Cannot be already loaded.
		 */
		static void AddReplaceWorld(Ref<World> oldWorld, Ref<World> newWorld){
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
		static void SetWindowTitle(const char* title);

	private:
		static Ref<World> renderWorld;
	
		static ConcurrentQueue<std::function<void(void)>> main_tasks;

        //change to adjust the ticking speed of the engine (default 90hz)
        static std::chrono::duration<double,std::micro> min_tick_time;
		
		static locked_hashset<Ref<World>,SpinLock> loadedWorlds;
	protected:
		
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
		timeDiff deltaTimeMicroseconds;
		const timeDiff maxTimeStep = std::chrono::milliseconds((long)1000);
		
		static double time;
	};
}
#define START_APP(APP) int main(int argc, char** argv){APP a; return a.run(argc, argv);}
