#pragma once

#include "DataStructures.hpp"
#include "mathtypes.hpp"
#include "Ref.hpp"
#include "Common3D.hpp"
#include "Manager.hpp"
#include "Filesystem.hpp"
#include "Debug.hpp"
#include <span>
#if !RVE_SERVER
    #include <RGL/Types.hpp>
#endif
#include "MeshAllocation.hpp"

struct aiMesh;
struct aiScene;

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
    
    template<template<typename...> class T>
    struct MeshPartBase{
        T<uint32_t> indices;
        T<vertex_t> vertices;
    };
	struct MeshPart : public MeshPartBase<RavEngine::Vector>{};
    
    template<typename T>
    struct basic_immutable_span : public std::span<const T,std::dynamic_extent>{
        basic_immutable_span(){}
        basic_immutable_span(const T* ptr, size_t count) : std::span<const T,std::dynamic_extent>(ptr,count){}
    };
    
    struct MeshPartView : public MeshPartBase<basic_immutable_span>{
        MeshPartView(){}
        MeshPartView(const MeshPart& other){
			vertices = { other.vertices.data(),other.vertices.size() };
			indices = { other.indices.data(), other.indices.size() };
        }
    };

    // if we do not want this meshasset having ownership of the mesh
    // set this to false
    bool destroyOnDestruction = true;
    
	/**
	Convert an assimp mesh to a MeshPart
	@param mesh the assimp mesh to convert
	@param scaleMat the matrix to apply to each vertex of the mesh
	@return converted MeshPart
	*/
	static MeshPart AIMesh2MeshPart(const aiMesh* mesh, const matrix4& scaleMat);
    
	uint32_t GetNumLods() const {
		return 1;	// TODO: lods are not yet supported
	}

	MeshAsset(const MeshAsset&) = delete;
	MeshAsset(MeshAsset&&) = delete;
    
protected:
#if !RVE_SERVER
    RGLBufferPtr vertexBuffer, indexBuffer;
#endif

	size_t totalVerts = 0, totalIndices = 0;
    Bounds bounds;
    float radius = 0;

	friend class RenderEngine;
#if !RVE_SERVER
    MeshRange meshAllocation;
#endif
	
	/**
	 Initialize from multiple meshs consisting of a single vertex and index list
	 @param mp the meshes to initialize from
	 */
	void InitializeFromMeshPartFragments(const  RavEngine::Vector<MeshPart>& mp, const MeshAssetOptions& options = MeshAssetOptions());
	
	/**
	 Initialize from a complete mesh consisting of a single vertex and index list
	 @param mp the mesh to initialize from
	 */
	void InitializeFromRawMesh(const MeshPart& mp, const MeshAssetOptions& options = MeshAssetOptions());
    void InitializeFromRawMeshView(const MeshPartView& mp, const MeshAssetOptions& options = MeshAssetOptions());
	
	// optionally stores a copy of the mesh in system memory
	MeshPart systemRAMcopy;
	
	void InitAll(const aiScene* scene, const MeshAssetOptions& opt);
	void InitPart(const aiScene* scene, const std::string& name, const std::string& fileName, const MeshAssetOptions& opt);

    friend class RenderEngine;
	
public:

	auto GetRadius() const {
		return radius;
	}
#if !RVE_SERVER
	auto GetAllocation() const {
		return meshAllocation;
	}
#endif
	
    struct Manager : public GenericWeakReadThroughCache<std::string,MeshAsset>{};
	
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
	
	MeshAsset(const Filesystem::Path& pathOnDisk, const MeshAssetOptions& options = MeshAssetOptions());

	MeshAsset(const Filesystem::Path& pathOnDisk, const std::string& modelName, const MeshAssetOptions& options = MeshAssetOptions());
	/**
	 Create a MeshAsset from multiple vertex and index lists
	 @param rawMeshData the index and triangle data
	 @param keepCopyInSystemMemory maintain a copy of the mesh data in system RAM, for use in features that need it like mesh colliders
	 */
	MeshAsset(const RavEngine::Vector<MeshPart>& rawMeshData, const MeshAssetOptions& options = MeshAssetOptions()){
		InitializeFromMeshPartFragments(rawMeshData, options);
	}
	
	/**
	 Create a MeshAsset from a single vertex and index list
	 @param mesh the index and triangle data
	 */
	MeshAsset(const MeshPart& mesh, const MeshAssetOptions& options = MeshAssetOptions()){
		InitializeFromRawMesh(mesh, options);
	}
    
    MeshAsset(const MeshPartView& mesh, const MeshAssetOptions& options = MeshAssetOptions()){
        InitializeFromRawMeshView(mesh,options);
    }
    
    constexpr inline const decltype(bounds)& GetBounds() const{
        return bounds;
    }
	
    virtual ~MeshAsset();

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
