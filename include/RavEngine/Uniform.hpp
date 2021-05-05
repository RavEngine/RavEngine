#pragma once
#include <bgfx/bgfx.h>
#include <string>

namespace RavEngine{

/**
 Base class for uniforms
 */
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
	
	Uniform(){};
	
	/**
	 Set the values in a uniform
	 @param value the pointer to the beginning of the array of values
	 @param numValues the number of values in the array
	 */
	template<typename T>
	inline void SetValues(T* value, int numValues){
		bgfx::setUniform(handle, value, numValues);
	}
	
	//TODO: static factory to create vector of uniforms from Material
	
	/**
	 * @return true if this uniform is valid and therefore safe to use
	 */
	inline bool IsValid() const{
		return bgfx::isValid(handle);
	}
	
	virtual ~Uniform(){
		if (bgfx::isValid(handle)){
			bgfx::destroy(handle);
		}
	}
	
	/**
	 Conversion operator for interal use
	 */
	inline operator bgfx::UniformHandle() const{
		return handle;
	}
	
protected:
	bgfx::UniformHandle handle = BGFX_INVALID_HANDLE;
	/**
	 Construct a uniform given a name
	 @param name the name of the uniform
	 @note Uniforms are always unique, creating multiple uniforms with the same name will not create separate instances.
	 */
	Uniform(const std::string& name, Type type, int size){
		handle = bgfx::createUniform(name.c_str(), static_cast<bgfx::UniformType::Enum>(type), size);
	}
};

/**
Sampler uniform class
 */
class SamplerUniform : public Uniform{
public:
	/**
	 Construct a sampler uniform given a name
	 @param name the name of the uniform
	 @note Uniforms are always unique, creating multiple uniforms with the same name will not create separate instances.
	 */
	SamplerUniform(const std::string& name, int size = 1) : Uniform(name,Uniform::Type::Sampler,size){}
	SamplerUniform(){}
};

/**
 Vector4 uniform class
 */
class Vector4Uniform : public Uniform{
public:
	/**
	 Construct a sampler uniform given a name
	 @param name the name of the uniform
	 @note Uniforms are always unique, creating multiple uniforms with the same name will not create separate instances.
	 */
	Vector4Uniform(const std::string& name, int size = 1) : Uniform(name,Uniform::Type::Vec4,size){}
	Vector4Uniform(){}
};

/**
 Matrix 3x3 uniform class
 */
class Mat3Uniform : public Uniform{
public:
	/**
	 Construct a sampler uniform given a name
	 @param name the name of the uniform
	 @note Uniforms are always unique, creating multiple uniforms with the same name will not create separate instances.
	 */
	Mat3Uniform(const std::string& name, int size = 1) : Uniform(name,Uniform::Type::Mat3,size){}
	Mat3Uniform(){}
};

/**
 Matrix 4x4 uniform class
 */
class Mat4Uniform : public Uniform{
public:
	/**
	 Construct a sampler uniform given a name
	 @param name the name of the uniform
	 @note Uniforms are always unique, creating multiple uniforms with the same name will not create separate instances.
	 */
	Mat4Uniform(const std::string& name, int size = 1) : Uniform(name,Uniform::Type::Mat4,size){}
	Mat4Uniform(){}
};

}
