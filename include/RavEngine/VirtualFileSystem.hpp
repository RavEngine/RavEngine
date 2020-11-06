#pragma once
#include "SharedObject.hpp"

namespace RavEngine{
class VirtualFilesystem : public SharedObject{
public:
	VirtualFilesystem(const std::string&);

	const std::string FileContentsAt(const char* path);

	bool Exists(const char* path);
	
	virtual ~VirtualFilesystem();
protected:
	std::string rootname;
};
}
