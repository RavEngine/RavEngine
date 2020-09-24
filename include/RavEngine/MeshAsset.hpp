#pragma once

#include "SharedObject.hpp"
#include <bgfx/bgfx.h>

namespace RavEngine{

class MeshAsset : public SharedObject{
public:
	MeshAsset();
	
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
	
protected:
	bgfx::VertexBufferHandle vertexBuffer;
	bgfx::IndexBufferHandle indexBuffer;
};

}
