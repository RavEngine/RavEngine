#pragma once
#include "WeakRef.hpp"

template<typename T>
class Ref {
	T* ptr = nullptr;
public:
	//default constructor
	Ref() : ptr(nullptr) {}

	/**
	@returns if this ref is a null ref
	*/
	inline bool isNull() const{
		return ptr == nullptr;
	}

	/**
	Makes this ref object a null ref. Releases any previously held pointer.
	*/
	inline void setNull() {
		if (!isNull()) {
			ptr->release();
		}
		ptr = nullptr;
	}

	//constructor
	Ref(T* const other) {
		if (other == nullptr){
			setNull();
		}
		else{
			other->retain();
			ptr = other;
		}
	}

	//construct from weak pointer
	Ref(const WeakRef<T>& other) : Ref(other.get()) {};

	//copy
	Ref(const Ref<T>& other) : Ref(other.ptr) {}

	 //destructor
	 //if this crashes, I have a bug because the object was released too early
	virtual ~Ref() {
		if (!isNull()) {
			ptr->release();
		}
	}

	//copy assignment
	inline Ref<T>& operator=(const Ref<T>& other) {
		if (&other == this) {
			return *this;
		}
		//copy pointer and increment its retain
		if (!isNull()) {
			ptr->release();
		}
		ptr = other.get();
		if (ptr != nullptr) {
			ptr->retain();
		}
		return *this;
	}

	//arrow operator
	inline T* operator->() const {
        assert(get() != nullptr); //you are performing null access!
		return get();
	}

	//dereference
	inline T* operator*() const {
        assert(get() != nullptr); //you are performing null access!
		return get();
	}

	//get bare pointer (caution!)
	inline T* get() const {
		return ptr;
	}
	
	//truthy
	inline operator bool() const{
		return ! isNull();
	}

	template<typename U>
	inline operator Ref<U>() const {
		static_assert(std::is_base_of<RavEngine::SharedObject, U>::value, "U is not a base class of SharedObject");
		static_assert(std::is_base_of<U, T>::value || std::is_base_of<T,U>::value,"This conversion is not an upcast or downcast");
		return static_cast<U*>(ptr);
	}

	//equality
	inline bool operator==(const Ref<T>& other) const {
		return other.get() == get();
	}

	//less (requires T and U to have < operator defined
	template<typename U>
	inline bool operator<(const Ref<U>& other) const {
		return *get() < *other.get();
	}
};

//hash function for ref
namespace std {
	template<typename T>
	struct hash<Ref<T>> {
		//Strong pointer hash uses the object it is pointing at
		inline std::size_t operator()(const Ref<T>& k) const
		{
			return k->Hash();
		}
	};
}
