#include "IInputListener.hpp"
#include "InputManager.hpp"

using namespace RavEngine;

IInputListener::~IInputListener()
{
	isTearingDown = true;
	for (auto& p : senders) {
		auto ref = p.first;
		//remove this from sender
		if (!ref.isNull()) {
			ref.get()->UnbindAllFor(this);
		}
	}
}

void IInputListener::OnRegister(const WeakRef<InputManager> &i){
	senders[i]++;
}

void IInputListener::OnUnregister(const WeakRef<InputManager> &i) {
	if (isTearingDown || i.isNull()){
		return;
	}
	if (senders.contains(i)) {
		senders[i]--;
	}
	if (senders[i] == 0) {
		senders.erase(i);
	}
}
