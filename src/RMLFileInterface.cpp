#include "RMLFileInterface.hpp"
#include "App.hpp"
#include "Debug.hpp"

using namespace RavEngine;

// Opens a file.
Rml::FileHandle VFSInterface::Open(const Rml::String& path){
	auto ptr = new VFShandle{App::Resources->FileContentsAt(path.c_str())};
	return reinterpret_cast<Rml::FileHandle>(ptr);
}

// Closes a previously opened file.
void VFSInterface::Close(Rml::FileHandle file){
	VFShandle* handle = reinterpret_cast<VFShandle*>(file);
	delete handle;
}

// Reads data from a previously opened file.
size_t VFSInterface::Read(void* buffer, size_t size, Rml::FileHandle file){
	//copy from the string, beginning at offset and ending at offset+size, into buffer
	VFShandle* handle = reinterpret_cast<VFShandle*>(file);
	size_t begin = handle->offset;
	size_t max_end = begin + size;
	char* strptr = &handle->filedata[0];
	
	auto nbytes = std::min(max_end,handle->filedata.size());
	
	memcpy(buffer, strptr + begin, nbytes);
	handle->offset += nbytes;
	
	return nbytes;
}

// Seeks to a point in a previously opened file.
bool VFSInterface::Seek(Rml::FileHandle file, long offset, int origin){
	VFShandle* handle = reinterpret_cast<VFShandle*>(file);
	switch(origin){
		case SEEK_CUR:		//from current position
			handle->offset += offset;
			break;
		case SEEK_END:		//from end of file
			handle->offset = handle->filedata.size() - offset;
			break;
		case SEEK_SET:	//beginning of file
			handle->offset = offset;
			break;
		default:
			Debug::Fatal("Invalid origin flag");
	}
}

// Returns the current position of the file pointer.
size_t VFSInterface::Tell(Rml::FileHandle file){
	VFShandle* handle = reinterpret_cast<VFShandle*>(file);
	return handle->offset;
}
