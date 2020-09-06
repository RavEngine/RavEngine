//
//  SkateSystem.h
//  MacFramework
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "RavEngine/System.hpp"
#include "RavEngine/Transform.hpp"

/**
 This system simply moves an entity along one axis
 */
class Skate : public RavEngine::System{
public:
    virtual ~Skate(){}
    
    virtual void Tick(float fpsScale, Ref<RavEngine::Entity> e) const override{
        auto newPos = e->transform()->GetLocalPosition();
        newPos += 5 * fpsScale;
        e->transform()->SetLocalPosition(newPos);
    }

    std::list<std::type_index> QueryTypes() const override {
        return { typeid(RavEngine::Transform) };
    }
};