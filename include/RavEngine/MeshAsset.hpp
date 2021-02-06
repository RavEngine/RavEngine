#pragma once

#include "DataStructures.hpp"
#include "mathtypes.hpp"
#include <bgfx/bgfx.h>

namespace RavEngine{

class MeshAsset {
public:
	
	/**
	 Create a MeshAsset
	 @param path the path to the asset in the embedded filesystem
	 */
	MeshAsset(const std::string& path, const decimalType scale = 1.0);
	
	const bgfx::VertexBufferHandle getVertexBuffer(){
		return vertexBuffer;
	}
	const bgfx::IndexBufferHandle getIndexBuffer(){
		return indexBuffer;
	}
	
	virtual ~MeshAsset(){
		bgfx::destroy(vertexBuffer);
		bgfx::destroy(indexBuffer);
	}
	
protected:
	bgfx::VertexBufferHandle vertexBuffer;
	bgfx::IndexBufferHandle indexBuffer;
};

}
