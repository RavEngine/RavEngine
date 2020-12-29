#include "VirtualFileSystem.hpp"
#include <physfs.h>
#include "Debug.hpp"

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
	PHYSFS_mount(cstr, "", 1);
	auto root = PHYSFS_enumerateFiles("/");
	if (*root == NULL) {
		cerr << PHYSFS_WHY() << endl;
		Debug::Fatal(PHYSFS_WHY());
	}
	rootname = cstr;
}
const std::string RavEngine::VirtualFilesystem::FileContentsAt(const char* path)
{
	
	if(!Exists(path)){
		Debug::Fatal("cannot open {}{}",rootname,path);
	}
	
	auto ptr = PHYSFS_openRead(path);
	auto size = PHYSFS_fileLength(ptr)+1;
	
	char* buffer = new char[size];
	
	int length_read = PHYSFS_read(ptr,buffer,1,size);
	buffer[size-1] = '\0';	//add null terminator
	PHYSFS_close(ptr);
	
	//TODO: remove extra copy, perhaps use string_view?
	string cpy(buffer,size);
	delete[] buffer;
	
	return cpy;
}

bool RavEngine::VirtualFilesystem::Exists(const char* path)
{
	return PHYSFS_exists(path);
}


RavEngine::VirtualFilesystem::~VirtualFilesystem()
{
}
