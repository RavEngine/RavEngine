#include "IInputAction.hpp"
#include "InputSystem.hpp"

RavEngine::IInputListener::~IInputListener()
{
	for (auto& i : senders) {
		//remove this from sender
		Ref<InputSystem>(i)->UnbindAllFor(this);
	}
}
