//
//  WorldTest.hpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once

#include "World.hpp"
#include "PlayerActor.hpp"

class TestWorld : public World{
public:
    void posttick(float fpsScale) override;
    TestWorld();
    Ref<PlayerActor> player = new PlayerActor();

    virtual ~TestWorld() {

    }
protected:
};
