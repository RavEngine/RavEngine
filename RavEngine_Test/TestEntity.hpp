//
//  TestEntity.hpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once

#include "Entity.hpp"
#include "PhysicsBodyComponent.h"
#include "WeakRef.hpp"

class TestEntity : public Entity{
public:
    TestEntity();
    void Tick(float scale) override;
    void OnColliderEnter(const WeakRef<PhysicsBodyComponent>&) override;
};