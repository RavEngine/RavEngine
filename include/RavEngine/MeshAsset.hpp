#pragma once

#include "DataStructures.hpp"
#include "mathtypes.hpp"
#include <bgfx/bgfx.h>
#include "Ref.hpp"
#include <vector>
#include "Common3D.hpp"
#include "WeakRef.hpp"
#include "SpinLock.hpp"
#include "Manager.hpp"
#include <boost/container_hash/hash.hpp>

struct aiMesh;

namespace RavEngine{

struct MeshAssetOptions{
    bool keepInSystemRAM = false;
    bool uploadToGPU = true;
    float scale = 1.0;
    
    inline bool operator==(const MeshAssetOptions& other) const{
        return keepInSystemRAM == other.keepInSystemRAM && uploadToGPU == other.uploadToGPU && scale == other.scale;
    }
};

class MeshAsset {
public:
	typedef VertexNormalUV vertex_t;

	struct MeshPart{
		std::vector<uint32_t> indices;
		std::vector<vertex_t> vertices;
	};

    // if we do not want this meshasset having ownership of the mesh (for example in use with Exchange)
    // set this to false
    bool destroyOnDestruction = true;
    
	/**
	Convert an assimp mesh to a MeshPart
	@param mesh the assimp mesh to convert
	@param scaleMat the matrix to apply to each vertex of the mesh
	@return converted MeshPart
	*/
	static MeshPart AIMesh2MeshPart(const aiMesh* mesh, const matrix4& scaleMat);
    
    struct Bounds{
        float min[3] = {0,0,0};
        float max[3] = {0,0,0};
    };
    
protected:
	bgfx::VertexBufferHandle vertexBuffer = BGFX_INVALID_HANDLE;
	bgfx::IndexBufferHandle indexBuffer = BGFX_INVALID_HANDLE;

	size_t totalVerts = 0, totalIndices = 0;
    Bounds bounds;
   	
	inline void Destroy(){
        if (destroyOnDestruction){
            if (bgfx::isValid(vertexBuffer)){
                bgfx::destroy(vertexBuffer);
            }
            if (bgfx::isValid(indexBuffer)){
                bgfx::destroy(indexBuffer);
            }
        }
		vertexBuffer = BGFX_INVALID_HANDLE;
		indexBuffer = BGFX_INVALID_HANDLE;
	}
	
	/**
	 Initialize from multiple meshs consisting of a single vertex and index list
	 @param mp the meshes to initialize from
	 */
	void InitializeFromMeshPartFragments(const std::vector<MeshPart>& mp, const MeshAssetOptions& options = MeshAssetOptions());
	
	/**
	 Initialize from a complete mesh consisting of a single vertex and index list
	 @param mp the mesh to initialize from
	 */
	void InitializeFromRawMesh(const MeshPart& mp, const MeshAssetOptions& options = MeshAssetOptions());
	
	// optionally stores a copy of the mesh in system memory
	MeshPart systemRAMcopy;
	
public:
	
    struct Manager : public GenericWeakManager<std::string,MeshAsset>{
    public:
        
        /**
         Load a mesh from cache. If the mesh is not cached in memory, it will be loaded from disk.
         @param str the name of the mesh
         */
        static Ref<MeshAsset> GetMesh(const std::string& str){
            return GenericWeakManager<std::string,MeshAsset>::Get(str,MeshAssetOptions());
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
	MeshAsset(const std::string& path, const MeshAssetOptions& options = MeshAssetOptions());
	
	/**
	 Create a MeshAsset from a specific mesh inside the scene file. To load all meshes into one mesh, use the other constructor.
	 @param path the path to the asset in the embedded filesystem
	 @param modelName the name of the mesh inside the scene file to load
	 @param scale the scale factor when loading
	 @param keepCopyInSystemMemory maintain a copy of the mesh data in system RAM, for use in features that need it like mesh colliders
	 */
	MeshAsset(const std::string& path, const std::string& modelName, const MeshAssetOptions& options = MeshAssetOptions());

	
	/**
	 Create a MeshAsset from multiple vertex and index lists
	 @param rawMeshData the index and triangle data
	 @param keepCopyInSystemMemory maintain a copy of the mesh data in system RAM, for use in features that need it like mesh colliders
	 */
	MeshAsset(const std::vector<MeshPart>& rawMeshData, const MeshAssetOptions& options = MeshAssetOptions()){
		InitializeFromMeshPartFragments(rawMeshData, options);
	}
	
	/**
	 Create a MeshAsset from a single vertex and index list
	 @param mesh the index and triangle data
	 */
	MeshAsset(const MeshPart& mesh, const MeshAssetOptions& options = MeshAssetOptions()){
		InitializeFromRawMesh(mesh, options);
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
	
	constexpr inline const bgfx::VertexBufferHandle getVertexBuffer() const{
		return vertexBuffer;
	}
    constexpr inline const bgfx::IndexBufferHandle getIndexBuffer() const{
		return indexBuffer;
	}
    
    constexpr inline const decltype(bounds)& GetBounds() const{
        return bounds;
    }
	
	virtual ~MeshAsset(){
		Destroy();
	}

    constexpr inline const decltype(totalVerts) GetNumVerts() const {
		return totalVerts;
	}

    constexpr inline const decltype(totalIndices) GetNumIndices() const {
		return totalIndices;
	}
	
    constexpr inline decltype(systemRAMcopy)& GetSystemCopy(){
		return systemRAMcopy;
	}
	
    inline bool hasSystemRAMCopy() const{
        return systemRAMcopy.vertices.size() > 0;
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

namespace boost{
    template<>
struct hash<RavEngine::MeshAssetOptions>{
    inline size_t operator()(const RavEngine::MeshAssetOptions& opt){
            size_t seed = 0;
            boost::hash_combine(seed,opt.keepInSystemRAM);
            boost::hash_combine(seed,opt.uploadToGPU);
            boost::hash_combine(seed,opt.scale);
            return seed;
        }
    };
}
