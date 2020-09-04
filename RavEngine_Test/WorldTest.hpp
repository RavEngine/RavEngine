//
//  WorldTest.hpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once

#include "World.hpp"
#include "PlayerActor.hpp"
#include "IInputAction.hpp"

class TestWorld : public RavEngine::World, public RavEngine::IInputListener{
public:
    void posttick(float fpsScale) override;
    TestWorld();
    Ref<PlayerActor> player = new PlayerActor();

    virtual ~TestWorld() {

    }
	void ResetCam();
	void SpawnEntities(float);
};
