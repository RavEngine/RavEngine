#include "IInputAction.hpp"
#include "InputSystem.hpp"

RavEngine::IInputListener::~IInputListener()
{
	for (auto& p : senders) {
		auto ref = p.first;
		//remove this from sender
		if (!ref.isNull()) {
			Ref<InputSystem>(ref)->UnbindAllFor(this);
		}
	}
}
