#include "VirtualFileSystem.hpp"
#include <physfs.h>
#include "Filesystem.hpp"
#include <span>

#ifdef __APPLE__
    #include <CoreFoundation/CFBundle.h>
#endif

#if __ANDROID__
#include <SDL3/SDL_iostream.h>

// Android sucks and makes reading data out of the Apk difficult.
// we must emulate some behavior in order for PhysFS to work with it.

// this code is adapted from https://blog.apicici.com/posts/2023/04/using-sdl2-to-mount-an-archive-from-the-android-apk-assets-with-physicsfs/

struct sdl_rw_file_info {
    std::string name;
    SDL_IOStream* RW = nullptr;
    Sint64 cursor_pos = 0;
    Sint64 size = 0;
};

static PHYSFS_sint64 io_read(PHYSFS_Io* io, void* buffer, PHYSFS_uint64 len) {
    auto finfo = (sdl_rw_file_info*)io->opaque;
    const PHYSFS_uint64 bytes_left = (finfo->size - finfo->cursor_pos);
    PHYSFS_sint64 bytes_read;

    if (bytes_left < len) { len = bytes_left; }

    bytes_read = SDL_ReadIO(finfo->RW, buffer, len);

    if (bytes_read > 0) {
        finfo->cursor_pos += bytes_read;
    }
    else {
        SDL_ClearError();
        bytes_read = SDL_GetError()[0] != '\0' ? -1 : 0;
    }

    return bytes_read;
}

static int io_seek(PHYSFS_Io* io, PHYSFS_uint64 offset) {
    auto finfo = (sdl_rw_file_info*)io->opaque;

    if (offset >= finfo->size) {
        PHYSFS_setErrorCode(PHYSFS_ERR_PAST_EOF);
        return 0;
    }

    finfo->cursor_pos = SDL_SeekIO(finfo->RW, offset, SDL_IO_SEEK_SET);
    return finfo->cursor_pos >= 0;
}

static PHYSFS_sint64 io_tell(PHYSFS_Io* io) {
    auto finfo = (sdl_rw_file_info*)io->opaque;
    return finfo->cursor_pos;
}

static PHYSFS_sint64 io_length(PHYSFS_Io* io) {
    auto finfo = (sdl_rw_file_info*)io->opaque;
    return finfo->size;
}


static void io_destroy(PHYSFS_Io* io) {
    auto finfo = (sdl_rw_file_info*)io->opaque;
    SDL_CloseIO(finfo->RW);
    delete finfo;
    delete io;
}

static PHYSFS_Io* io_duplicate(PHYSFS_Io* io);

PHYSFS_Io* GetAndroidIo(const char* path){
    SDL_IOStream* RW = SDL_IOFromFile(path, "rb");
    if (!RW){
        return nullptr;
    }

    auto finfo = new sdl_rw_file_info;
    finfo->RW = RW;
    finfo->size = SDL_GetIOSize(RW);
    if (finfo->size < 0) {
        SDL_CloseIO(RW);
        delete finfo;
        return nullptr;
    }
    finfo->cursor_pos = 0;
    finfo->name = path;

    auto retval = new PHYSFS_Io;

    retval->version = 0;
    retval->opaque = (void*)finfo;
    retval->read = io_read;
    retval->write = nullptr; // read-only
    retval->seek = io_seek;
    retval->tell = io_tell;
    retval->length = io_length;
    retval->duplicate = io_duplicate;
    retval->flush = nullptr; // read-only
    retval->destroy = io_destroy;

    return retval;
}

static PHYSFS_Io* io_duplicate(PHYSFS_Io* io) {
    auto origfinfo = (sdl_rw_file_info*)io->opaque;
    auto retval = GetAndroidIo(origfinfo->name.c_str());

    if (!retval) { return nullptr; }

    auto finfo = (sdl_rw_file_info*)retval->opaque;
    finfo->cursor_pos = origfinfo->cursor_pos;
    // seek the SDL_RWops to the appropriate position
    if (SDL_SeekIO(finfo->RW, finfo->cursor_pos, SDL_IO_SEEK_SET) < 0) {
        retval->destroy(retval); // free allocated memory
        return nullptr;
    }

    return retval;
}

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
    auto rvedatapath = Format("{}.rvedata",path);
	bundlepath = (bundlepath + rvedatapath);
    const char* cstr = bundlepath.c_str();
    
	CFRelease(absoluteResourceURL);
    CFRelease(resourcePath);
    CFRelease(resourcesURL);
#else
    auto rvedatapath = Format("{}.rvedata",path);
    const char* cstr = rvedatapath.c_str();
    streamingAssetsPath = Filesystem::CurrentWorkingDirectory();
#endif

    streamingAssetsPath = streamingAssetsPath / Format("{}_Streaming",path);

#if __ANDROID__
    // we need to do some additional setup here. Android `assets` are not accessible to C functions
    // out of the box. we must copy them to a readable location using the Android api

    PHYSFS_Io* androidIo = GetAndroidIo(cstr);
    if(!androidIo){
        Debug::Fatal("Android error: Cannot load {}", cstr);
    }

    if (PHYSFS_mountIo(androidIo, cstr, "", 1) == 0){
        Debug::Fatal("PHYSFS Android Error: {}", PHYSFS_WHY());

        // PHYSFS_mountIo does not destroy the PHYSFS_Io if it fails
        // so we do it manually
        androidIo->destroy(androidIo);
    }

#else
    //1 means add to end, can put 0 to make it first searched
	if (PHYSFS_mount(cstr, "", 1) == 0) {
		Debug::Fatal("PHYSFS Error: {}",PHYSFS_WHY());
	}
#endif


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
	return PHYSFS_exists(Format("{}/{}",rootname,path).c_str());
}

void RavEngine::VirtualFilesystem::IterateDirectory(const char* path, Function<void(const std::string&)> callback)
{
	string fullpath = Format("{}/{}", rootname, path);
	auto all = PHYSFS_enumerateFiles(fullpath.c_str());
	Debug::Assert(all != nullptr, "{} not found", path);
	for (int i = 0; *(all+i) != nullptr; i++) {
		callback(Format("{}/{}",path,*(all+i)));
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
