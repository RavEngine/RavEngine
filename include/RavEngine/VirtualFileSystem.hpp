#pragma once
#include "SharedObject.hpp"
#include <ttvfs.h>

namespace RavEngine{
class VirtualFilesystem : public SharedObject{
public:
	VirtualFilesystem(const std::string&);

	const std::string FileContentsAt(const std::string& path);

	bool Exists(const std::string& path);
	
	virtual ~VirtualFilesystem();
protected:
	ttvfs::Root vfs;
};
}
