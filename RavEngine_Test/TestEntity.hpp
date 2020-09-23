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
#include "RavEngine/ScriptComponent.hpp"
#include "RavEngine/BuiltinMaterials.hpp"

class TestEntityController : public RavEngine::ScriptComponent, public RavEngine::IPhysicsActor {
public:
    void Tick(float scale) override;

    void OnColliderEnter(const WeakRef<RavEngine::PhysicsBodyComponent>&) override;
};

class TestEntity : public RavEngine::Entity, public RavEngine::IPhysicsActor{
protected:
    static Ref<RavEngine::PhysicsMaterial> sharedMat;
    static Ref<RavEngine::DefaultMaterialInstance> sharedMatInst;
public:
    TestEntity();
};