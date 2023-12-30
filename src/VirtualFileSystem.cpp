#include "VirtualFileSystem.hpp"
#include <physfs.h>
#include "Filesystem.hpp"
#include <span>

#ifdef __APPLE__
    #include <CoreFoundation/CFBundle.h>
#endif

// defined in client user libraries automatically by cmake
// if the user does not define these, the library will not link
extern const std::span<const char> cmrc_get_file_data(const std::string_view& path);
extern const std::string_view RVE_VFS_get_name();

using namespace RavEngine;
using namespace std;

inline const char* PHYSFS_WHY(){
	return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
}

VirtualFilesystem::VirtualFilesystem() {
	auto path = RVE_VFS_get_name();
	rootname = path;
#ifdef __APPLE__
    CFBundleRef AppBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(AppBundle);
	CFURLRef absoluteResourceURL = CFURLCopyAbsoluteURL(resourcesURL);
	CFStringRef resourcePath = CFURLCopyPath( absoluteResourceURL);

    string bundlepath = CFStringGetCStringPtr(resourcePath, kCFStringEncodingUTF8);
    streamingAssetsPath = bundlepath;
    auto rvedatapath = std::format("{}.rvedata",path);
	bundlepath = (bundlepath + rvedatapath);
    const char* cstr = bundlepath.c_str();
    
	CFRelease(absoluteResourceURL);
    CFRelease(resourcePath);
    CFRelease(resourcesURL);
#else
    auto rvedatapath = std::format("{}.rvedata",path);;
    const char* cstr = rvedatapath.c_str();
    streamingAssetsPath = Filesystem::CurrentWorkingDirectory();
#endif

    streamingAssetsPath = streamingAssetsPath / std::format("{}_Streaming",path);

	//1 means add to end, can put 0 to make it first searched
	if (PHYSFS_mount(cstr, "", 1) == 0) {
		Debug::Fatal("PHYSFS Error: {}",PHYSFS_WHY());
	}
	auto root = PHYSFS_enumerateFiles("/");
	if (*root == NULL) {
		Debug::Fatal("PHYSFS Error: {}", PHYSFS_WHY());
	}
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
	return PHYSFS_exists(std::format("{}/{}",rootname,path).c_str());
}

void RavEngine::VirtualFilesystem::IterateDirectory(const char* path, Function<void(const std::string&)> callback)
{
	string fullpath = std::format("{}/{}", rootname, path);
	auto all = PHYSFS_enumerateFiles(fullpath.c_str());
	Debug::Assert(all != nullptr, "{} not found", path);
	for (int i = 0; *(all+i) != nullptr; i++) {
		callback(std::format("{}/{}",path,*(all+i)));
	}
	PHYSFS_freeList(all);
}

const std::span<const char> RavEngine::VirtualFilesystem::GetShaderData(const std::string_view name)
{
#if __APPLE__
	return {};
#else
	auto filedata = cmrc_get_file_data(name);

	return filedata;
#endif
}
