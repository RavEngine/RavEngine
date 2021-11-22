#include "IPhysicsActor.hpp"
#include "Ref.hpp"
#include "PhysicsBodyComponent.hpp"

using namespace std;
using namespace RavEngine;

void RavEngine::IPhysicsActor::OnDestroy()
{
	for (auto& a : senders) {
        //TODO: why is this const_cast necessary?
        const_cast<PolymorphicComponentHandle<PhysicsBodyComponent>&>(a)->RemoveReceiver(ipa_id);
	}
}

Receiver::Receiver(IPhysicsActor* actor) : ipa_id(actor->ipa_id), owner(actor->owner){}
