#pragma once
#include "Entity.hpp"
#include "Transform.hpp"

namespace RavEngine{
struct GameObject : public RavEngine::Entity{
    void Create(){
        EmplaceComponent<Transform>();
    }
};
}
