
#include "Material.hpp"
#include <sstream>
#include <fstream>

using namespace std;

/**
Load a shader at a path
@param filename the path to the compiled bgfx shader
@return a handle to a BGFX shader
*/
bgfx::ShaderHandle loadShader(const string& filename)
{
    const char* shaderPath = "???";

    switch (bgfx::getRendererType()) {
    case bgfx::RendererType::Noop:
    case bgfx::RendererType::Direct3D9:  shaderPath = "shaderc/dx9/";   break;
    case bgfx::RendererType::Direct3D11:
    case bgfx::RendererType::Direct3D12: shaderPath = "shaderc/dx11/";  break;
    case bgfx::RendererType::Gnm:        shaderPath = "shaderc/pssl/";  break;
    case bgfx::RendererType::Metal:      shaderPath = "shaderc/metal/"; break;
    case bgfx::RendererType::OpenGL:     shaderPath = "shaderc/glsl/";  break;
    case bgfx::RendererType::OpenGLES:   shaderPath = "shaderc/essl/";  break;
    case bgfx::RendererType::Vulkan:     shaderPath = "shaderc/spirv/"; break;
    }

//    auto path = pwd / filesystem::path(shaderPath) / filesystem::path(filename);
    //auto pwd = filesystem::current_path();
    auto path = /*pwd */ shaderPath + filename;

    ifstream fin(path, ios::binary);
    ostringstream data;
    data << fin.rdbuf();
    auto str = data.str();

    const bgfx::Memory* mem = bgfx::copy(str.c_str(), str.size());

    return bgfx::createShader(mem);
}

/**
Create the default material. It draws vertex colors.
*/
Material::Material() {
    /*bgfx::ShaderHandle vsh = loadShader("vs_cubes.bin");
    bgfx::ShaderHandle fsh = loadShader("fs_cubes.bin");
    program = bgfx::createProgram(vsh, fsh, true);*/
}
