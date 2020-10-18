#pragma once
#include <bgfx/bgfx.h>

namespace RavEngine{

class Uniform{
public:
	enum Type{
		Sampler,
		End,
		Vec4,
		Mat3,
		Mat4,
		Count
	};
	
	/**
	 Construct a uniform given a name
	 @param name the name of the uniform
	 @note Uniforms are always unique, creating multiple uniforms with the same name will not create separate instances. Changing one will change all of them. Because of this behavior, these objects do not descend from SharedObject.
	 */
	Uniform(const std::string& name, Type type, int size = 1){
		handle = bgfx::createUniform(name.c_str(), static_cast<bgfx::UniformType::Enum>(type), size);
	}
	
	template<typename T>
	void SetValues(T* value, int numValues){
		bgfx::setUniform(handle, value, numValues);
	}
	
	//TODO: static factory to create vector of uniforms from Material
	
	/**
	 * @return true if this uniform is valid and therefore safe to use
	 */
	bool IsValid(){
		return bgfx::isValid(handle);
	}
	
	virtual ~Uniform(){
		//bgfx::destroy(handle);
	}
protected:
	bgfx::UniformHandle handle;
};
}
