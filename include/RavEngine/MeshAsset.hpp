#pragma once

#include "DataStructures.hpp"
#include "mathtypes.hpp"
#include <bgfx/bgfx.h>
#include "Ref.hpp"
#include <vector>
#include "Common3D.hpp"
#include "WeakRef.hpp"
#include "SpinLock.hpp"

struct aiMesh;

namespace RavEngine{

class MeshAsset {
public:
	typedef VertexNormalUV vertex_t;

	struct MeshPart{
		std::vector<uint32_t> indices;
		std::vector<vertex_t> vertices;
	};

	/**
	Convert an assimp mesh to a MeshPart
	@param mesh the assimp mesh to convert
	@param scaleMat the matrix to apply to each vertex of the mesh
	@return converted MeshPart
	*/
	static MeshPart AIMesh2MeshPart(const aiMesh* mesh, const matrix4& scaleMat);
protected:
	bgfx::VertexBufferHandle vertexBuffer = BGFX_INVALID_HANDLE;
	bgfx::IndexBufferHandle indexBuffer = BGFX_INVALID_HANDLE;

	size_t totalVerts = 0, totalIndices = 0;
	
	inline void Destroy(){
		if (bgfx::isValid(vertexBuffer)){
			bgfx::destroy(vertexBuffer);
		}
		if (bgfx::isValid(indexBuffer)){
			bgfx::destroy(indexBuffer);
		}
		vertexBuffer = BGFX_INVALID_HANDLE;
		indexBuffer = BGFX_INVALID_HANDLE;
	}
	
	/**
	 Initialize from multiple meshs consisting of a single vertex and index list
	 @param mp the meshes to initialize from
	 */
	void InitializeFromMeshPartFragments(const std::vector<MeshPart>& mp, bool keepCopyInSystemMemory);
	
	/**
	 Initialize from a complete mesh consisting of a single vertex and index list
	 @param mp the mesh to initialize from
	 */
	void InitializeFromRawMesh(const MeshPart& mp, bool keepCopyInSystemMemory);
	
	// optionally stores a copy of the mesh in system memory
	MeshPart systemRAMcopy;
	
public:
	
    struct Manager{
    private:
        static phmap::flat_hash_map<std::string,WeakRef<MeshAsset>> meshes;
        static SpinLock mtx;
    public:
        /**
         Load a mesh from cache. If the mesh is not cached in memory, it will be loaded from disk.
         @param str the name of the mesh
         @param extras additional arguments to pass to meshasset constructor
         @note Using this with the specific model loading constructor is not supported and will produce unexpected results.
         */
        template<typename ... A>
        static Ref<MeshAsset> GetMesh(const std::string& str, A ... extras){
            //TODO: optimize
            mtx.lock();
            if (meshes.contains(str)){
                if (auto ptr = meshes.at(str).lock()){
                    mtx.unlock();
                    return ptr;
                }
            }
            Ref<MeshAsset> m = std::make_shared<MeshAsset>(str,extras...);
            meshes.insert(std::make_pair(str,m));
            mtx.unlock();
            return m;
        }
        
        /**
         Load a mesh from cache. If the mesh is not cached in memory, it will be loaded from disk.
         @param str the name of the mesh
         */
        static Ref<MeshAsset> GetMesh(const std::string& str){
            // TODO: don't duplicate default args here?
            return GetMesh(str,1,false);
        }
    };
    
	/**
	 Default constructor that creates an invalid MeshAsset. Useful in conjunction with Exchange.
	 */
	MeshAsset(){}
	
	/**
	 Create a MeshAsset
	 @param path the path to the asset in the embedded filesystem
	 @param scale the scale factor when loading
	 @param keepCopyInSystemMemory maintain a copy of the mesh data in system RAM, for use in features that need it like mesh colliders
	 */
	MeshAsset(const std::string& path, const decimalType scale = 1.0, bool keepCopyInSystemMemory = false);
	
	/**
	 Create a MeshAsset from a specific mesh inside the scene file. To load all meshes into one mesh, use the other constructor.
	 @param path the path to the asset in the embedded filesystem
	 @param modelName the name of the mesh inside the scene file to load
	 @param scale the scale factor when loading
	 @param keepCopyInSystemMemory maintain a copy of the mesh data in system RAM, for use in features that need it like mesh colliders
	 */
	MeshAsset(const std::string& path, const std::string& modelName, const decimalType scale = 1.0, bool keepCopyInSystemMemory = false);

	
	/**
	 Create a MeshAsset from multiple vertex and index lists
	 @param rawMeshData the index and triangle data
	 @param keepCopyInSystemMemory maintain a copy of the mesh data in system RAM, for use in features that need it like mesh colliders
	 */
	MeshAsset(const std::vector<MeshPart>& rawMeshData, bool keepCopyInSystemMemory = false){
		InitializeFromMeshPartFragments(rawMeshData, keepCopyInSystemMemory);
	}
	
	/**
	 Create a MeshAsset from a single vertex and index list
	 @param mesh the index and triangle data
	 */
	MeshAsset(const MeshPart& mesh, bool keepCopyInSystemMemory = false){
		InitializeFromRawMesh(mesh, keepCopyInSystemMemory);
	}
	
	/**
	 Move a MeshAsset's data into this MeshAsset.
	 @param other the other MeshAsset, which will become invalid after this call and should not be used.
	 */
	inline void Exchange(Ref<MeshAsset> other, bool destroyCurrent = true){
		if (destroyCurrent){
			Destroy();
		}
		vertexBuffer = other->vertexBuffer;
		indexBuffer = other->indexBuffer;
		totalVerts = other->totalVerts;
		totalIndices = other->totalIndices;
		
		other->vertexBuffer = BGFX_INVALID_HANDLE;
		other->vertexBuffer = BGFX_INVALID_HANDLE;
	}
	
	inline const bgfx::VertexBufferHandle getVertexBuffer(){
		return vertexBuffer;
	}
	inline const bgfx::IndexBufferHandle getIndexBuffer(){
		return indexBuffer;
	}
	
	virtual ~MeshAsset(){
		Destroy();
	}

	inline const decltype(totalVerts) GetNumVerts() const {
		return totalVerts;
	}

	inline const decltype(totalIndices) GetNumIndices() const {
		return totalIndices;
	}
	
	inline decltype(systemRAMcopy)& GetSystemCopy(){
		return systemRAMcopy;
	}
	
	/**
	 In case the system memory copy is no longer needed, destroy it.
	 This is not undoable.
	 */
	inline void DeallocSystemCopy(){
		systemRAMcopy = MeshPart{};
	}
};

}
