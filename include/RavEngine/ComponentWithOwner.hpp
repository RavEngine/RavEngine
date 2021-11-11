#pragma once
#include "Entity.hpp"
#include "CTTI.hpp"

namespace RavEngine {
class ComponentWithOwner : public AutoCTTI{
    Entity owner;
public:
    decltype(owner) GetOwner() const {
        return owner;
    }
};
}
