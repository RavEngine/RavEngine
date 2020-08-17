 
#include <RavEngine_App.hpp>
#include <WeakRef.hpp>
#include <SharedObject.hpp>
#include <cassert>
#include <GameplayStatics.hpp>
#include "WorldTest.hpp"

class TestApp : public RavEngine_App{
	
	void OnStartup(int argc, char** argv) override{
		{
			WeakRef<RavEngine::SharedObject> w;
			{
				Ref<RavEngine::SharedObject> re(new RavEngine::SharedObject());
				w = re;
				assert(w.get() == re.get());
			}
			assert(w.get() == nullptr);
		}

		//setup video settings
		RavEngine::GameplayStatics::VideoSettings.vsync = true;
		RavEngine::GameplayStatics::VideoSettings.width = 800;
		RavEngine::GameplayStatics::VideoSettings.height = 480;

		//create a world
		RavEngine::GameplayStatics::currentWorld = new TestWorld();
	}

	int OnShutdown() override {


		return 0;
	}
};
