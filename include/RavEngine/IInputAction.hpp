#pragma once
#include "WeakRef.hpp"

namespace RavEngine {
	class InputSystem;

	struct IInputListener {
		void OnRegister(const WeakRef<InputSystem>& i) {
			senders[i]++;
		}
		void OnUnregister(const WeakRef<InputSystem>& i) {
			if (senders.find(i) != senders.end()) {
				senders[i]--;
			}
			if (senders[i] == 0) {
				senders.erase(i);
			}
		}

		~IInputListener();
	private:
		std::unordered_map<WeakRef<InputSystem>, int> senders;
	};
}