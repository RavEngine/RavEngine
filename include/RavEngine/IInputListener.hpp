#pragma once

namespace RavEngine {
	struct IInputListener {
		virtual void AnyActionDown(const int charcode){}
		virtual void AnyActionUp(const int charcode){}
	};
}
