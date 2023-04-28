
#include "Material.hpp"
#include <sstream>
#include <fstream>
#include <RenderEngine.hpp>
#include "mathtypes.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Filesystem.hpp"
#include "Common3D.hpp"
#include "App.hpp"
#include <typeindex>
#include "Debug.hpp"
#include "VirtualFileSystem.hpp"
#include <RGL/RGL.hpp>

using namespace std;
using namespace RavEngine;


// return the folder name the shaders are stored in
static inline std::string_view shader_api() {
	switch (RGL::CurrentAPI()) {
	case RGL::API::Direct3D12:
		return "dx";
	case RGL::API::Metal:
		return "mtl";
	case RGL::API::Vulkan:
		return "vk";
	default:
		Debug::Fatal("Current backend is not supported.");
		return "";
	}
}

/**
Create a material given a shader. Also registers it in the material manager
@param shader the path to the shader
*/
Material::Material(const std::string& name) : name(name) {	
	//get all shader files for this programs
	auto dir = StrFormat("shaders/{}/{}", shader_api(), name);

#if 0
    RavEngine::Vector<uint8_t> vertex_src, fragment_src;
	GetApp()->GetResources().FileContentsAt(StrFormat("{}/vertex.bin",dir).c_str(),vertex_src);
	GetApp()->GetResources().FileContentsAt(StrFormat("{}/fragment.bin",dir).c_str(),fragment_src);

	//must have a vertex and a fragment shader
# if 0
	bgfx::ShaderHandle vsh = loadShader(vertex_src);
	bgfx::ShaderHandle fsh = loadShader(fragment_src);
	program = bgfx::createProgram(vsh, fsh, true);
	if (!bgfx::isValid(program)){
		Debug::Fatal("Material is invalid.");
	}
	bgfx::setName(vsh, fmt::format("Material VS {}",name).c_str());
	bgfx::setName(fsh, fmt::format("Material FS {}",name).c_str());
#endif
#endif
}