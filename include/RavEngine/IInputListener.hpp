#pragma once
#include "WeakRef.hpp"
#include <phmap.h>

namespace RavEngine {
	class InputManager;

	struct IInputListener {
		virtual void AnyActionDown(const int charcode){}
		virtual void AnyActionUp(const int charcode){}
	};
}
