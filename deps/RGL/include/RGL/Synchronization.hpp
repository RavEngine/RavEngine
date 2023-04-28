#pragma once
#include <variant>

namespace RGL {
	struct IFence {
		virtual void Wait() = 0;
		virtual void Reset() = 0;
		virtual void Signal() = 0;
	};
}