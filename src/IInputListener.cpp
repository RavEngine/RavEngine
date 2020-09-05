#include "IInputListener.hpp"
#include "InputManager.hpp"

RavEngine::IInputListener::~IInputListener()
{
	for (auto& p : senders) {
		auto ref = p.first;
		//remove this from sender
		if (!ref.isNull()) {
			Ref<InputManager>(ref)->UnbindAllFor(this);
		}
	}
}
