#pragma once
#include <chrono>
#include "RenderEngine.hpp"
#include "VirtualFileSystem.hpp"
#include <functional>
#include <queue>
#include "SpinLock.hpp"

namespace RavEngine {
	typedef std::chrono::high_resolution_clock clocktype;
	typedef std::chrono::microseconds timeDiff;
	typedef std::chrono::seconds deltaSeconds;
	typedef std::chrono::time_point<clocktype> timePoint;

	class App {
	public:
		virtual ~App() {}

		/**
		Invoked automatically. Passes command line arguments.
		*/
		int run(int argc, char** argv);

		static const float evalNormal;	//normal speed is 60 hz

		static Ref<VirtualFilesystem> Resources;
		
		/**
		 Dispatch a task to be executed on the main thread.
		 @param f the block to execute
		 @note To pass parameters, do not reference! Instead, you must explicitly copy the values you want to pass:
		 @code
 int x = 5; int y = 6;
 RavEngine::App::DispatchMainThread([=]{
	std::cout << x << y << std::endl;
 })
		 @endcode
		 */
		static void DispatchMainThread(const std::function<void(void)>& f){
			queue_lock.lock();
			main_tasks.push(f);
			queue_lock.unlock();
		}
	protected:
		static SpinLock queue_lock;
		static std::queue<std::function<void(void)>> main_tasks;

		//#define LIMIT_TICK
#ifdef LIMIT_TICK
	//change to adjust the ticking speed of the engine (default ~60 fps)
		std::chrono::microseconds tickrate = std::chrono::microseconds((long)166.66);
#endif

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
		timePoint lastFrameTime = clocktype::now();
		timeDiff deltaTimeMicroseconds;
		const timeDiff maxTimeStep = std::chrono::milliseconds((long)1000);
		//Render engine
		Ref<RenderEngine> Renderer = new RenderEngine();
	};
}
#define START_APP(APP) int main(int argc, char** argv){APP a; return a.run(argc, argv);}
