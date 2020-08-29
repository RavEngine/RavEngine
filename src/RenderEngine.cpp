//
//  RenderEngine.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#include "RenderEngine.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "GameplayStatics.hpp"
#include "CameraComponent.hpp"
#include "World.hpp"
#include "SDLSurface.hpp"
#include <memory>
#include "LLGL/RenderSystem.h"
#include "LLGL/LLGL.h"
#include "Gauss/Gauss.h"

#include <SDL_syswm.h>
#include <SDL.h>
#include <SDL_video.h>

#ifdef __APPLE_
#include <Cocoa/Cocoa.h>
#endif

using namespace std;
using namespace RavEngine;

std::shared_ptr<SDLSurface> RenderEngine::surface;

/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine(const WeakRef<World>& w) : world(w) {
	//call Init()
	Init();
}


RavEngine::RenderEngine::~RenderEngine()
{
}

/**
 Make the rendering system aware of an object
 @param e the entity to load
 */
void RenderEngine::Spawn(Ref<Entity> e){
	
}

/**
 Remove an entity from the system. This does NOT destroy the entity from the world.
 @param e the entity to remove
 */
void RenderEngine::Destroy(Ref<Entity> e){
	
}

/**
 Render one frame using the current state of every object in the world
 */
void RenderEngine::Draw(){
	//get the active camera
	auto components = Ref<World>(world.get())->Components();
	auto allcams = components.GetAllComponentsOfType<CameraComponent>();

	//set the view transform - all entities drawn will use this matrix
	for (auto& cam : allcams) {
		auto owning = Ref<CameraComponent>(cam);
		if (owning->isActive()) {
			//TODO: set projection
			auto size = surface->GetDrawableArea();
			owning->SetTargetSize(size.width, size.height);
			break;
		}
	}

    //draw each entity (TODO: multithread, skip statics)
	auto worldOwning = Ref<World>(world);
	auto entitylist = worldOwning->getEntities();
	for (auto& entity : entitylist) {
		entity->transform()->Apply();

    }

	//draw
}

/**
@return the name of the current rendering API
*/
const string RenderEngine::currentBackend(){
	
	return "Unknown";
}

SDL_Window* const RavEngine::RenderEngine::GetWindow()
{
	return surface->getWindowPtr();
}

/**
Update the viewport to the correct size of the container window
*/
void RenderEngine::resize() {
	//fix the window size
	auto size = surface->GetDrawableArea();

	//TOOD: update renderer size

#ifdef __APPLE__
	//resizeMetalLayer(getNativeWindow(window));
#endif
}

/**
Initialize static singletons. Invoked automatically if needed.
*/
void RenderEngine::Init()
{
	// don't initialize again if already initialized
	if (surface != nullptr)
	{
		return;
	}

#ifdef __APPLE__
	//need to make a metal layer on Mac
	//nativeWindow = setUpMetalLayer(nativeWindow);
#else
#endif

	//create instance of surface
	surface = std::make_shared<RavEngine::SDLSurface>(LLGL::Extent2D{ 800, 480 }, "RavEngine");

#ifdef _WIN32
	const string backend = "Direct3D11";
#elif defined __APPLE__
	const string backend = "Metal";
#endif

    // Load render system module
    std::unique_ptr<LLGL::RenderSystem> renderer = LLGL::RenderSystem::Load(backend);

    // Create render context
    LLGL::RenderContextDescriptor contextDesc;
    {
        contextDesc.videoMode.resolution = surface->GetContentSize();
        contextDesc.vsync.enabled = true;
#ifdef ENABLE_MULTISAMPLING
        contextDesc.samples = 8; // check if LLGL adapts sample count that is too high
#endif
    }
    surface->SetContext(renderer->CreateRenderContext(contextDesc,surface));

    // Print renderer information
    const auto& info = renderer->GetRendererInfo();

    std::cout << "Renderer:         " << info.rendererName << std::endl;
    std::cout << "Device:           " << info.deviceName << std::endl;
    std::cout << "Vendor:           " << info.vendorName << std::endl;
    std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

    // Vertex data structure
    struct Vertex
    {
        float      position[3];
        LLGL::ColorRGBf   color;
    };

    // Vertex data (3 vertices for our triangle)

    Vertex vertices[] =
    {
        {{ -1, -1, -1 }, {255,0,0}}, {{ -1,  1, -1 },{0,255,0}}, {{  1,  1, -1 },{0,0,255}}, {{  1, -1, -1 },{255,0,0}},
        {{ -1, -1,  1 },{0,255,0}}, {{ -1,  1,  1 },{0,0,255}}, {{  1,  1,  1 },{255,0,0}}, {{  1, -1,  1 },{0,255,0}},
    };

    uint32_t indices[] = {
        0, 1, 2, 0, 2, 3, // front
        3, 2, 6, 3, 6, 7, // right
        4, 5, 1, 4, 1, 0, // left
        1, 5, 6, 1, 6, 2, // top
        4, 0, 3, 4, 3, 7, // bottom
        7, 6, 5, 7, 5, 4, // back
    };

    cout << "vertex buffer size: " << sizeof(vertices) << ", index buffer size: " << sizeof(indices) << endl;

    struct Settings {
        Gs::Matrix4f wvpMatrix; //todo: 16 byte pack alignment for constant buffers
    } settings;

    //calculate the model view projection matrix and set it in the uniform
    Gs::Matrix4f worldMatrix;
    worldMatrix.LoadIdentity();
    Gs::Translate(worldMatrix,Gs::Vector3(0,0,5));
    Gs::RotateFree(worldMatrix, Gs::Vector3f(0.4, 1, 0), 1.0f);
    settings.wvpMatrix = Gs::ProjectionMatrix4f::Perspective(1.6666,0.1,100,Gs::Deg2Rad(45.0),0).ToMatrix4() * worldMatrix;

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

    LLGL::PipelineLayout* pipelinelayout = renderer->CreatePipelineLayout(pldesc);

    //create the CPU-mirror to update uniforms
    LLGL::BufferDescriptor constantBufferDesc;
    constantBufferDesc.size = sizeof(settings);
    constantBufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::ReadWrite;
    constantBufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
    //constantBufferDesc.miscFlags = LLGL::MiscFlags::DynamicUsage;
    LLGL::Buffer* constantBuffer = renderer->CreateBuffer(constantBufferDesc,&settings);

    //create a resource heap with the constant buffer
    LLGL::ResourceHeapDescriptor heapdesc;
    heapdesc.pipelineLayout = pipelinelayout;
    heapdesc.resourceViews = { constantBuffer };

    LLGL::ResourceHeap* resourceHeap = renderer->CreateResourceHeap(heapdesc);

    // Vertex format
    LLGL::VertexFormat vertexFormat;

    // Append 2D float vector for position attribute
    vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });

    // Append 3D unsigned byte vector for color
    vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGB32Float });

    // Update stride in case our vertex structure is not 4-byte aligned
    vertexFormat.SetStride(sizeof(Vertex));

    // Create vertex buffer
    LLGL::BufferDescriptor vertexBufferDesc;
    {
        vertexBufferDesc.size = sizeof(vertices);                 // Size (in bytes) of the vertex buffer
        vertexBufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;    // Enables the buffer to be bound to a vertex buffer slot
        vertexBufferDesc.vertexAttribs = vertexFormat.attributes;          // Vertex format layout
    }
    LLGL::Buffer* vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

    LLGL::BufferDescriptor indexBufferDesc;
    indexBufferDesc.size = sizeof(indices);
    indexBufferDesc.bindFlags = LLGL::BindFlags::IndexBuffer;
    indexBufferDesc.format = LLGL::Format::R32UInt;
    LLGL::Buffer* indexBuffer = renderer->CreateBuffer(indexBufferDesc,indices); 

    // Create shaders
    LLGL::Shader* vertShader = nullptr;
    LLGL::Shader* fragShader = nullptr;

    const auto& languages = renderer->GetRenderingCaps().shadingLanguages;

    LLGL::ShaderDescriptor vertShaderDesc, fragShaderDesc;

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
        vertShaderDesc = { LLGL::ShaderType::Vertex,   "../src/Example.hlsl", "VS", "vs_4_0" };
        fragShaderDesc = { LLGL::ShaderType::Fragment, "../src/Example.hlsl", "PS", "ps_4_0" };
    }
    else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::Metal) != languages.end())
    {
        vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.metal", "VS", "1.1" };
        fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.metal", "PS", "1.1" };
    }

    // Specify vertex attributes for vertex shader
    vertShaderDesc.vertex.inputAttribs = vertexFormat.attributes;

    vertShader = renderer->CreateShader(vertShaderDesc);
    fragShader = renderer->CreateShader(fragShaderDesc);

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
    {
        shaderProgramDesc.vertexShader = vertShader;
        shaderProgramDesc.fragmentShader = fragShader;
    }
    LLGL::ShaderProgram* shaderProgram = renderer->CreateShaderProgram(shaderProgramDesc);

    // Link shader program and check for errors
    if (shaderProgram->HasErrors()) {
        cerr << shaderProgram->GetReport() << endl;
        throw std::runtime_error("");
    }

    // Create graphics pipeline
    LLGL::PipelineState* pipeline = nullptr;
    std::unique_ptr<LLGL::Blob> pipelineCache;

#ifdef ENABLE_CACHED_PSO
    // Try to read PSO cache from file
    const std::string cacheFilename = "GraphicsPSO." + rendererModule + ".cache";
    if ((pipelineCache = LLGL::Blob::CreateFromFile(cacheFilename)) != nullptr)
    {
        // Create graphics PSO from cache
        pipeline = renderer->CreatePipelineState(*pipelineCache);
        std::cout << "Pipeline cache restored: " << pipelineCache->GetSize() << " bytes" << std::endl;
    }
    else
#endif
    {
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram = shaderProgram;
            pipelineDesc.renderPass = surface->GetContext()->GetRenderPass();
#ifdef ENABLE_MULTISAMPLING
            pipelineDesc.rasterizer.multiSampleEnabled = (contextDesc.samples > 1);
#endif
            pipelineDesc.pipelineLayout = pipelinelayout;
            pipelineDesc.rasterizer.cullMode = LLGL::CullMode::Back;
            pipelineDesc.rasterizer.frontCCW = false;
            pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList;
        }
        // Create graphics PSO
        pipeline = renderer->CreatePipelineState(pipelineDesc);
    }

    // Get command queue to record and submit command buffers
    LLGL::CommandQueue* queue = renderer->GetCommandQueue();

    // Create command buffer to submit subsequent graphics commands to the GPU
    LLGL::CommandBuffer* commands = renderer->CreateCommandBuffer();

    
    // Begin recording commands
    commands->Begin();
    {
        // Set graphics pipeline
        commands->SetPipelineState(*pipeline);

        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

        // Set viewport and scissor rectangle
        commands->SetViewport(surface->GetContext()->GetResolution());

        if (resourceHeap) {
            commands->SetResourceHeap(*resourceHeap);
        }

        LLGL::ShaderReflection refl;
        shaderProgram->Reflect(refl);
        //commands->SetUniform(1, &settings.wvpMatrix, sizeof(settings.wvpMatrix));

        // Set vertex buffer
        commands->SetVertexBuffer(*vertexBuffer);
        commands->SetIndexBuffer(*indexBuffer);

        // Set the render context as the initial render target
        commands->BeginRenderPass(*surface->GetContext());
        {
            // Clear color buffer
            commands->Clear(LLGL::ClearFlags::Color);

            // Draw triangle with 3 vertices
            commands->DrawIndexed(36, 0);
        }
        commands->EndRenderPass();
    }
    commands->End();
    queue->Submit(*commands);

    // Present the result on the screen
    surface->GetContext()->Present();
    
}