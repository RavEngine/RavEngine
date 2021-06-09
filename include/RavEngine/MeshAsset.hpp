#pragma once

#include "DataStructures.hpp"
#include "mathtypes.hpp"
#include <bgfx/bgfx.h>
#include "Ref.hpp"

namespace RavEngine{

class MeshAsset {
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
	
public:
	
	/**
	 Default constructor that creates an invalid MeshAsset. Useful in conjunction with Exchange.
	 */
	MeshAsset(){}
	
	/**
	 Create a MeshAsset
	 @param path the path to the asset in the embedded filesystem
	 */
	MeshAsset(const std::string& path, const decimalType scale = 1.0);
	
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
};

}
