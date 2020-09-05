#include <chrono>

namespace RavEngine {
	class App {
	public:
		virtual ~App() {}

		/**
		Invoked automatically. Passes command line arguments.
		*/
		int run(int argc, char** argv);

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

	};
}
#define START_APP(APP) int main(int argc, char** argv){APP a; return a.run(argc, argv);}