
class RavEngine_App {
public:
	virtual ~RavEngine_App() {}

	/**
	Invoked automatically. Passes command line arguments.
	*/
	int run(int argc, char** argv);
	
protected:
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

	void setupwindow();
};
#define START_APP(APP) int main(int argc, char** argv){APP a; return a.run(argc, argv);}