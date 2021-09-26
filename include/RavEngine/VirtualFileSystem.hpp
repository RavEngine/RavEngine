#pragma once
#include <string>
#include <functional>
#include "DataStructures.hpp"
#include "Utilities.hpp"
#include "Debug.hpp"

struct PHYSFS_File;

namespace RavEngine{
class VirtualFilesystem {
private:
    struct ptrsize{
        PHYSFS_File* ptr = nullptr;
        size_t size = 0;
    };
    
    const ptrsize GetSizeAndPtr(const char* path);
    
    void close(PHYSFS_File* file);
    
    size_t ReadInto(PHYSFS_File*, uint8_t* data, size_t size);
public:
	VirtualFilesystem(const std::string&);

	/**
	 Get the file data as a string
	 @param path the resources path to the asset
	 @return the file data
	 */
    template<typename vec = RavEngine::Vector<uint8_t>>
	const vec FileContentsAt(const char* path)
    {
        vec fileData;
        FileContentsAt(path,fileData);
        return fileData;
    }
	
	/**
	 Get the file data in a vector
	 @param path the resources path to the asset
	 @param datavec the vector to write the data into
	 */
    template<typename vec = RavEngine::Vector<uint8_t>>
    void FileContentsAt(const char* path, vec& datavec){
        auto fullpath = StrFormat("{}/{}",rootname,path);
        
        if(!Exists(path)){
            Debug::Fatal("cannot open {}{}",rootname,path);
        }
        
        auto ptrsize = GetSizeAndPtr(fullpath.c_str());
        auto ptr = ptrsize.ptr;
        auto size = ptrsize.size;
        
        datavec.resize(size);
        
        size_t length_read = ReadInto(ptrsize.ptr, datavec.data(), size);
        datavec.data()[size-1] = '\0';    //add null terminator
        close(ptrsize.ptr);
    }

	/**
	 @return true if the VFS has the file at the path
	 */
	bool Exists(const char* path);

	/**
	Get all the files in a directory
	@param path the path to the folder 
	@param callback function to call on each filename
	*/
	void IterateDirectory(const char* path, std::function<void(const std::string&)> callback);
	
protected:
	std::string rootname;
};
}
