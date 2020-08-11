
#include "RavEngine_App.hpp"

#include <GameplayStatics.hpp>

using namespace std;
using namespace filament;

int RavEngine_App::run(int argc, char** argv) {

	//invoke startup hook
    setupwindow();
	OnStartup(argc, argv);
    
    Config config;
    config.title = "hellotriangle";
    
    auto setup = [&](Engine* engine, View* view, Scene* scene) {
        
    };

    
    int exitcode = 0;
    auto cleanup = [&](Engine* engine, View*, Scene*) {
        exitcode = OnShutdown();
    };
    
     FilamentApp::get().animate([&](Engine* engine, View* view, double now) {
         //tick here
     });
    
    FilamentApp::get().run(config, setup, cleanup);

    return exitcode;
}


void RavEngine_App::setupwindow(){

}
