//
//  TestEntity.hpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once

#include "Entity.hpp"
#include "PhysicsBodyComponent.hpp"
#include "WeakRef.hpp"
#include "PhysicsMaterial.hpp"
#include "Material.hpp"

class TestEntity : public RavEngine::Entity{
protected:
    static Ref<RavEngine::PhysicsMaterial> sharedMat;
    static Ref<RavEngine::Material> sharedMatInstance;
public:
    TestEntity();
    void Tick(float scale) override;
    void OnColliderEnter(const WeakRef<RavEngine::PhysicsBodyComponent>&) override;
};