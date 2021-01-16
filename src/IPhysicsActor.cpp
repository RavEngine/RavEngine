#include "IPhysicsActor.hpp"
#include "Ref.hpp"
#include "PhysicsBodyComponent.hpp"

using namespace std;
using namespace RavEngine;

void RavEngine::IPhysicsActor::OnRegisterBody(const WeakRef<PhysicsBodyComponent>& p)
{
	senders.insert(p);
}

void RavEngine::IPhysicsActor::OnUnregisterBody(const WeakRef<PhysicsBodyComponent>& p)
{
	senders.erase(p);
}

RavEngine::IPhysicsActor::~IPhysicsActor()
{
	for (auto& a : senders) {
		if (!a.expired()){
			//this will fail if not a SharedObject
			a.get()->RemoveReceiver(this);
		}
	}
}
