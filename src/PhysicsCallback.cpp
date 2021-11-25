#include "PhysicsCallback.hpp"
#include "Ref.hpp"
#include "PhysicsBodyComponent.hpp"

using namespace std;
using namespace RavEngine;

RavEngine::PhysicsCallback::~PhysicsCallback()
{
	for (auto& a : senders) {
        //TODO: why is this const_cast necessary?
        const_cast<PolymorphicComponentHandle<PhysicsBodyComponent>&>(a)->RemoveReceiver(ipa_id);
	}
}
