
#include "RavEngine_App.hpp"

#include <GameplayStatics.hpp>



using namespace std;

int RavEngine_App::run(int argc, char** argv) {

	//invoke startup hook
    setupwindow();
	OnStartup(argc, argv);
        
    

    return OnShutdown();
}


void RavEngine_App::setupwindow(){

}
