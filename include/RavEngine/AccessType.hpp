#pragma once
#include "Ref.hpp"

namespace RavEngine {

	/*template<typename T, bool required = true>
	class AccessRead {
		Ref<const T> ptr;
	public:
		AccessRead(decltype(ptr) p) : ptr(p) {}

		AccessRead(Ref<const Component> c) : ptr(std::static_pointer_cast<T>(c)) {}

		inline decltype(ptr)& get() {
			return ptr;
		}
	};

	template<typename T, bool required = true>
	class AccessReadWrite {
		Ref<T> ptr;
	public:
		AccessReadWrite(decltype(ptr) p) : ptr(p) {}

		AccessReadWrite(Ref<Component> c) : ptr(std::static_pointer_cast<T>(c)) {}

		inline decltype(ptr)& get() {
			return ptr;
		}
	};

	template<typename T>
	class AccessDelete {};
	
	template<typename T>
	class AccessCreate {};*/
}