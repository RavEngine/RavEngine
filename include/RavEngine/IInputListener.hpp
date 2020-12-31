#pragma once
#include "WeakRef.hpp"
#include <phmap.h>

namespace RavEngine {
	class InputManager;

	struct IInputListener {
		void OnRegister(const WeakRef<InputManager>& i);
		void OnUnregister(const WeakRef<InputManager>& i);
		
		virtual void AnyActionDown(const int charcode){}
		virtual void AnyActionUp(const int charcode){}

		~IInputListener();
	private:
		phmap::flat_hash_map<WeakRef<InputManager>, int> senders;
	};
}
