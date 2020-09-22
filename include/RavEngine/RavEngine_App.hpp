#pragma once
#include <chrono>
#include "RenderEngine.hpp"

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
	protected:

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
	protected:
		//last frame time, frame delta time, framerate scale, maximum frame time
		timePoint lastFrameTime = clocktype::now();
		timeDiff deltaTimeMicroseconds;
		const timeDiff maxTimeStep = std::chrono::milliseconds((long)1000);
		//Render engine
		Ref<RenderEngine> Renderer = new RenderEngine();
	};
}
#define START_APP(APP) int main(int argc, char** argv){APP a; return a.run(argc, argv);}
