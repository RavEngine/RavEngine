#pragma once
#include <bgfx/bgfx.h>
#include <vector>
#include "Common3D.hpp"

namespace RavEngine{

	class Buffer{
	public:
		virtual bool IsValid() = 0;
	};

	class VertexBuffer : public Buffer{
	public:
		/**
		 Creates an invalid empty vertex buffer
		 */
		VertexBuffer(){};
		/**
		 Construct a VertexBuffer object
		 @param vertices a vector containing all of the Vertices to include
		 */
		VertexBuffer(const std::vector<Vertex>& vertices){
			bgfx::VertexLayout pcvDecl;
			
			//vertex format
			pcvDecl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
			
			auto vbm = bgfx::copy(&vertices[0], vertices.size() * sizeof(Vertex));
			handle = bgfx::createVertexBuffer(vbm, pcvDecl);
		}
		operator bgfx::VertexBufferHandle() const{
			return handle;
		}
		/**
		 @return true if the buffer is valid and safe to use
		 */
		bool IsValid() override{
			return bgfx::isValid(handle);
		}
		
	protected:
		bgfx::VertexBufferHandle handle;
	};

	class IndexBuffer : public Buffer{
	public:
		/**
		 Creates an invalid empty index buffer
		 */
		IndexBuffer(){}
		/**
		 Create an IndexBuffer object
		 @param indices the vector containing the indices to use in Triangle List form
		 */
		IndexBuffer(const std::vector<uint16_t>&  indices){
			auto ibm = bgfx::copy(&indices[0], indices.size() * sizeof(uint16_t));
			handle = bgfx::createIndexBuffer(ibm);

		}
		operator bgfx::IndexBufferHandle() const{
			return handle;
		}
		/**
		 @return true if the buffer is valid and safe to use
		 */
		bool IsValid() override{
			return bgfx::isValid(handle);
		}
	protected:
		bgfx::IndexBufferHandle handle;
	};
}
