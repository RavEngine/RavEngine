#include "VirtualFileSystem.hpp"
#include <ttvfs_zip.h>
#include <sstream>

using namespace RavEngine;
using namespace std;

VirtualFilesystem::VirtualFilesystem(const std::string& path) {
	//configure
	vfs.AddLoader(new ttvfs::DiskLoader);
	vfs.AddArchiveLoader(new ttvfs::VFSZipArchiveLoader);
	
	//mount the archive
	const char* cstr = path.c_str();
	vfs.AddArchive(cstr);
}
const std::string RavEngine::VirtualFilesystem::FileContentsAt(const std::string& path)
{
	ttvfs::File* vf = vfs.GetFile(path.c_str());

	//try to locate and open
	if (!vf || !vf->open("r")) {
		//TODO: optimize - this is inefficient
		ostringstream buffer;
		for (int i = 0; i < vf->size(); ++i) {
			char* c;
			vf->read(c, 1);
			buffer << c;
		}
		return buffer.str();
	}
	else {
		throw runtime_error("Cannot open " + path);
	}
}

bool RavEngine::VirtualFilesystem::Exists(const std::string& path)
{
	return vfs.GetFile(path.c_str()) != nullptr;
}

RavEngine::VirtualFilesystem::~VirtualFilesystem()
{
}
