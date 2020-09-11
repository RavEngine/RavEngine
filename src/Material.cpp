
#include "Material.hpp"
#include <sstream>
#include <fstream>
#include <RenderEngine.hpp>
#include <LLGL/LLGL.h>
#include "SDLSurface.hpp"
#include "mathtypes.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <filesystem>
#include <zipper/zipper.h>
#include <zipper/unzipper.h>
#include "Common3D.hpp"

using namespace std;
using namespace RavEngine;
using namespace zipper;
using namespace std::filesystem;

MaterialManager::MaterialStore MaterialManager::materials;
matrix4 MaterialManager::projectionMatrix;
matrix4 MaterialManager::viewMatrix;
mutex MaterialManager::mtx;

// mapping names to types
const unordered_map<string, ShaderStage> stagemap{
	{"VS",ShaderStage::Vertex},
	{"FS",ShaderStage::Fragment},
	{"G",ShaderStage::Geometry},
	{"TE",ShaderStage::TessEval},
	{"TC",ShaderStage::TessControl},
	{"C",ShaderStage::Compute},
	
};

void RavEngine::Material::SetTransformMatrix(const matrix4& mat)
{
    transformMatrix = mat;
}

void Material::Draw(LLGL::CommandBuffer* const commands,LLGL::Buffer* vertexBuffer, LLGL::Buffer* indexBuffer)
{
    //calculate wvp matrix
    auto view = MaterialManager::GetCurrentViewMatrix();
    auto projection = MaterialManager::GetCurrentProjectionMatrix();
    auto wvp = projection * view * transformMatrix; //transformMatrix * view * projection;

    const decimalType* pSource = (const decimalType*)glm::value_ptr(wvp);
    for (int i = 0; i < 16; ++i) {
        settings.wvpMatrix[i] = pSource[i];
    }

    // Set graphics pipeline
    commands->SetPipelineState(*pipeline);

    commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

    if (resourceHeap) {
        commands->SetResourceHeap(*resourceHeap);
    }

    // Set vertex buffer
    commands->SetVertexBuffer(*vertexBuffer);
    commands->SetIndexBuffer(*indexBuffer);
    commands->DrawIndexed(indexBuffer->GetDesc().size / sizeof(uint32_t), 0);
}

/**
Create a material given a shader. Also registers it in the material manager
@param shader the path to the shader
*/
Material::Material(const std::string& name, const std::string& vertShaderSrc, const std::string& fragShaderSrc) : name(name) {
	//check if material is already loaded
	if (MaterialManager::HasMaterialByName(name)) {
		throw new runtime_error("Material with name " + name + "is already allocated! Use GetMaterialByName to get it.");
	}
	
	struct shader_src{
		ShaderStage type;
		std::string source;
		shader_src(ShaderStage t, const std::string& s) : type(t), source(s){}
	};
	vector<shader_src> uncompressed_shaders;
	{
		//create shader program
		Unzipper uz(name + ".bin");
		auto names = uz.entries();
		for (const auto& n : names){
			//uncompress shader
			std::vector<unsigned char> unzipped_entry;
			uz.extractEntryToMemory(n.name, unzipped_entry);
			string name_only = path(n.name).replace_extension("");
			uncompressed_shaders.emplace_back(stagemap.at(name_only),string(unzipped_entry.begin(),unzipped_entry.end()));
		}
	}

    //maps the memory appropriately so uniforms can be set
    uint32_t constantBufferIndex = 0;       //needs to be 1 on Metal
    LLGL::PipelineLayoutDescriptor pldesc;
    pldesc.bindings = {
        LLGL::BindingDescriptor{
            "Settings", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer,
            LLGL::StageFlags::VertexStage,   //this makes the uniform availabe to the vertex stage. Change to make available to other shaders
             constantBufferIndex
        }
    };

    LLGL::PipelineLayout* pipelinelayout = RenderEngine::GetRenderSystem()->CreatePipelineLayout(pldesc);    

    //create the CPU-mirror to update uniforms
    LLGL::BufferDescriptor constantBufferDesc;
    constantBufferDesc.size = sizeof(settings);
    constantBufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::ReadWrite;
    constantBufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
    //constantBufferDesc.miscFlags = LLGL::MiscFlags::DynamicUsage;
    constantBuffer = RenderEngine::GetRenderSystem()->CreateBuffer(constantBufferDesc, &settings);

    //create a resource heap with the constant buffer
    LLGL::ResourceHeapDescriptor heapdesc;
    heapdesc.pipelineLayout = pipelinelayout;
    heapdesc.resourceViews = { constantBuffer };

    resourceHeap = RenderEngine::GetRenderSystem()->CreateResourceHeap(heapdesc);

    const auto& languages = RenderEngine::GetRenderSystem()->GetRenderingCaps().shadingLanguages;

    LLGL::ShaderDescriptor vertShaderDesc, fragShaderDesc;

    auto path = std::filesystem::current_path();

    if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::GLSL) != languages.end())
    {
#if 0
        if (contextDesc.profileOpenGL.contextProfile == LLGL::OpenGLContextProfile::CompatibilityProfile)
        {
            vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.120.vert" };
            fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.120.frag" };
        }
        else
#endif
        {
#ifdef __APPLE__
            vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.140core.vert" };
            fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.140core.frag" };
#else
            vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.vert" };
            fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.frag" };
#endif
        }
    }
    else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::SPIRV) != languages.end())
    {
        //vertShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex, "Example.450core.vert.spv");
        //fragShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.450core.frag.spv");
    }
    else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end())
    {
        vertShaderDesc = { LLGL::ShaderType::Vertex,   "../../src/Example.hlsl", "VS", "vs_4_0" };
        fragShaderDesc = { LLGL::ShaderType::Fragment, "../../src/Example.hlsl", "PS", "ps_4_0" };
    }
    else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::Metal) != languages.end())
    {
		vertShaderDesc.source = uncompressed_shaders[0].source.c_str();
		vertShaderDesc.sourceSize = uncompressed_shaders[0].source.size();
		vertShaderDesc.sourceType = LLGL::ShaderSourceType::CodeString;
		vertShaderDesc.type = LLGL::ShaderType::Vertex;
		vertShaderDesc.profile = "1.1";
		vertShaderDesc.entryPoint = "main0";

		fragShaderDesc.source = uncompressed_shaders[1].source.c_str();
		fragShaderDesc.sourceSize = uncompressed_shaders[1].source.size();
		fragShaderDesc.sourceType = LLGL::ShaderSourceType::CodeString;
		fragShaderDesc.type = LLGL::ShaderType::Fragment;
		fragShaderDesc.profile = "1.1";
		fragShaderDesc.entryPoint = "main0";
		
//		vertShaderDesc = { LLGL::ShaderType::Vertex,  "defaultMaterial/VS.metal", "main0", "1.1" };
//		fragShaderDesc = { LLGL::ShaderType::Fragment, "defaultMaterial/FS.metal", "main0", "1.1" };
    }

    LLGL::VertexFormat vertexFormat;

    // Append 2D float vector for position attribute
    vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });

    // Append 3D unsigned byte vector for color
    vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGB32Float });

    // Update stride in case our vertex structure is not 4-byte aligned
    vertexFormat.SetStride(sizeof(Vertex));

    // Specify vertex attributes for vertex shader
    vertShaderDesc.vertex.inputAttribs = vertexFormat.attributes;

    // Create shaders
    LLGL::Shader* vertShader = RenderEngine::GetRenderSystem()->CreateShader(vertShaderDesc);
    LLGL::Shader* fragShader = RenderEngine::GetRenderSystem()->CreateShader(fragShaderDesc);

    for (auto shader : { vertShader, fragShader })
    {
        if (shader != nullptr)
        {
            std::string log = shader->GetReport();
            if (!log.empty())
                std::cerr << log << std::endl;
        }
    }

    // Create shader program which is used as composite
    LLGL::ShaderProgramDescriptor shaderProgramDesc;

    shaderProgramDesc.vertexShader = vertShader;
    shaderProgramDesc.fragmentShader = fragShader;

    LLGL::ShaderProgram* shaderProgram = RenderEngine::GetRenderSystem()->CreateShaderProgram(shaderProgramDesc);

    // Link shader program and check for errors
    if (shaderProgram->HasErrors()) {
        cerr << shaderProgram->GetReport() << endl;
        throw std::runtime_error(shaderProgram->GetReport());
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;

    pipelineDesc.shaderProgram = shaderProgram;
    pipelineDesc.renderPass = RenderEngine::GetSurface()->GetContext()->GetRenderPass();
#ifdef ENABLE_MULTISAMPLING
    pipelineDesc.rasterizer.multiSampleEnabled = (contextDesc.samples > 1);
#endif
    pipelineDesc.pipelineLayout = pipelinelayout;
    pipelineDesc.rasterizer.cullMode = LLGL::CullMode::Back;
    pipelineDesc.rasterizer.frontCCW = true;
    pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList;
    pipelineDesc.depth.testEnabled = true;  //enable depth
    pipelineDesc.depth.writeEnabled = true;
    // Create graphics PSO
    pipeline = RenderEngine::GetRenderSystem()->CreatePipelineState(pipelineDesc);

	//register material
	MaterialManager::RegisterMaterial(this);
}

Material::Material() : Material("defaultMaterial","","") {}

Material::~Material() {
}

/**
@returns if a material with the given name has been loaded.
@param the name of the material to find

*/
bool RavEngine::MaterialManager::HasMaterialByName(const std::string& name)
{
	mtx.lock();
	bool has = materials.find(name) != materials.end();
	mtx.unlock();
	return has;
}

/**
Mark a material for deletion by name. The material will remain allocated until its last reference is released.
@param name the name of the material to mark for deletion
*/
void RavEngine::MaterialManager::UnregisterMaterialByName(const std::string& name)
{
	mtx.lock();
	materials.erase(name);
	mtx.unlock();
}

void RavEngine::MaterialManager::RegisterMaterial(Ref<Material> mat)
{
	mtx.lock();
	materials.insert(std::make_pair(mat->GetName(), mat));
	mtx.unlock();
}
