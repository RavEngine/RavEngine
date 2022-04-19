#include "VirtualFileSystem.hpp"
#include <physfs.h>
#include <fmt/format.h>
#include "Filesystem.hpp"

#ifdef __APPLE__
    #include <CoreFoundation/CFBundle.h>
#endif

using namespace RavEngine;
using namespace std;

inline const char* PHYSFS_WHY(){
	return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
}

VirtualFilesystem::VirtualFilesystem(const std::string& path) {
#ifdef __APPLE__
    CFBundleRef AppBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(AppBundle);
	CFURLRef absoluteResourceURL = CFURLCopyAbsoluteURL(resourcesURL);
	CFStringRef resourcePath = CFURLCopyPath( absoluteResourceURL);

    string bundlepath = CFStringGetCStringPtr(resourcePath, kCFStringEncodingUTF8);
	bundlepath = (bundlepath + path);
    const char* cstr = bundlepath.c_str();
    
	CFRelease(absoluteResourceURL);
    CFRelease(resourcePath);
    CFRelease(resourcesURL);
#else
    const char* cstr = path.c_str();
#endif

	//1 means add to end, can put 0 to make it first searched
	auto pwd = Filesystem::CurrentWorkingDirectory();
	if (PHYSFS_mount(cstr, "", 1) == 0) {
		Debug::Fatal("PHYSFS Error: {}",PHYSFS_WHY());
	}
	auto root = PHYSFS_enumerateFiles("/");
	if (*root == NULL) {
		Debug::Fatal("PHYSFS Error: {}", PHYSFS_WHY());
	}
	rootname = Filesystem::Path(path).replace_extension("").string();
	PHYSFS_freeList(root);
}

const VirtualFilesystem::ptrsize VirtualFilesystem::GetSizeAndPtr(const char *path){
    auto ptr = PHYSFS_openRead(path);
    size_t size = PHYSFS_fileLength(ptr);
    return ptrsize{ptr,size};
}

void VirtualFilesystem::close(PHYSFS_File *file){
    PHYSFS_close(file);
}

size_t VirtualFilesystem::ReadInto(PHYSFS_File* file, void* output, size_t size){
    return PHYSFS_readBytes(file,output,size);
}

bool RavEngine::VirtualFilesystem::Exists(const char* path)
{
	return PHYSFS_exists(StrFormat("{}/{}",rootname,path).c_str());
}

void RavEngine::VirtualFilesystem::IterateDirectory(const char* path, Function<void(const std::string&)> callback)
{
	string fullpath = StrFormat("{}/{}", rootname, path);
	auto all = PHYSFS_enumerateFiles(fullpath.c_str());
	Debug::Assert(all != nullptr, "{} not found", path);
	for (int i = 0; *(all+i) != nullptr; i++) {
		callback(StrFormat("{}/{}",path,*(all+i)));
	}
	PHYSFS_freeList(all);
}
