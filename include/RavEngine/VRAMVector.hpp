#pragma once
#include <RGL/RGL.hpp>
#include <RGL/Buffer.hpp>
#include <RGL/Device.hpp>

namespace RavEngine {

	struct VRAMVectorBase {
		void TrashOldVector(RGLBufferPtr buffer);
	};

	/**
	Implements a vector on top of RGLBuffer. The data in this buffer can also be used in shaders or other GPU actions. 
	This buffer uses SHARED memory. Be aware of this and its consequences with respect to performance.
	*/
	template<typename T, bool GPUWritable = false>
	struct VRAMVector : public VRAMVectorBase {
		RGLBufferPtr buffer = nullptr;
		using size_type = uint32_t;
		using index_type = uint32_t;

		struct iterator {
			T* data = nullptr;
			void operator++(int discard) {
				data++;
			}

			iterator operator++() {
				return iterator{ data + 1 };
			}

			iterator operator+(index_type i) {
                return iterator{.data = data + i};
			}
			
			T& operator*() {
				return *(data);
			}
		};
		struct const_iterator {
			T* data = nullptr;

			const_iterator(iterator i) : data(i.data) {}

			const T& operator*() {
				return *(data);
			}

			const_iterator operator+(index_type i) {
				return const_iterator(data + i);
			}

			void operator++(int discard) {
				data++;
			}
			const_iterator operator++() {
				return const_iterator{ data + 1 };
			}
		};

		constexpr static uint32_t initialSize = 16;

		RGL::BufferConfig settings{
			initialSize,
			{.StorageBuffer = true, .VertexBuffer = true},
			sizeof(T),
			RGL::BufferAccess::Shared,
			{.Writable = GPUWritable}
		};
		RGLDevicePtr owningDevice = nullptr;

		size_type nValues = 0;	// current capacity is stored in the Settings struct

		void InitialSetup(RGLDevicePtr owningDevice) {
			this->owningDevice = owningDevice;
			resize(initialSize);
		}
        
        VRAMVector(){
            
        }
        
        VRAMVector(const VRAMVector&) = delete; // disallow copying
        
        VRAMVector(VRAMVector&& other){         // move construction
            buffer = std::move(other.buffer);
            nValues = other.nValues;
            settings = other.settings;
        }

		~VRAMVector() {
			TrashOldVector(buffer);
		}

		auto data() {
			return static_cast<T*>(buffer->GetMappedDataPtr());
		}

		auto size() const {
			return nValues;
		}

		void resize(size_type newSize) {
			auto oldbuffer = buffer;
			
			auto settingscpy = settings;
			settings.nElements = newSize;
			buffer = owningDevice->CreateBuffer(settingscpy);
			buffer->MapMemory();
			if (oldbuffer) {
				// copy over old data
                buffer->SetBufferData({oldbuffer->GetMappedDataPtr(), size() * sizeof(T)});
				TrashOldVector(oldbuffer);
			}
		}

		void grow() {
			resize(settings.nElements * 2);
		}

		void resizeIfNeeded() {
			if (nValues == settings.nElements) {
				grow();
			}
		}

		template<typename ... Args>
		auto& emplace_back(Args&& ... args) {
			resizeIfNeeded();
			auto value = new(data() + size()) T{ args... };
			nValues++;
			return *value;
		}

		void push_back(const T& value) {
			resizeIfNeeded();
			this->operator[](size()) = value;
			nValues++;
		}

		void at(index_type i) {
			if (i >= 0 && i < size()) {
				return this->operator[](i);
			}
			else {
				throw std::out_of_range("index out of bounds");
			}
		}

		auto& operator[](index_type i) {
			return *(data() + i);
		}

		const auto& operator[](index_type i) const {
			return *(data() + i);
		}

		void erase(index_type i) {
			(data() + i)->~T();
			if (i == size() - 1) {
				nValues--;
			}
		}

		void erase(iterator i) {
			erase(i.data() - data());
		}

		void erase(const_iterator i) {
			erase(i.data() - data());
		}

		void pop_back() {
			erase(size() - 1);
		}

		auto begin() {
			return iterator{data()};
		}
		auto end() {
			return iterator{data() + nValues };
		}

		auto& back() {
			index_type i = 1;
			if (size() >= 0) {
				i = size();
			}
			return this->operator[](i - 1);
		}

		auto begin() const {
			return const_iterator{ data()};
		}

		auto end() const {
			return const_iterator{ data() + nValues };
		}
	};
}
