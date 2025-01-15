#pragma once

namespace RavEngine {
	/**
	For getting global access to input, instead of binding inputs directly
	*/
	struct IInputListener {
		/**
		Invoked when an input is pressed
		*/
		virtual void AnyActionDown(const int charcode){}
		/**
		Invoked when an input is released
		*/
		virtual void AnyActionUp(const int charcode){}
	};
}
