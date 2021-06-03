#pragma once
#include "Ref.hpp"

namespace RavEngine {

	template<typename T, bool required = true>
	class AccessRead {
		Ref<T> ptr;
	public:
		AccessRead(decltype(ptr) p) : ptr(p) {}

		inline const decltype(ptr)& get() const {
			return ptr;
		}
	};

	template<typename T, bool required = true>
	class AccessReadWrite {
		Ref<T> ptr;
	public:
		AccessReadWrite(decltype(ptr) p) : ptr(p) {}

		inline decltype(ptr)& get() {
			return ptr;
		}
	};

	template<typename T>
	class AccessDelete {};
	
	template<typename T>
	class AccessCreate {};
}