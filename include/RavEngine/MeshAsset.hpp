#pragma once

#include "DataStructures.hpp"
#include "mathtypes.hpp"
#include <bgfx/bgfx.h>

namespace RavEngine{

class MeshAsset {
protected:
	bgfx::VertexBufferHandle vertexBuffer;
	bgfx::IndexBufferHandle indexBuffer;

	size_t totalVerts = 0, totalIndices = 0;
public:
	
	/**
	 Create a MeshAsset
	 @param path the path to the asset in the embedded filesystem
	 */
	MeshAsset(const std::string& path, const decimalType scale = 1.0);
	
	inline const bgfx::VertexBufferHandle getVertexBuffer(){
		return vertexBuffer;
	}
	inline const bgfx::IndexBufferHandle getIndexBuffer(){
		return indexBuffer;
	}
	
	virtual ~MeshAsset(){
		bgfx::destroy(vertexBuffer);
		bgfx::destroy(indexBuffer);
	}

	inline const decltype(totalVerts) GetNumVerts() const {
		return totalVerts;
	}

	inline const decltype(totalIndices) GetNumIndices() const {
		return totalIndices;
	}
};

}
