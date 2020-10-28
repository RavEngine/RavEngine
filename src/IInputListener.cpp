#include "IInputListener.hpp"
#include "InputManager.hpp"

using namespace RavEngine;

IInputListener::~IInputListener()
{
	for (auto& p : senders) {
		auto ref = p.first;
		//remove this from sender
		if (!ref.isNull()) {
			Ref<InputManager>(ref)->UnbindAllFor(this);
		}
	}
}

void IInputListener::OnRegister(const WeakRef<InputManager> &i){
	senders[i]++;
}

void IInputListener::OnUnregister(const WeakRef<InputManager> &i) {
	if (senders.find(i) != senders.end()) {
		senders[i]--;
	}
	if (senders[i] == 0) {
		senders.erase(i);
	}
}
