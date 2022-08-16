#pragma once
#include "Entity.hpp"
#include "CTTI.hpp"

namespace RavEngine {
class ComponentWithOwner : public AutoCTTI{
protected:
    entity_t owner;
public:
    ComponentWithOwner(const decltype(owner) o) : owner(o){}
    Entity GetOwner() const {
        return Entity(owner);
    }
};
}

class Disableable{
    bool Enabled = true;
public:
	decltype(Enabled) GetEnabled() const {return Enabled; }
	void SetEnabled(decltype(Enabled) in ) {Enabled = in; }
};
