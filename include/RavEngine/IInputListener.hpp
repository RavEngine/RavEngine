#pragma once
#include "WeakRef.hpp"
#include <phmap.h>

namespace RavEngine {
	class InputManager;

	struct IInputListener {
		void OnRegister(const WeakRef<InputManager>& i);
		void OnUnregister(const WeakRef<InputManager>& i);

		~IInputListener();
	private:
		phmap::parallel_flat_hash_map<WeakRef<InputManager>, int> senders;
	};
}
