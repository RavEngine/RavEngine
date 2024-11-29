#pragma once
#include <cstddef>
#include <span>
#include <fstream>

namespace RavEngine {

	enum class IStreamStatus {
		Success,
		EndOfStream,
		Unknown
	};

	struct IStreamResult {
		size_t bytesRead;
		IStreamStatus status;
	};

	struct IStream {

		virtual IStreamResult read(std::span<std::byte> outData) = 0;
		virtual void reset() = 0;

		template<typename T>
		IStreamResult readT(T* data) {
			return read({ reinterpret_cast<std::byte*>(data),sizeof(T) });
		}

		virtual void advance(size_t) = 0;

		virtual size_t size() = 0;
		virtual size_t current_pos() = 0;
	};

	
	class MemoryStream : public IStream {
		std::span<std::byte> data;
		size_t offset = 0;
	public:
		MemoryStream(const decltype(data)& data) : data(data) {};

		IStreamResult read(std::span<std::byte> outData) final;

		void reset() final;
		void advance(size_t) final;

		size_t size() final;
		size_t current_pos() final;
	};

	class FileStream : public IStream{
		std::ifstream input;
		std::streampos begin;
	public:
		FileStream(decltype(input) && input);

		IStreamResult read(std::span<std::byte> outData) final;

		void reset() final;
		void advance(size_t) final;

		size_t size() final;
		size_t current_pos() final;
	};
}