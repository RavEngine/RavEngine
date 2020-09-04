#pragma once
#include "WeakRef.hpp"

namespace RavEngine {
	class InputSystem;

	struct IInputListener {
		void OnRegister(const WeakRef<InputSystem>& i) {
			senders.insert(i);
		}
		void OnUnregister(const WeakRef<InputSystem>& i) {
			senders.erase(i);
		}

		~IInputListener();
	private:
		std::unordered_set<WeakRef<RavEngine::InputSystem>> senders;
	};
}