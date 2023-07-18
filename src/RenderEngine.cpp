//
//  RenderEngine.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//	Some code adapted from: https://github.com/pezcode/Cluster/blob/master/src/Renderer/DeferredRenderer.cpp
//

#include "RenderEngine.hpp"
#include <string>
#include "CameraComponent.hpp"
#include "World.hpp"
#include <memory>
#include "Material.hpp"
#include "StaticMesh.hpp"
#include "App.hpp"
#include "GUI.hpp"
#include <RmlUi/Debugger.h>
#include "Utilities.hpp"
#include "InputManager.hpp"
#include "AnimatorComponent.hpp"
#include "Skybox.hpp"

	#if defined __linux__ && !defined(__ANDROID__)
	#define SDL_VIDEO_DRIVER_X11 1		//Without this X11 support doesn't work
	#define SDL_VIDEO_DRIVER_WAYLAND 1
#endif
#include <SDL_syswm.h>
#include <SDL.h>
#include <SDL_video.h>
#include <im3d.h>
#include <BuiltinMaterials.hpp>
#include "Common3D.hpp"
#include "DebugDrawer.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <fmt/core.h>
#include <iostream>
#include "Debug.hpp"
#include <chrono>
#include <cstdio>
#include <RGL/RGL.hpp>
#include <RGL/Device.hpp>
#include <RGL/Synchronization.hpp>
#include <RGL/Surface.hpp>
#include <RGL/Swapchain.hpp>
#include <RGL/CommandBuffer.hpp>
#include <RGL/Pipeline.hpp>
#include <RGL/RenderPass.hpp>
#include <RGL/Sampler.hpp>
#include "MeshAsset.hpp"
#include <numbers>
#include "Texture.hpp"

#ifdef __APPLE__
	#include "AppleUtilities.h"
#endif

#if _WIN32 && !_UWP
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")
#elif _UWP
#include <winrt/Windows.Graphics.Display.h>
using namespace winrt;
#endif

using namespace std;
using namespace RavEngine;

SDL_Window* RenderEngine::window = nullptr;
RenderEngine::vs RenderEngine::VideoSettings;


#ifndef NDEBUG
STATIC(RenderEngine::debuggerContext);
STATIC(RenderEngine::debuggerInput);
#endif

static constexpr uint16_t shadowMapSize = 2048;

auto genIcosphere(uint16_t subdivs){
    struct IcoSphereCreator
    {
        struct TriangleIndices
        {
            int v1;
            int v2;
            int v3;

            TriangleIndices(int v1, int v2, int v3) : v1(v1), v2(v2), v3(v3){}
        };
#pragma pack(push, 1)
		struct vec3 {
			float x, y, z;
			vec3(float x, float y, float z) : x(x), y(y), z(z) {}
		};
#pragma pack(pop)

		using vec_t = vec3;
		static_assert(sizeof(vec_t) == sizeof(float) * 3, "vec_t is not the correct size!");

        
        struct MeshGeometry3D{
            std::vector<vec_t> Positions;
            std::vector<uint32_t> TriangleIndices;
        };
        
        using Int64 = int64_t;

        MeshGeometry3D geometry;
        int index = 0;
        std::unordered_map<int64_t, int> middlePointIndexCache;

        // add vertex to mesh, fix position to be on unit sphere, return index
        int addVertex(vec_t p)
        {
            double length = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            geometry.Positions.emplace_back(p.x/length, p.y/length, p.z/length);
            return index++;
        }

        // return index of point in the middle of p1 and p2
        int getMiddlePoint(int p1, int p2)
        {
            // first check if we have it already
            bool firstIsSmaller = p1 < p2;
            Int64 smallerIndex = firstIsSmaller ? p1 : p2;
            Int64 greaterIndex = firstIsSmaller ? p2 : p1;
            Int64 key = (smallerIndex << 32) + greaterIndex;

            auto it = middlePointIndexCache.find(key);
            if (it != middlePointIndexCache.end())
            {
                return (*it).second;
            }

            // not in cache, calculate it
            auto point1 = geometry.Positions[p1];
            auto point2 = geometry.Positions[p2];
			vec_t middle{
				(point1.x + point2.x) / 2.0f,
				(point1.y + point2.y) / 2.0f,
				(point1.z + point2.z) / 2.0f };

            // add vertex makes sure point is on unit sphere
            int i = addVertex(middle);

            // store it, return index
            middlePointIndexCache.insert(std::make_pair(key, i));
            return i;
        }

        MeshGeometry3D Create(int recursionLevel)
        {

            // create 12 vertices of a icosahedron
            float t = (1.0 + std::sqrt(5.0)) / 2.0;

			addVertex({ -1,  t,  0 });
			addVertex({ 1,  t,  0 });
			addVertex({ -1, -t,  0 });
			addVertex({ 1, -t,  0 });

			addVertex({ 0, -1,  t });
			addVertex({ 0,  1,  t });
			addVertex({ 0, -1, -t });
			addVertex({ 0,  1, -t });

			addVertex({ t,  0, -1 });
			addVertex({ t,  0,  1 });
			addVertex({ -t,  0, -1 });
			addVertex({ -t,  0,  1 });


            // create 20 triangles of the icosahedron
            auto faces = std::vector<TriangleIndices>();

            // 5 faces around point 0
            faces.emplace_back(0, 11, 5);
            faces.emplace_back(0, 5, 1);
            faces.emplace_back(0, 1, 7);
            faces.emplace_back(0, 7, 10);
            faces.emplace_back(0, 10, 11);

            // 5 adjacent faces
            faces.emplace_back(1, 5, 9);
            faces.emplace_back(5, 11, 4);
            faces.emplace_back(11, 10, 2);
            faces.emplace_back(10, 7, 6);
            faces.emplace_back(7, 1, 8);

            // 5 faces around point 3
            faces.emplace_back(3, 9, 4);
            faces.emplace_back(3, 4, 2);
            faces.emplace_back(3, 2, 6);
            faces.emplace_back(3, 6, 8);
            faces.emplace_back(3, 8, 9);

            // 5 adjacent faces
            faces.emplace_back(4, 9, 5);
            faces.emplace_back(2, 4, 11);
            faces.emplace_back(6, 2, 10);
            faces.emplace_back(8, 6, 7);
            faces.emplace_back(9, 8, 1);


            // refine triangles
            for (int i = 0; i < recursionLevel; i++)
            {
                auto faces2 = std::vector<TriangleIndices>();
                for (auto& tri : faces)
                {
                    // replace triangle by 4 triangles
                    int a = getMiddlePoint(tri.v1, tri.v2);
                    int b = getMiddlePoint(tri.v2, tri.v3);
                    int c = getMiddlePoint(tri.v3, tri.v1);

                    faces2.emplace_back(tri.v1, a, c);
                    faces2.emplace_back(tri.v2, b, a);
                    faces2.emplace_back(tri.v3, c, b);
                    faces2.emplace_back(a, b, c);
                }
                faces = faces2;
            }

            // done, now add triangles to mesh
            for (auto& tri : faces)
            {
                geometry.TriangleIndices.push_back(tri.v1);
                geometry.TriangleIndices.push_back(tri.v2);
                geometry.TriangleIndices.push_back(tri.v3);
            }

            return this->geometry;
        }
    };
    IcoSphereCreator creator;
    auto data = creator.Create(subdivs);

    return data;
}

// adapted from: https://gist.github.com/andresfelipemendez/ef4f832084822bcf400600879c3b02e1
// creates a cone with tip at (0,0,0) extending along -y
auto generateCone(float radius, float height, int numberOfSides)
{
	using Vector3 = glm::vec3;

	std::vector<Vector3> sides;
	std::vector<Vector3> vertices;
	std::vector<uint16_t> triangles;

	struct Mesh {
		decltype(vertices) verts;
		decltype(triangles) indices;
	};

	constexpr auto deg2rad = (std::numbers::pi * 2) / 360.f;

	auto Rotate = [](Vector3 o, float r) {
		float ca = std::cos(r);
		float sa = std::sin(r);
		return Vector3(ca * o.x - sa * o.z, o.y,  sa * o.x + ca * o.z);
	};

	float angle = 360 / numberOfSides;
	for (int i = 0; i < numberOfSides; i++) {
		sides.push_back(Rotate(Vector3(0, -height, radius), (angle * i) * deg2rad));
	}

	vertices = sides;	// copy
	vertices.emplace_back(0,0,0);
	vertices.emplace_back(0,-height,0);

	int bottomIndex = vertices.size() - 2;
	int heightIndex = vertices.size() - 1;

	for (int i = 0; i < sides.size(); i++)
	{
		int nextVertexCicle = (i + 1) % sides.size();

		triangles.push_back(nextVertexCicle);
		triangles.push_back(i);
		triangles.push_back(bottomIndex);

		triangles.push_back(i);
		triangles.push_back(nextVertexCicle);
		triangles.push_back(heightIndex);
	}

	Mesh cube;
	cube.verts = std::move(vertices);
	cube.indices = std::move(triangles);

	return cube;
}

void DebugRenderWrapper(const Im3d::DrawList& drawList){
	GetApp()->GetRenderEngine().DebugRender(drawList);
}

/**
Initialize static singletons. Invoked automatically if needed.
*/
void RenderEngine::Init(const AppConfig& config)
{
	//setup RGL if it is not already setup
	if (window != nullptr)
	{
		return;
	}
	
	window = SDL_CreateWindow("RavEngine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, VideoSettings.width, VideoSettings.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	
	if (window == NULL){
		Debug::Fatal("Unable to create main window: {}",SDL_GetError());
	}

	RGL::API api = RGL::API::PlatformDefault;
	{
#if _UWP
		size_t n_elt;
		char* envv;
		_dupenv_s(&envv, &n_elt, "RGL_BACKEND");
#else
		auto envv = std::getenv("RGL_BACKEND");
#endif
		if (envv == nullptr) {
			goto cont;
		}
		auto backend = std::string_view(envv);

		const std::unordered_map<std::string_view, RGL::API> apis{
			{"metal", decltype(apis)::value_type::second_type::Metal},
			{"d3d12", decltype(apis)::value_type::second_type::Direct3D12},
			{"vulkan", decltype(apis)::value_type::second_type::Vulkan},
		};

		auto it = apis.find(backend);
		if (it != apis.end()) {
			api = (*it).second;
		}
		else {
			std::cerr << "No backend \"" << backend << "\", expected one of:\n";
			for (const auto& api : apis) {
				std::cout << "\t - " << RGL::APIToString(api.second) << "\n";
			}
		}
	}
cont:

	RGL::InitOptions opt{
		.api = api,
		.engineName = "RavEngine",
	};
	RGL::Init(opt);

	// for debug wireframes
	auto& data = Im3d::GetAppData();
    data.drawCallback = &DebugRenderWrapper;
	
}


/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine(const AppConfig& config) {
	Init(config);

	UpdateBufferDims();

	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(window, &wmi)) {
		throw std::runtime_error("Cannot get native window information");
}
	surface = RGL::CreateSurfaceFromPlatformHandle(
#if _UWP
		{ wmi.info.winrt.window },
#elif _WIN32
		{ &wmi.info.win.window },
#elif TARGET_OS_IPHONE
		{ wmi.info.uikit.window },
#elif __APPLE__
		{ wmi.info.cocoa.window },
#elif __linux__
		{ wmi.info.x11.display, wmi.info.x11.window },
#elif __EMSCRIPTEN__
	{nullptr, nullptr},
#else
#error Unknown platform
#endif
		true
	);

	device = RGL::IDevice::CreateSystemDefaultDevice();
    
#if __APPLE__
    if (!AppleGPUMeetsMinSpec(device)) {
        Debug::Fatal("Cannot proceed: device \"{}\" is under the minimum spec!", device->GetBrandString());
    }
#endif
    
	mainCommandQueue = device->CreateCommandQueue(RGL::QueueType::AllCommands);
	swapchain = device->CreateSwapchain(surface, mainCommandQueue, bufferdims.width, bufferdims.height);
	mainCommandBuffer = mainCommandQueue->CreateCommandBuffer();
	swapchainFence = device->CreateFence(true);
	textureSampler = device->CreateSampler({});
	shadowSampler = device->CreateSampler({
		.addressModeU = RGL::SamplerAddressMode::Border,
		.addressModeV = RGL::SamplerAddressMode::Border,
		.borderColor = {1,1,1,1},
		.compareFunction = RGL::DepthCompareFunction::Less
	});

	createGBuffers();

	shadowTexture = device->CreateTexture({
		.usage = {.Sampled = true, .DepthStencilAttachment = true },
		.aspect = {.HasDepth = true },
		.width = 2048,
		.height = 2048,
		.format = RGL::TextureFormat::D32SFloat,
		.debugName = "Shadow Texture"
	});

	auto tmpcmd = mainCommandQueue->CreateCommandBuffer();
	auto tmpfence = device->CreateFence(false);
	tmpcmd->Begin();
	tmpcmd->TransitionResource(shadowTexture.get(), RGL::ResourceLayout::Undefined, RGL::ResourceLayout::DepthReadOnlyOptimal, RGL::TransitionPosition::Top);
	tmpcmd->End();
	tmpcmd->Commit({
		.signalFence = tmpfence
		});
	tmpfence->Wait();


	// create "fixed-function" pipeline layouts
	ambientLightRenderPipelineLayout = device->CreatePipelineLayout({
		.bindings = {
			{
				.binding = 0,
				.type = RGL::BindingType::Sampler,
				.stageFlags = RGL::BindingVisibility::Fragment,
			},
			{
				.binding = 1,
				.type = RGL::BindingType::SampledImage,
				.stageFlags = RGL::BindingVisibility::Fragment,
			},
		},
		.constants = {
			{
				sizeof(AmbientLightUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
			}
		}
	});

	lightRenderPipelineLayout = device->CreatePipelineLayout({
		.bindings = {
				{
					.binding = 0,
					.type = RGL::BindingType::Sampler,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 1,
					.type = RGL::BindingType::Sampler,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 2,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 3,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 4,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 5,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Fragment,
				},
				{
					.binding = 8,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Fragment,
				}

		},
		.constants = {
			{
				sizeof(LightingUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
			}
		}
	});

	pointLightRenderPipelineLayout = device->CreatePipelineLayout({
		.bindings = {
				{
				.binding = 0,
				.type = RGL::BindingType::Sampler,
				.stageFlags = RGL::BindingVisibility::Fragment,
			},
				{
				.binding = 1,
				.type = RGL::BindingType::Sampler,
				.stageFlags = RGL::BindingVisibility::Fragment,
			},
			{
				.binding = 2,
				.type = RGL::BindingType::SampledImage,
				.stageFlags = RGL::BindingVisibility::Fragment,
			},
			{
				.binding = 3,
				.type = RGL::BindingType::SampledImage,
				.stageFlags = RGL::BindingVisibility::Fragment,
			},
			{
				.binding = 4,
				.type = RGL::BindingType::SampledImage,
				.stageFlags = RGL::BindingVisibility::Fragment,
			},
		},
		.constants = {
			{
				sizeof(PointLightUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
			}
		}
		});

	// create render passes
	deferredRenderPass = RGL::CreateRenderPass({
		   .attachments = {
			   {
				   .format = colorTexFormat,
				   .loadOp = RGL::LoadAccessOperation::Clear,
				   .storeOp = RGL::StoreAccessOperation::Store,
			   },
			   {
				   .format = normalTexFormat,
				   .loadOp = RGL::LoadAccessOperation::Clear,
				   .storeOp = RGL::StoreAccessOperation::Store,
			   },
		   },
		   .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			   .format = RGL::TextureFormat::D32SFloat,
			   .loadOp = RGL::LoadAccessOperation::Clear,
			   .storeOp = RGL::StoreAccessOperation::Store,
			   .clearColor = {1,1,1,1}
		   }
		});

	ambientLightRenderPass = RGL::CreateRenderPass({
		.attachments = {
			{
				.format = colorTexFormat,
				.loadOp = RGL::LoadAccessOperation::Clear,
				.storeOp = RGL::StoreAccessOperation::Store,
			}
		},
		.depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			.format = RGL::TextureFormat::D32SFloat,
			.loadOp = RGL::LoadAccessOperation::Load,
			.storeOp = RGL::StoreAccessOperation::Store,
		}
	});

	lightingRenderPass = RGL::CreateRenderPass({
		.attachments = {
			{
				.format = colorTexFormat,
				.loadOp = RGL::LoadAccessOperation::Load,
				.storeOp = RGL::StoreAccessOperation::Store,
			}
		},
		.depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			.format = RGL::TextureFormat::D32SFloat,
			.loadOp = RGL::LoadAccessOperation::Load,
			.storeOp = RGL::StoreAccessOperation::Store,
		}
	});

	finalRenderPass = RGL::CreateRenderPass({
		.attachments = {
			{
				.format = RGL::TextureFormat::BGRA8_Unorm,
				.loadOp = RGL::LoadAccessOperation::Clear,
				.storeOp = RGL::StoreAccessOperation::Store,
			},
		},
		 .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			.format = RGL::TextureFormat::D32SFloat,
			.loadOp = RGL::LoadAccessOperation::Load,
			.storeOp = RGL::StoreAccessOperation::Store,
		}
	});

	shadowRenderPass = RGL::CreateRenderPass({
		.attachments = {},
		.depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			.format = RGL::TextureFormat::D32SFloat,
			.loadOp = RGL::LoadAccessOperation::Clear,
			.storeOp = RGL::StoreAccessOperation::Store,
			.clearColor = {1,1,1,1}
		}
	});

	// create lighting render pipelines
	constexpr static uint32_t width = 640, height = 480;
	auto createLightingPipeline = [this](RGLShaderLibraryPtr vsh, RGLShaderLibraryPtr fsh, uint32_t vertexStride, uint32_t instanceStride, const std::vector<RGL::RenderPipelineDescriptor::VertexConfig::VertexAttributeDesc>& vertexAttributeDesc, RGLPipelineLayoutPtr layout, RGL::WindingOrder windingorder = RGL::WindingOrder::Counterclockwise) {

		RGL::RenderPipelineDescriptor::VertexConfig vertConfig{
			.vertexBindings = {
				{
					.binding = 0,
					.stride = vertexStride,
				},
				{
					.binding = 1,
					.stride = instanceStride,
					.inputRate = RGL::InputRate::Instance
				}
			},
			.attributeDescs = vertexAttributeDesc
		};

		RGL::RenderPipelineDescriptor rpd{
			.stages = {
				{
					.type = RGL::ShaderStageDesc::Type::Vertex,
					.shaderModule = vsh,
				},
				{
					.type = RGL::ShaderStageDesc::Type::Fragment,
					.shaderModule = fsh,
				}
			},
			.vertexConfig = vertConfig,
			.inputAssembly = {
				.topology = RGL::PrimitiveTopology::TriangleList,
			},
			.viewport = {
				.width = width,
				.height = height
			},
			.scissor = {
				.extent = {width, height}
			},
			.rasterizerConfig = {
				.windingOrder = windingorder,
			},
			.colorBlendConfig = {
				.attachments = {
					{
						.format = colorTexFormat,
						.destinationColorBlendFactor = RGL::BlendFactor::One,
						.destinationAlphaBlendFactor = RGL::BlendFactor::One,
						.blendEnabled = true
					},
				}
			},
			.depthStencilConfig = {
				.depthFormat = RGL::TextureFormat::D32SFloat,
				.depthTestEnabled = true,
				.depthWriteEnabled = false,
				.depthFunction = RGL::DepthCompareFunction::Greater,
			},
			.pipelineLayout = layout,
		};

		return device->CreateRenderPipeline(rpd);
	};

	// data needed for lights
	struct Vertex2D {
		glm::vec2 pos;
	};

	auto ambientLightFSH = LoadShaderByFilename("ambientlight.fsh", device);
	auto ambientLightVSH = LoadShaderByFilename("ambientlight.vsh", device);
	ambientLightRenderPipeline = createLightingPipeline(ambientLightVSH, ambientLightFSH, sizeof(Vertex2D), sizeof(glm::vec4), {
				{
					.location = 0,
					.binding = 0,
					.offset = 0,
					.format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
				},
				{
					.location = 1,
					.binding = 1,
					.offset = 0,
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				}
		}, ambientLightRenderPipelineLayout);

	auto dirLightFSH = LoadShaderByFilename("directionallight.fsh", device);
	auto dirLightVSH = LoadShaderByFilename("directionallight.vsh", device);
	dirLightRenderPipeline = createLightingPipeline(dirLightVSH, dirLightFSH, sizeof(Vertex2D), sizeof(World::DirLightUploadData), {
				{
					.location = 0,
					.binding = 0,
					.offset = 0,
					.format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
				},
				{
					.location = 1,
					.binding = 1,
					.offset = 0,
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				},
				{
					.location = 2,
					.binding = 1,
					.offset = offsetof(World::DirLightUploadData, direction),
					.format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
				}
		}, lightRenderPipelineLayout);

	auto pointLightFSH = LoadShaderByFilename("pointlight.fsh", device);
	auto pointLightVSH = LoadShaderByFilename("pointlight.vsh", device);

	pointLightRenderPipeline = createLightingPipeline(pointLightVSH, pointLightFSH, sizeof(glm::vec3), sizeof(World::PointLightUploadData), {
				{
					.location = 0,
					.binding = 0,
					.offset = 0,
					.format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
				},
				{
					.location = 1,
					.binding = 1,
					.offset = 0,
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				},
				{
					.location = 2,
					.binding = 1,
					.offset = sizeof(glm::vec4),
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				},
				{
					.location = 3,
					.binding = 1,
					.offset = sizeof(glm::vec4) * 2,
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				},
				{
					.location = 4,
					.binding = 1,
					.offset = sizeof(glm::vec4) * 3,
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				},
				{
					.location = 5,
					.binding = 1,
					.offset = sizeof(glm::vec4) * 4,
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				},
		}, pointLightRenderPipelineLayout, RGL::WindingOrder::Clockwise);

	auto spotLightFSH = LoadShaderByFilename("spotlight.fsh", device);
	auto spotLightVSH = LoadShaderByFilename("spotlight.vsh", device);

	spotLightRenderPipeline = createLightingPipeline(spotLightVSH, spotLightFSH, sizeof(glm::vec3), sizeof(World::SpotLightDataUpload), {
				{
					.location = 0,
					.binding = 0,
					.offset = 0,
					.format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
				},
				// per-instance attrs - matrix
				{
					.location = 1,
					.binding = 1,
					.offset = 0,
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				},
				{
					.location = 2,
					.binding = 1,
					.offset = sizeof(glm::vec4),
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				},
				{
					.location = 3,
					.binding = 1,
					.offset = sizeof(glm::vec4) * 2,
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				},
				{
					.location = 4,
					.binding = 1,
					.offset = sizeof(glm::vec4) * 3,
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				},
				// colorintensity
				{
					.location = 5,
					.binding = 1,
					.offset = sizeof(glm::vec4) * 4,
					.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
				},
				// penumbra angle
				{
					.location = 6,
					.binding = 1,
					.offset = sizeof(glm::vec4) * 5,
					.format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
				},
		}, pointLightRenderPipelineLayout, RGL::WindingOrder::Clockwise);

	// copy shader
	auto lightToFbFSH = LoadShaderByFilename("light_to_fb.fsh",device);
	auto lightToFbVSH = LoadShaderByFilename("light_to_fb.vsh",device);

	lightToFBPipelineLayout = device->CreatePipelineLayout({
		.bindings = {
				{
				.binding = 0,
				.type = RGL::BindingType::Sampler,
				.stageFlags = RGL::BindingVisibility::Fragment,
			},
			{
				.binding = 1,
				.type = RGL::BindingType::SampledImage,
				.stageFlags = RGL::BindingVisibility::Fragment,
			},
		},
		.constants = {
			{
				sizeof(LightingUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
			}
		}
	});

    lightToFBRenderPipeline = device->CreateRenderPipeline(RGL::RenderPipelineDescriptor{
			.stages = {
				{
					.type = RGL::ShaderStageDesc::Type::Vertex,
					.shaderModule = lightToFbVSH,
				},
				{
					.type = RGL::ShaderStageDesc::Type::Fragment,
					.shaderModule = lightToFbFSH,
				}
			},
			.vertexConfig = {
				.vertexBindings = {
					{
						.binding = 0,
						.stride = sizeof(Vertex2D),
					},
				},
				.attributeDescs = {
					{
						.location = 0,
						.binding = 0,
						.offset = 0,
						.format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
					},
				}
			},
			.inputAssembly = {
				.topology = RGL::PrimitiveTopology::TriangleList,
			},
			.viewport = {
				.width = width,
				.height = height
			},
			.scissor = {
				.extent = {width, height}
			},
			.rasterizerConfig = {
				.windingOrder = RGL::WindingOrder::Counterclockwise,
			},
			.colorBlendConfig = {
				.attachments = {
					{
						.format = RGL::TextureFormat::BGRA8_Unorm
					},
				}
            },
			.depthStencilConfig = {
				.depthFormat = RGL::TextureFormat::D32SFloat,
				.depthTestEnabled = true,
				.depthWriteEnabled = false,
				.depthFunction = RGL::DepthCompareFunction::Greater,
			},
            .pipelineLayout = lightToFBPipelineLayout,
		});

	auto guiVSH = LoadShaderByFilename("gui.vsh", device);
	auto guiFSH = LoadShaderByFilename("gui.fsh", device);

	static_assert(sizeof(Rml::Vertex::colour) == sizeof(uint32_t));

	guiRenderPipeline = device->CreateRenderPipeline(RGL::RenderPipelineDescriptor{
			.stages = {
				{
					.type = RGL::ShaderStageDesc::Type::Vertex,
					.shaderModule = guiVSH,
				},
				{
					.type = RGL::ShaderStageDesc::Type::Fragment,
					.shaderModule = guiFSH,
				}
			},
			.vertexConfig = {
				.vertexBindings = {
					{
						.binding = 0,
						.stride = sizeof(Rml::Vertex),
					},
				},
				.attributeDescs = {
					{
						.location = 0,
						.binding = 0,
						.offset = offsetof(Rml::Vertex, position),
						.format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
					},
					{
						.location = 1,
						.binding = 0,
						.offset = offsetof(Rml::Vertex, colour),
						.format = RGL::VertexAttributeFormat::R32_Uint,
					},
					{
						.location = 2,
						.binding = 0,
						.offset = offsetof(Rml::Vertex, tex_coord),
						.format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
					},
				}
			},
			.inputAssembly = {
				.topology = RGL::PrimitiveTopology::TriangleList,
			},
			.viewport = {
				.width = width,
				.height = height
			},
			.scissor = {
				.extent = {width, height}
			},
			.rasterizerConfig = {
				.windingOrder = RGL::WindingOrder::Counterclockwise,
			},
			.colorBlendConfig = {
				.attachments = {
					{
						.format = RGL::TextureFormat::BGRA8_Unorm,
						.sourceColorBlendFactor = RGL::BlendFactor::SourceAlpha,
						.destinationColorBlendFactor = RGL::BlendFactor::OneMinusSourceAlpha,
						.alphaBlendOperation = RGL::BlendOperation::Add,
						.colorWriteMask = RGL::ColorWriteMask::RGB,
						.blendEnabled = true,
					},
				}
			},
			.depthStencilConfig = {
				.depthFormat = RGL::TextureFormat::D32SFloat,
				.depthTestEnabled = false,
				.depthWriteEnabled = false,
				.depthFunction = RGL::DepthCompareFunction::Greater,
			},
			.pipelineLayout = lightToFBPipelineLayout,
		});

	//TODO: on unified memory systems, don't make a staging buffer, and mark the transientBuffer as shared
	transientBuffer = device->CreateBuffer({
		65535,
		{.StorageBuffer = true},
		sizeof(char),
		RGL::BufferAccess::Private,
		{.TransferDestination = true, .debugName = "Transient Buffer" }
	});
	transientStagingBuffer = device->CreateBuffer({
		65535,
		{.StorageBuffer = true},
		sizeof(char),
		RGL::BufferAccess::Shared,
		{.Transfersource = true, .debugName = "Transient Staging Buffer" }
	});
	transientStagingBuffer->MapMemory();

	// lighting meshes
	constexpr static Vertex2D vertices[] = {
		{{0, -10}},
		{{10, 10}},
		{{-10, 10}}
	};
	screenTriVerts = device->CreateBuffer({
		   {.VertexBuffer = true},
		   sizeof(Vertex2D),
		   vertices,
		   RGL::BufferAccess::Private
		});
	screenTriVerts->SetBufferData(vertices);

	auto pointLightMeshData = genIcosphere(2);

	pointLightVertexBuffer = device->CreateBuffer({
		uint32_t(pointLightMeshData.Positions.size()),
		{.VertexBuffer = true},
		sizeof(float) * 3,
		RGL::BufferAccess::Private
	});

	pointLightIndexBuffer = device->CreateBuffer({
		uint32_t(pointLightMeshData.TriangleIndices.size()),
		{.IndexBuffer = true},
		sizeof(uint32_t),
		RGL::BufferAccess::Private,
	});

	pointLightVertexBuffer->SetBufferData({ pointLightMeshData.Positions.data(), pointLightMeshData.Positions.size() * sizeof(pointLightMeshData.Positions[0]) });
	pointLightIndexBuffer->SetBufferData({ pointLightMeshData.TriangleIndices.data(), pointLightMeshData.TriangleIndices.size() * sizeof(pointLightMeshData.TriangleIndices[0]) });
	nPointLightIndices = pointLightMeshData.TriangleIndices.size();

	auto coneMesh = generateCone(1, 1, 16);

	spotLightVertexBuffer = device->CreateBuffer({
		uint32_t(coneMesh.verts.size()),
		{.VertexBuffer = true},
		sizeof(float) * 3,
		RGL::BufferAccess::Private
	});

	spotLightIndexBuffer = device->CreateBuffer({
		uint32_t(coneMesh.indices.size()),
		{.IndexBuffer = true},
		sizeof(uint16_t),
		RGL::BufferAccess::Private
	});

	spotLightVertexBuffer->SetBufferData({coneMesh.verts.data(), coneMesh.verts.size() * sizeof(coneMesh.verts[0])});
	spotLightIndexBuffer->SetBufferData({coneMesh.indices.data(), coneMesh.indices.size() * sizeof(coneMesh.indices[0])});
	nSpotLightIndices = coneMesh.indices.size();

	// debug render pipelines
#ifndef NDEBUG
	auto debugVSH = LoadShaderByFilename("debug.vsh", device);
	auto debugFSH = LoadShaderByFilename("debug.fsh", device);
	auto createDebugRenderPipeline = [this, debugVSH, debugFSH](RGL::PolygonOverride drawMode, RGL::PrimitiveTopology topology) {
		RGL::RenderPipelineDescriptor rpd{
			.stages = {
				{
					.type = RGL::ShaderStageDesc::Type::Vertex,
					.shaderModule = debugVSH,
				},
				{
					.type = RGL::ShaderStageDesc::Type::Fragment,
					.shaderModule = debugFSH,
				}
			},
			.vertexConfig = {
				.vertexBindings = {
					{
						.binding = 0,
						.stride = sizeof(Im3d::VertexData),
					},
				},
				.attributeDescs = {
					{
						.location = 0,
						.binding = 0,
						.offset = 0,
						.format = RGL::VertexAttributeFormat::R32G32B32A32_SignedFloat,
					},
					{
						.location = 1,
						.binding = 0,
						.offset = sizeof(glm::vec4),
						.format = RGL::VertexAttributeFormat::R32_Uint,
					},
				}
			},
			.inputAssembly = {
				.topology = topology,
			},
			.viewport = {
				.width = width,
				.height = height
			},
			.scissor = {
				.extent = {width, height}
			},
			.rasterizerConfig = {
				.polygonOverride = RGL::PolygonOverride::Line,
				.windingOrder = RGL::WindingOrder::Counterclockwise,
			},
			.colorBlendConfig = {
				.attachments = {
					{
						.format = RGL::TextureFormat::BGRA8_Unorm
					},
				}
			},
			.depthStencilConfig = {
				.depthFormat = RGL::TextureFormat::D32SFloat,
				.depthTestEnabled = false,
				.depthWriteEnabled = false,
				.depthFunction = RGL::DepthCompareFunction::Greater,
			},
			.pipelineLayout = lightToFBPipelineLayout,
		};
		return device->CreateRenderPipeline(rpd);
	};
	im3dLineRenderPipeline = createDebugRenderPipeline(RGL::PolygonOverride::Line, RGL::PrimitiveTopology::LineList);
	im3dPointRenderPipeline = createDebugRenderPipeline(RGL::PolygonOverride::Point, RGL::PrimitiveTopology::PointList);
	im3dTriangleRenderPipeline = createDebugRenderPipeline(RGL::PolygonOverride::Fill, RGL::PrimitiveTopology::TriangleList);
#endif

	// skinned mesh compute pipeline
	auto skinnedCSH = LoadShaderByFilename("skinning_cs.csh", device);
	auto skinnedPipelineLayout = device->CreatePipelineLayout({
		 .bindings = {
				{
					.binding = 0,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = true
				},
				{
					.binding = 1,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = false
				},
				{
					.binding = 2,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = false
				},
				{
					.binding = 3,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = false
				},
			},
			.constants = {{ sizeof(SkinningUBO), 0, RGL::StageVisibility::Compute}}
	});
	skinnedMeshComputePipeline = device->CreateComputePipeline(RGL::ComputePipelineDescriptor{
		.stage = {
			.type = RGL::ShaderStageDesc::Type::Compute,
			.shaderModule = skinnedCSH
		},
		.pipelineLayout = skinnedPipelineLayout
	});

	ReallocateVertexAllocationToSize(initialVerts);
	ReallocateIndexAllocationToSize(initialIndices);

	auto defaultCullingLayout = device->CreatePipelineLayout({
		.bindings = {
				{
					.binding = 0,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = false
				},
				{
					.binding = 1,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = false
				},
				{
					.binding = 2,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = true
				},
				{
					.binding = 3,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = true
				},
			},
			.constants = {{ sizeof(CullingUBO), 0, RGL::StageVisibility::Compute}}
	});
	auto defaultCullCSH = LoadShaderByFilename("defaultcull.csh", device);
	defaultCullingComputePipeline = device->CreateComputePipeline({
		.stage = {
			.type = RGL::ShaderStageDesc::Type::Compute,
			.shaderModule = defaultCullCSH
		},
		.pipelineLayout = defaultCullingLayout
	});

	auto skinningDrawCallPrepareLayout = device->CreatePipelineLayout({
		.bindings = {
			{
				.binding = 0,
				.type = RGL::BindingType::StorageBuffer,
				.stageFlags = RGL::BindingVisibility::Compute,
				.writable = true
			}
		},
		.constants = {{ sizeof(SkinningPrepareUBO), 0, RGL::StageVisibility::Compute}}
	});

	auto drawcallPrepareCSH = LoadShaderByFilename("skinned_mesh_drawcall.csh",device);
	skinningDrawCallPreparePipeline = device->CreateComputePipeline({
		.stage = {
			.type = RGL::ShaderStageDesc::Type::Compute,
			.shaderModule = drawcallPrepareCSH,
		},
		.pipelineLayout = skinningDrawCallPrepareLayout
	});
}

void RavEngine::RenderEngine::createGBuffers()
{
	uint32_t width = bufferdims.width;
	uint32_t height = bufferdims.height;
	gcTextures.enqueue(depthStencil);
	gcTextures.enqueue(diffuseTexture);
	gcTextures.enqueue(normalTexture);
	gcTextures.enqueue(lightingTexture);

	depthStencil = device->CreateTexture({
		.usage = { .Sampled = true, .DepthStencilAttachment = true },
		.aspect = { .HasDepth = true },
		.width = width,
		.height = height,
		.format = RGL::TextureFormat::D32SFloat,
		.debugName = "Depth Texture"
		}
	);
	diffuseTexture = device->CreateTexture({
		.usage = { .Sampled = true, .ColorAttachment = true },
		.aspect = { .HasColor = true },
		.width = width,
		.height = height,
		.format = colorTexFormat,
		.initialLayout = RGL::ResourceLayout::Undefined,
		.debugName = "Color gbuffer"
		}
	);
	normalTexture = device->CreateTexture({
		.usage = { .Sampled = true, .ColorAttachment = true },
		.aspect = { .HasColor = true },
		.width = width,
		.height = height,
		.format = normalTexFormat,
		.initialLayout = RGL::ResourceLayout::Undefined,
		.debugName = "Normal gbuffer"
		}
	);
	lightingTexture = device->CreateTexture({
		.usage = { .Sampled = true, .ColorAttachment = true },
		.aspect = { .HasColor = true },
		.width = width,
		.height = height,
		.format = colorTexFormat,
		.initialLayout = RGL::ResourceLayout::Undefined,
		.debugName = "Lighting texture"
		}
	);

	auto tmpcmd = mainCommandQueue->CreateCommandBuffer();
	auto tmpfence = device->CreateFence(false);
	tmpcmd->Begin();
	for (const auto& ptr : { diffuseTexture , normalTexture, lightingTexture }) {
		tmpcmd->TransitionResource(ptr.get(), RGL::ResourceLayout::Undefined, RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::TransitionPosition::Top);
	}
	tmpcmd->TransitionResource(depthStencil.get(), RGL::ResourceLayout::Undefined, RGL::ResourceLayout::DepthReadOnlyOptimal, RGL::TransitionPosition::Top);
	tmpcmd->End();
	tmpcmd->Commit({
		.signalFence = tmpfence
		});
	tmpfence->Wait();
}

RavEngine::RenderEngine::~RenderEngine()
{
	mainCommandQueue->WaitUntilCompleted();
	swapchainFence->Wait();
	DestroyUnusedResources();
	device->BlockUntilIdle();
}

void RenderEngine::DestroyUnusedResources() {
	// deallocate the resources that have been freed

	auto clear = [](auto& queue) {
		typename std::remove_reference_t<decltype(queue)>::value_type item;
		while (queue.try_dequeue(item)) {}

	};

	clear(gcBuffers);
	clear(gcTextures);
	clear(gcPipelineLayout);
	clear(gcRenderPipeline);
}

void RenderEngine::resize(){
	UpdateBufferDims();
	createGBuffers();
#if TARGET_OS_IPHONE
	//view must be manually sized on iOS
	//also this API takes screen points not pixels
	resizeMetalLayer(metalLayer,windowdims.width, windowdims.height);
#endif
	mainCommandQueue->WaitUntilCompleted();
	swapchain->Resize(bufferdims.width, bufferdims.height);
}

void RenderEngine::SyncVideoSettings(){
	SDL_SetWindowSize(window, VideoSettings.width, VideoSettings.height);
}

/**
@return the name of the current rendering API
*/
const string_view RenderEngine::GetCurrentBackendName(){
	
	return RGL::APIToString(RGL::CurrentAPI());
}

float RavEngine::RenderEngine::GetCurrentFPS()
{
	return 1.0f / ((GetLastFrameTime() / 1000.f)/1000);
}

float RavEngine::RenderEngine::GetLastFrameTime()
{
	return currentFrameTime;
}


void RenderEngine::UpdateBufferDims(){
	// on non-apple platforms this is in pixels, on apple platforms it is in "screen points"
	// which will be dealt with later
	SDL_GetWindowSize(window, &windowdims.width, &windowdims.height);

	// update bufferdims
	bufferdims.width = windowdims.width;
	bufferdims.height = windowdims.height;

	// get the canvas size in pixels
# if _WIN32 && !_UWP
	
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(window, &wmi)) {
		Debug::Fatal("Cannot get native window information");
	}
	auto monitor = MonitorFromWindow(wmi.info.win.window, MONITOR_DEFAULTTONEAREST);
	DEVICE_SCALE_FACTOR fac;
	if (GetScaleFactorForMonitor(monitor,&fac) == S_OK) {
		win_scalefactor = (static_cast<int>(fac) / 100.0);
	}
	else {
		Debug::Fatal("GetScaleFactorForMonitor failed");
	}
#elif _UWP
	auto dinf = winrt::Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
	win_scalefactor = static_cast<int32_t>(dinf.ResolutionScale()) / 100.0;
#elif __APPLE__
	// since iOS and macOS do not use OpenGL we cannot use the GL call here
	// instead we derive it by querying display data
	float scale = GetWindowScaleFactor(window);
	bufferdims.width = windowdims.width * scale;
	bufferdims.height = windowdims.height * scale;
#endif
}

#ifndef NDEBUG
void RenderEngine::InitDebugger() const{
	if (!debuggerContext){
		Im3d::AppData& data = Im3d::GetAppData();
		data.drawCallback = &DebugRenderWrapper;
		
		debuggerContext.emplace(10,10);
		auto ctxd = (*debuggerContext).GetData();
		bool status = Rml::Debugger::Initialise(ctxd->context);
		
		debuggerInput = make_unique<InputManager>();
		
		debuggerInput->BindAnyAction(ctxd);
		debuggerInput->AddAxisMap("MouseX", Special::MOUSEMOVE_X);
		debuggerInput->AddAxisMap("MouseY", Special::MOUSEMOVE_Y);
		
		debuggerInput->AddAxisMap("ScrollY", Special::MOUSEWHEEL_Y);

		auto dbg = PointerInputBinder<GUIComponent>(&*debuggerContext);
		debuggerInput->BindAxis("MouseX", dbg, &GUIComponent::MouseX, CID::ANY, 0);	//no deadzone
		debuggerInput->BindAxis("MouseY", dbg, &GUIComponent::MouseY, CID::ANY, 0);
		debuggerInput->BindAxis("ScrollY", dbg, &GUIComponent::ScrollY, CID::ANY, 0);
	}
}

void RenderEngine::DeactivateDebugger() const{
	debuggerContext.reset();
	debuggerInput = nullptr;
}
#endif

void RenderEngine::SetWindowMode(WindowMode mode){
    int flag;
    switch(mode){
        case WindowMode::Windowed:
            flag = 0;
            break;
        case WindowMode::BorderlessFullscreen:
            flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
            break;
        case WindowMode::Fullscreen:
            flag = SDL_WINDOW_FULLSCREEN;
            break;
    }
    SDL_SetWindowFullscreen(window, flag);
}
