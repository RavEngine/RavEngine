
#include "RavEngine_App.hpp"

#include <GameplayStatics.hpp>

#include <fstream>

using namespace std;

int RavEngine_App::run(int argc, char** argv) {

	//invoke startup hook
    setupwindow();
	OnStartup(argc, argv);

	//in loop tick related things
    bool bQuit = false;

    while (!bQuit)
    {
     
    }


	//invoke shutdown
	return OnShutdown();
}


void RavEngine_App::setupwindow(){

}
