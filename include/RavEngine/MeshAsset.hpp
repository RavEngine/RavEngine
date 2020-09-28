#pragma once

#include "SharedObject.hpp"
#include <bgfx/bgfx.h>
#include <unordered_map>
#include <mutex>

#define MESHLIBRARY(A) CMRC_DECLARE(A);


namespace RavEngine{

class MeshAsset : public SharedObject{
public:
	
	/**
	 Create a MeshAsset
	 @param path the path to the asset in the embedded filesystem
	 */
	MeshAsset(const std::string& path);
	
	const bgfx::VertexBufferHandle& getVertexBuffer(){
		return vertexBuffer;
	}
	const bgfx::IndexBufferHandle& getIndexBuffer(){
		return indexBuffer;
	}
	
	virtual ~MeshAsset(){
		bgfx::destroy(vertexBuffer);
		bgfx::destroy(indexBuffer);
	}
	
	class Manager{
		friend class MeshAsset;
	public:
		/**
		 Get an already-loaded MeshAsset by path
		 @param the path to the MeshAsset
		 @return a reference to the MeshAsset, or a null ref if there is none.
		 @note The null state is expensive - use IsMeshAssetLoaded to check first, do not use this call to check.
		 */
		static Ref<MeshAsset> GetLoadedMeshAsset(const std::string& path);
		
		/**
		 Determine if a MeshAsset has been loaded
		 @param the path to the MeshAsset
		 @return true if a mesh asset is loaded at that path, false otherwise.
		 */
		static bool IsMeshAssetLoaded(const std::string& path);
		
		/**
		 Remove a MeshAsset from the loaded list
		 @param the path to remove
		 @note This will not deallocate the MeshAsset right away. The MeshAsset will be unloaded when its final reference is deallocated.
		 */
		static void RemoveMeshAsset(const std::string& path);
		
	protected:
		static std::mutex mtx;
		static std::unordered_map<std::string,Ref<MeshAsset>> meshes;
		
		/**
		 Add a MeshAsset to the map
		 */
		static void RegisterMeshAsset(const std::string& path, Ref<MeshAsset> m);
	private:
		Manager() = delete;	//prevent instantiation
	};
	
protected:
	bgfx::VertexBufferHandle vertexBuffer;
	bgfx::IndexBufferHandle indexBuffer;
};

}
