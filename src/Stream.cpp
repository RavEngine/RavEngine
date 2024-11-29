#include "Stream.hpp"

namespace RavEngine {

	IStreamResult MemoryStream::read(std::span<std::byte> outData)
	{
		size_t nBytesToRead = std::min(outData.size_bytes(), data.size_bytes() - offset);

		if (nBytesToRead > 0) {
			std::memcpy(outData.data(), data.data(), nBytesToRead);
		}
		else {
			return { 0, IStreamStatus::EndOfStream };
		}

		advance(nBytesToRead);
	}

	void MemoryStream::reset()
	{
		offset = 0;
	}

	void MemoryStream::advance(size_t amt)
	{
		offset += amt;
		offset = std::min(offset, data.size_bytes());	// ensure it never goes past 
	}

	size_t MemoryStream::size()
	{
		return data.size();
	}

	size_t MemoryStream::current_pos()
	{
		return offset;
	}

	FileStream::FileStream(decltype(input) && input) : input(std::move(input)){
		begin = input.tellg();
	}

	IStreamResult FileStream::read(std::span<std::byte> outData)
	{
		auto start = current_pos();
		input.read(reinterpret_cast<char*>(outData.data()), outData.size());
		IStreamStatus status = IStreamStatus::Success;
		if (input.eof()) {
			status = IStreamStatus::EndOfStream;
		}
		else if (!input.good() || input.bad() || input.fail()) {
			status = IStreamStatus::Unknown;
		}

		return { current_pos() - start,status};
	}

	void FileStream::reset()
	{
		input.seekg(0);
	}

	void FileStream::advance(size_t amt)
	{
		input.seekg(input.tellg() + std::streampos(amt));
	}

	size_t FileStream::size()
	{
		auto currentPos = input.tellg();
		input.seekg(0, std::ios::end);
		auto end = input.tellg();
		input.seekg(currentPos);
		return size_t(end - begin);
	}

	size_t FileStream::current_pos()
	{
		return size_t(input.tellg());
	}

}