#pragma once
#include <cstdint>
#include <cstddef>

namespace RGL {
	struct MutableSpan {
		void* data = nullptr;
		size_t size = 0;
	};

	class untyped_span {
		const void* ptr = nullptr;
		const size_t size_bytes = 0;
	public:
		untyped_span(decltype(ptr) ptr, decltype(size_bytes) size_bytes) : ptr(ptr), size_bytes(size_bytes) {}

		template<typename T>
		untyped_span(const T& ptr) : ptr(&ptr), size_bytes(sizeof(T)) {}

		constexpr auto data() const { return ptr; }
		constexpr auto size() const { return size_bytes; }
	};

    // a bit of an odd concept. Its purpose is to have data Moved into it, which it will then free.
    // it assumes ownership of that data.
    template<typename freer>
    struct untyped_owning_span{
        void* ptr = nullptr;
        size_t size_bytes = 0;
        constexpr static freer free_fn{};
        
        untyped_owning_span(){}
        untyped_owning_span(decltype(ptr) ptr, decltype(size_bytes) size_bytes) : ptr(ptr), size_bytes(size_bytes) {}
        
        // move constructor
        untyped_owning_span(untyped_owning_span&& other) : ptr(other.ptr), size_bytes(other.size_bytes){
            other.ptr = nullptr;
            other.size_bytes = 0;
        }
        
        // remove copy ctor
        untyped_owning_span(const untyped_owning_span&) = delete;
        untyped_owning_span& operator=(const untyped_owning_span&) = delete;
        
        ~untyped_owning_span(){
            if (ptr){
                free_fn(ptr);
                ptr = nullptr;
                size_bytes = 0;
            }
        }
        
        operator untyped_span() const{
            return untyped_span{ptr, size_bytes};
        }
    };
}
