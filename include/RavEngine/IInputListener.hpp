#pragma once
#include "WeakRef.hpp"
#include <unordered_map>

namespace RavEngine {
	class InputManager;

	struct IInputListener {
		void OnRegister(const WeakRef<InputManager>& i);
		void OnUnregister(const WeakRef<InputManager>& i);

		~IInputListener();
	private:
		std::unordered_map<WeakRef<InputManager>, int> senders;
	};
}
