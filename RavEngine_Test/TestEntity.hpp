//
//  TestEntity.hpp
//  RavEngine_Test
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once

#include "RavEngine/Entity.hpp"
#include "RavEngine/PhysicsBodyComponent.hpp"
#include "RavEngine/WeakRef.hpp"
#include "RavEngine/PhysicsMaterial.hpp"
#include "RavEngine/Material.hpp"
#include "RavEngine/IPhysicsActor.hpp"

class TestEntity : public RavEngine::Entity, public RavEngine::IPhysicsActor{
protected:
    static Ref<RavEngine::PhysicsMaterial> sharedMat;
    static Ref<RavEngine::Material> sharedMatInstance;
public:
    TestEntity();
    void Tick(float scale) override;
    void OnColliderEnter(const WeakRef<RavEngine::PhysicsBodyComponent>&) override;
};