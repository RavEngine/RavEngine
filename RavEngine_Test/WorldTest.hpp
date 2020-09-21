//
//  WorldTest.hpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once

#include "RavEngine/World.hpp"
#include "PlayerActor.hpp"
#include "RavEngine/IInputListener.hpp"
#include "RavEngine/RavEngine_App.hpp"

class TestWorld : public RavEngine::World, public RavEngine::IInputListener{
public:
    void posttick(float fpsScale) override;
    TestWorld();
    Ref<PlayerActor> player = new PlayerActor();

    virtual ~TestWorld() {

    }
	void ResetCam();
	void SpawnEntities(float);


    void SampleFPS() {
        std::cout << "FPS: " << RavEngine::App::evalNormal / scale << std::endl;
    }
protected: 
       float scale = 1;
};
