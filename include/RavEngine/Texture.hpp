#pragma once
#include "SharedObject.hpp"
#include <bgfx/bgfx.h>
#include "Uniform.hpp"

namespace RavEngine{

class Texture : public SharedObject{
public:
	Texture(const std::string& filename);
	
	virtual ~Texture(){
		bgfx::destroy(texture);
	}
	
	void Bind(int id, const Uniform& uniform);
	
protected:
	bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
};

}
