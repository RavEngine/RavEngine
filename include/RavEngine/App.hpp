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

namespace RavEngine {
	typedef std::chrono::high_resolution_clock clocktype;
	typedef std::chrono::microseconds timeDiff;
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
		* Get the current application tick rate
		*/
		static float CurrentTPS();
		
		static Ref<InputManager> inputManager;

		/**
		* Set the current world to tick automatically
		* @param newWorld the new world
		*/
		static void SetWorld(Ref<World> newWorld) {
			if (currentWorld) {
				currentWorld->OnDeactivate();
			}
			currentWorld = newWorld;
			currentWorld->OnActivate();
		}

	private:
		static Ref<World> currentWorld;
	
		static ConcurrentQueue<std::function<void(void)>> main_tasks;

        //change to adjust the ticking speed of the engine (default 90hz)
        static std::chrono::duration<double,std::milli> min_tick_time;
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
