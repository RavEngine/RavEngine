#pragma once
#include "WeakRef.hpp"
#include <unordered_map>

namespace RavEngine {
	class InputManager;

	struct IInputListener {
		void OnRegister(const WeakRef<InputManager>& i) {
			senders[i]++;
		}
		void OnUnregister(const WeakRef<InputManager>& i) {
			if (senders.find(i) != senders.end()) {
				senders[i]--;
			}
			if (senders[i] == 0) {
				senders.erase(i);
			}
		}

		~IInputListener();
	private:
		std::unordered_map<WeakRef<InputManager>, int> senders;
	};
}
