#pragma once

#include "DataStructures.hpp"
#include "mathtypes.hpp"
#include "Ref.hpp"
#include "Common3D.hpp"
#include "WeakRef.hpp"
#include "SpinLock.hpp"
#include "Manager.hpp"
#include <boost/container_hash/hash.hpp>
#include "Filesystem.hpp"
#include "Debug.hpp"
#include <span>
#include <RGL/Types.hpp>

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

	enum class BitWidth : uint8_t {
		undefined,
		uint16,
		uint32
	};

    template<template<typename...> class T>
    struct MeshPartIndex {
        T<uint32_t> buffer32;
        T<uint16_t> buffer16;
        BitWidth mode;

        void reserve(size_t size) {
            switch (mode) {
            case BitWidth::uint16:
                buffer16.reserve(size);
                break;
            case BitWidth::uint32:
                buffer32.reserve(size);
                break;
            default:
                Debug::Fatal("Invalid Mode: {}",mode);
            }
        }

        void push_back(uint32_t index) {
            switch (mode) {
            case BitWidth::uint16:
                buffer16.push_back(index);
                break;
            case BitWidth::uint32:
                buffer32.push_back(index);
                break;
            default:
                Debug::Fatal("Invalid Mode: {}", mode);
            }
        }

        typename decltype(buffer32)::size_type size() const{
            switch (mode) {
            case BitWidth::uint16:
                return buffer16.size();
            case BitWidth::uint32:
                return buffer32.size();
                break;
            default:
                Debug::Fatal("Invalid Mode: {}", mode);
            }
            return 0;
        }

        const void* first_element_ptr() const {
            switch (mode) {
            case BitWidth::uint16:
                return static_cast<const void*>(& buffer16[0]);
            case BitWidth::uint32:
                return static_cast<const void*>(&buffer32[0]);
                break;
            default:
                Debug::Fatal("Invalid Mode: {}", mode);
            }
            return nullptr;
        }

        size_t size_bytes() const {
            switch (mode) {
            case BitWidth::uint16:
                    return buffer16.size() * sizeof(typename decltype(buffer16)::value_type);
            case BitWidth::uint32:
                    return buffer32.size() * sizeof(typename decltype(buffer32)::value_type);
                break;
            default:
                Debug::Fatal("Invalid Mode: {}", mode);
            }
            return 0;
        }

        uint32_t operator[](size_t index) const{
            switch (mode) {
            case BitWidth::uint16:
                return buffer16[index];
            case BitWidth::uint32:
                return buffer32[index];
                break;
            default:
                Debug::Fatal("Invalid Mode: {}", mode);
            }
            return 0;
        }
    };
    
    template<template<typename...> class T>
    struct MeshPartBase{
        MeshPartIndex<T> indices;
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
            vertices = decltype(vertices)(other.vertices.data(),other.vertices.size());
            indices.mode = other.indices.mode;
            indices.buffer32 = decltype(indices.buffer32)(other.indices.buffer32.data(),other.indices.size());
            indices.buffer16 = decltype(indices.buffer16)(other.indices.buffer16.data(),other.indices.size());
        }
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
	static MeshPart AIMesh2MeshPart(const aiMesh* mesh, const matrix4& scaleMat, BitWidth mode);
    
    struct Bounds{
        float min[3] = {0,0,0};
        float max[3] = {0,0,0};
    };

    
protected:
    RGLBufferPtr vertexBuffer, indexBuffer;

	size_t totalVerts = 0, totalIndices = 0;
    Bounds bounds;

	BitWidth indexBufferWidth;
	
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
	
    struct Manager : public GenericWeakReadThroughCache<std::string,MeshAsset>{};
    
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
    
	
	/**
	 Move a MeshAsset's data into this MeshAsset.
	 @param other the other MeshAsset, which will become invalid after this call and should not be used.
	 */
	inline void Exchange(Ref<MeshAsset> other, bool destroyCurrent = true){

		vertexBuffer = other->vertexBuffer;
		indexBuffer = other->indexBuffer;
		totalVerts = other->totalVerts;
		totalIndices = other->totalIndices;
		
        other->vertexBuffer.reset();
		other->indexBuffer.reset();
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
