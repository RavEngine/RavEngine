//
//  RenderEngine.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//	Some code adapted from: https://github.com/pezcode/Cluster/blob/master/src/Renderer/DeferredRenderer.cpp
//
#if !RVE_SERVER
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
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <im3d.h>
#include <BuiltinMaterials.hpp>
#include "Common3D.hpp"
#include "DebugDrawer.hpp"
#include <glm/gtc/type_ptr.hpp>
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
#include "Texture.hpp"
#include "Profile.hpp"
#include "BuiltinTonemap.hpp"

#ifdef __APPLE__
	#include "AppleUtilities.h"
#endif

#if _WIN32
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")
#endif
#include <ParticleMaterial.hpp>

using namespace std;
using namespace RavEngine;

RenderEngine::vs RenderEngine::VideoSettings;


#ifndef NDEBUG
STATIC(RenderEngine::debuggerContext);
STATIC(RenderEngine::debuggerInput);
#endif

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
            float length = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
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
            float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

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

	constexpr auto deg2rad = (3.14159265 * 2) / 360.f;

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

/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine(const AppConfig& config, RGLDevicePtr device) : device(device) {
    
#if __APPLE__
    if (!AppleGPUMeetsMinSpec(device)) {
        Debug::Fatal("Cannot proceed: device \"{}\" is under the minimum spec!", device->GetBrandString());
    }
#endif
    
	mainCommandQueue = device->CreateCommandQueue(RGL::QueueType::AllCommands);
	mainCommandBuffer = mainCommandQueue->CreateCommandBuffer();
    transformSyncCommandBuffer = mainCommandQueue->CreateCommandBuffer();
    transientCommandBuffer = mainCommandQueue->CreateCommandBuffer();
	textureSampler = device->CreateSampler({});
	shadowSampler = device->CreateSampler({
		.addressModeU = RGL::SamplerAddressMode::Border,
		.addressModeV = RGL::SamplerAddressMode::Border,
		.borderColor = {0,0,0,0},
		.compareFunction = RGL::DepthCompareFunction::Greater,
		.minFilter = RGL::MinMagFilterMode::Linear,
		.magFilter = RGL::MinMagFilterMode::Linear,
		.mipFilter = RGL::MipFilterMode::Linear,
	});

    depthPyramidSampler = device->CreateSampler({
        .addressModeU = RGL::SamplerAddressMode::Clamp,
        .addressModeV = RGL::SamplerAddressMode::Clamp,
		.addressModeW = RGL::SamplerAddressMode::Clamp,
        .minFilter = RGL::MinMagFilterMode::Linear,
        .magFilter = RGL::MinMagFilterMode::Linear,
        .mipFilter = RGL::MipFilterMode::Nearest,
		.reductionMode = RGL::SamplerReductionMode::Minimum,
    });
    
	

    std::array<float, 4> depthClearColor = {0,0,0,0};
	// create render passes
	litRenderPass = RGL::CreateRenderPass({
		   .attachments = {
			   {
				   .format = colorTexFormat,
				   .loadOp = RGL::LoadAccessOperation::Load,
				   .storeOp = RGL::StoreAccessOperation::Store,
			   },
		   },
		   .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			   .format = RGL::TextureFormat::D32SFloat,
			   .loadOp = RGL::LoadAccessOperation::Load,
			   .storeOp = RGL::StoreAccessOperation::Store,
			   .clearColor = depthClearColor
		   }
		});

	litClearRenderPass = RGL::CreateRenderPass(
		{
		   .attachments = {
			   {
				   .format = colorTexFormat,
				   .loadOp = RGL::LoadAccessOperation::Clear,
				   .storeOp = RGL::StoreAccessOperation::Store,
			   },
		   },
		   .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			   .format = RGL::TextureFormat::D32SFloat,
			   .loadOp = RGL::LoadAccessOperation::Clear,
			   .storeOp = RGL::StoreAccessOperation::Store,
			   .clearColor = depthClearColor
		   }
		}
	);

	// dummy pass that clears framebuffers
	lightingClearRenderPass = RGL::CreateRenderPass({
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
    
    unlitRenderPass = RGL::CreateRenderPass({
        .attachments = {
            {
                .format = colorTexFormat,
                .loadOp = RGL::LoadAccessOperation::Load,
                .storeOp = RGL::StoreAccessOperation::Store,
            },
        },
         .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
            .format = RGL::TextureFormat::D32SFloat,
            .loadOp = RGL::LoadAccessOperation::Load,
            .storeOp = RGL::StoreAccessOperation::Store,
        }
    });

	litTransparentPass = RGL::CreateRenderPass({
		.attachments = {},
        .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
            .format = RGL::TextureFormat::D32SFloat,
            .loadOp = RGL::LoadAccessOperation::Load,
            .storeOp = RGL::StoreAccessOperation::Store,
        }
	});

	unlitTransparentPass = RGL::CreateRenderPass({
		.attachments = {},
        .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
            .format = RGL::TextureFormat::D32SFloat,
            .loadOp = RGL::LoadAccessOperation::Load,
            .storeOp = RGL::StoreAccessOperation::Store,
        }
	});

	// make sure to bind revealage to slot 1
	transparentClearPass = RGL::CreateRenderPass({
		.attachments = {
			{
				.format = RenderTargetCollection::formats[0],
				.loadOp = RGL::LoadAccessOperation::Clear,
				.storeOp = RGL::StoreAccessOperation::Store,
				.clearColor = {0,0,0,1}
			},
			{
                .format = RenderTargetCollection::formats[1],
				.loadOp = RGL::LoadAccessOperation::Clear,
				.storeOp = RGL::StoreAccessOperation::Store,
				.clearColor = {0,0,0,1}
			},
            {
                .format = RenderTargetCollection::formats[2],
                .loadOp = RGL::LoadAccessOperation::Clear,
                .storeOp = RGL::StoreAccessOperation::Store,
                .clearColor = {0,0,0,1}
            },
            {
                .format = RenderTargetCollection::formats[3],
                .loadOp = RGL::LoadAccessOperation::Clear,
                .storeOp = RGL::StoreAccessOperation::Store,
                .clearColor = {0,0,0,1}
            },
            {
                .format = RenderTargetCollection::mlabDepthFormat,
                .loadOp = RGL::LoadAccessOperation::Clear,
                .storeOp = RGL::StoreAccessOperation::Store,
                .clearColor = {1,1,1,1}
            },
		}
	});

    
    postProcessRenderPass = RGL::CreateRenderPass({
        .attachments = {
            {
                .format = colorTexFormat,
                .loadOp = RGL::LoadAccessOperation::Load,
                .storeOp = RGL::StoreAccessOperation::Store,
            },
        }
    });
	postProcessRenderPassClear = RGL::CreateRenderPass({
		.attachments = {
			{
				.format = colorTexFormat,
				.loadOp = RGL::LoadAccessOperation::Clear,
				.storeOp = RGL::StoreAccessOperation::Store,
			},
		}
	});

	finalRenderPass = RGL::CreateRenderPass({
		.attachments = {
			{
				.format = RGL::TextureFormat::BGRA8_Unorm,
				.loadOp = RGL::LoadAccessOperation::Load,
				.storeOp = RGL::StoreAccessOperation::Store,
			},
		},
		 .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			.format = RGL::TextureFormat::D32SFloat,
			.loadOp = RGL::LoadAccessOperation::Load,
			.storeOp = RGL::StoreAccessOperation::Store,
		}
	});

	finalClearRenderPass = RGL::CreateRenderPass({
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
			.clearColor = depthClearColor
		}
	});

	depthPrepassRenderPass = RGL::CreateRenderPass({
		.attachments = {},
		.depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			.format = RGL::TextureFormat::D32SFloat,
			.loadOp = RGL::LoadAccessOperation::Load,
			.storeOp = RGL::StoreAccessOperation::Store,
			.clearColor = depthClearColor
		}
	});

	dummyShadowmap = device->CreateTexture({
		.usage = {.TransferDestination = true,  .Sampled = true,},
		.aspect = {.HasDepth = true },
		.width = 1,
		.height = 1,
		.format = RGL::TextureFormat::D32SFloat,
		.debugName = "Dummy Shadowmap"
		}
	);
    
    dummyCubemap = device->CreateTexture({
        .usage = {.TransferDestination = true,  .Sampled = true,},
        .aspect = {.HasDepth = true },
        .width = 1,
        .height = 1,
        .arrayLayers = 6,
        .format = RGL::TextureFormat::D32SFloat,
        .isCubemap = true,
        .debugName = "Dummy Shadowmap"
        }
    );

	// create lighting render pipelines
	constexpr static uint32_t width = 640, height = 480;
	auto createLightingPipeline = [this, device](RGLShaderLibraryPtr vsh, RGLShaderLibraryPtr fsh, uint32_t vertexStride, uint32_t instanceStride, const std::vector<RGL::RenderPipelineDescriptor::VertexConfig::VertexAttributeDesc>& vertexAttributeDesc, RGLPipelineLayoutPtr layout, RGL::WindingOrder windingorder = RGL::WindingOrder::Counterclockwise) {

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
				.depthFunction = RGL::DepthCompareFunction::Less,
			},
			.pipelineLayout = layout,
		};

		return device->CreateRenderPipeline(rpd);
	};

	// data needed for lights
	struct Vertex2D {
		glm::vec2 pos;
	};


	auto debugPipelineLayout = device->CreatePipelineLayout({
		.constants = {
			{
				sizeof(DebugUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
			}
		}
	});

	auto guiPipelineLayout = device->CreatePipelineLayout({
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
				sizeof(GUIUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Vertex | RGL::StageVisibility::Fragment)
				}
		}
	});

	auto guiVSH = LoadShaderByFilename("gui_vsh", device);
	auto guiFSH = LoadShaderByFilename("gui_fsh", device);

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
			.pipelineLayout = guiPipelineLayout,
		});

	//TODO: on unified memory systems, don't make a staging buffer, and mark the transientBuffer as shared
	transientBuffer = device->CreateBuffer({
		65535,
		{.StorageBuffer = true},
		sizeof(char),
		RGL::BufferAccess::Private,
		{.TransferDestination = true, .PixelShaderResource = true, .debugName = "Transient Buffer" }
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
		   RGL::BufferAccess::Private,
			{.debugName = "Screen Triangle Vertex Bufer"}
		});
	screenTriVerts->SetBufferData(vertices);

	constexpr static ParticleQuadVert quadVerts[] = {
		{{-1,1}},
		{{-1,-1}},
		{{1,1}},
		{{1,-1}},
	};

	quadVertBuffer = device->CreateBuffer({
		{.VertexBuffer = true},
		sizeof(ParticleQuadVert),
		quadVerts,
		RGL::BufferAccess::Private,
		{.debugName = "Quad Vertex Bufer"}
	});
	quadVertBuffer->SetBufferData(quadVerts);

	lightClusterBuffer = device->CreateBuffer({
		Clustered::numClusters,
		{.StorageBuffer = true},
		sizeof(Clustered::Cluster),
		RGL::BufferAccess::Private,
		{.Writable = true, .debugName = "Light cluster buffer"}
	});

	// debug render pipelines
#ifndef NDEBUG
	auto debugVSH = LoadShaderByFilename("debug_vsh", device);
	auto debugFSH = LoadShaderByFilename("debug_fsh", device);
	auto createDebugRenderPipeline = [this, debugVSH, debugFSH, debugPipelineLayout, device](RGL::PolygonOverride drawMode, RGL::PrimitiveTopology topology) {
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
				.depthFormat = depthFormat,
				.depthTestEnabled = false,
				.depthWriteEnabled = false,
				.depthFunction = RGL::DepthCompareFunction::Greater,
			},
			.pipelineLayout = debugPipelineLayout,
		};
		return device->CreateRenderPipeline(rpd);
	};
	im3dLineRenderPipeline = createDebugRenderPipeline(RGL::PolygonOverride::Line, RGL::PrimitiveTopology::LineList);
	im3dPointRenderPipeline = createDebugRenderPipeline(RGL::PolygonOverride::Point, RGL::PrimitiveTopology::PointList);
	im3dTriangleRenderPipeline = createDebugRenderPipeline(RGL::PolygonOverride::Fill, RGL::PrimitiveTopology::TriangleList);
#endif

	// cluster grid build
	auto gridBuildLayout = device->CreatePipelineLayout({
		.bindings = {
			{
				.binding = 0,
				.type = RGL::BindingType::StorageBuffer,
				.stageFlags = RGL::BindingVisibility::Compute,
				.writable = true
			}
		},
		.constants = {{ sizeof(GridBuildUBO), 0, RGL::StageVisibility::Compute}}
	});
	clusterBuildGridPipeline = device->CreateComputePipeline(RGL::ComputePipelineDescriptor{
		.stage = {
			.type = RGL::ShaderStageDesc::Type::Compute,
			.shaderModule = LoadShaderByFilename("cluster_build_grid_csh",device)
		},
		.pipelineLayout = gridBuildLayout
	});

	auto clusterPopulateLayout = device->CreatePipelineLayout({
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
			}
		},
		.constants = {{ sizeof(GridAssignUBO), 0, RGL::StageVisibility::Compute}}
	});

	clusterPopulatePipeline = device->CreateComputePipeline(RGL::ComputePipelineDescriptor{
		.stage = {
			.type = RGL::ShaderStageDesc::Type::Compute,
			.shaderModule = LoadShaderByFilename("cluster_assign_lights_csh",device)
		},
		.pipelineLayout = clusterPopulateLayout
	});

	// skinned mesh compute pipeline
	auto skinnedCSH = LoadShaderByFilename("skinning_cs_csh", device);
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
				{
					.binding = 4,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
					.writable = false
				},
                {
                    .binding = 5,
                    .type = RGL::BindingType::StorageBuffer,
                    .stageFlags = RGL::BindingVisibility::Compute,
                    .writable = false
                },
				{
					.binding = 6,
					.type = RGL::BindingType::StorageBuffer,
					.stageFlags = RGL::BindingVisibility::Compute,
				},
				{
					.binding = 7,
					.type = RGL::BindingType::SampledImage,
					.stageFlags = RGL::BindingVisibility::Compute,
				},
				{
					.binding = 8,
					.type = RGL::BindingType::Sampler,
					.stageFlags = RGL::BindingVisibility::Compute,
				},
				
				
			},
			.constants = {{ sizeof(CullingUBO), 0, RGL::StageVisibility::Compute}}
	});
	auto defaultCullCSH = LoadShaderByFilename("defaultcull_csh", device);
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

	auto drawcallPrepareCSH = LoadShaderByFilename("skinned_mesh_drawcall_csh",device);
	skinningDrawCallPreparePipeline = device->CreateComputePipeline({
		.stage = {
			.type = RGL::ShaderStageDesc::Type::Compute,
			.shaderModule = drawcallPrepareCSH,
		},
		.pipelineLayout = skinningDrawCallPrepareLayout
	});
    
    auto depthPyramidLayout = device->CreatePipelineLayout({
        .bindings = {
            {
                .binding = 0,
                .type = RGL::BindingType::StorageImage,
                .stageFlags = RGL::BindingVisibility::Compute,
                .writable = true
            },
            {
                .binding = 2,
                .type = RGL::BindingType::Sampler,
                .stageFlags = RGL::BindingVisibility::Compute,
            },
            {
                .binding = 1,
                .type = RGL::BindingType::SampledImage,
                .stageFlags = RGL::BindingVisibility::Compute,
            }
        }
    });
    auto depthPyramidCSH = LoadShaderByFilename("depthpyramid_csh",device);
    depthPyramidPipeline = device->CreateComputePipeline({
        .stage = {
            .type = RGL::ShaderStageDesc::Type::Compute,
            .shaderModule = depthPyramidCSH
        },
        .pipelineLayout = depthPyramidLayout
    });

	auto depthPyramidCopyVSH = LoadShaderByFilename("depthpyramidcopy_vsh", device);
	auto depthPyramidCopyFSH = LoadShaderByFilename("depthpyramidcopy_fsh", device);

	auto depthPyramidCopyLayout = device->CreatePipelineLayout({
		.bindings = {
			{
				.binding = 0,
				.type = RGL::BindingType::SampledImage,
				.stageFlags = RGL::BindingVisibility::Fragment,
			},
			{
				.binding = 1,
				.type = RGL::BindingType::Sampler,
				.stageFlags = RGL::BindingVisibility::Fragment,
			}
		},
		.constants = {{sizeof(PyramidCopyUBO), 0,RGL::StageVisibility::Fragment}}
	});

	depthPyramidCopyPipeline = device->CreateRenderPipeline(RGL::RenderPipelineDescriptor{
		.stages = {
				{
					.type = RGL::ShaderStageDesc::Type::Vertex,
					.shaderModule = depthPyramidCopyVSH,
				},
				{
					.type = RGL::ShaderStageDesc::Type::Fragment,
					.shaderModule = depthPyramidCopyFSH,
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
		.rasterizerConfig = {
			.windingOrder = RGL::WindingOrder::Counterclockwise,
		},
		.colorBlendConfig = {
			.attachments = {
				{
					.format = depthPyramidFormat
				},
			}
		},
		.depthStencilConfig = {
			.depthTestEnabled = false,
			.depthWriteEnabled = false,
		},
		.pipelineLayout = depthPyramidCopyLayout,
	});

	depthPyramidCopyPass = RGL::CreateRenderPass({
		   .attachments = {
			   {
				   .format = depthPyramidFormat,
				   .loadOp = RGL::LoadAccessOperation::Clear,
				   .storeOp = RGL::StoreAccessOperation::Store,
			   },
		   },
		});
    defaultPostEffectVSH = LoadShaderByFilename("defaultpostprocess_vsh", device);
   

	auto transparencyApplyLayout = device->CreatePipelineLayout({
		.bindings = {
			{
				.binding = 0,
				.type = RGL::BindingType::StorageImage,
				.stageFlags = RGL::BindingVisibility::Fragment,
			},
            {
                .binding = 1,
                .type = RGL::BindingType::StorageImage,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },
            {
                .binding = 2,
                .type = RGL::BindingType::StorageImage,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },
            {
                .binding = 3,
                .type = RGL::BindingType::StorageImage,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },
		},
	});

	transparencyApplyPipeline = device->CreateRenderPipeline(RGL::RenderPipelineDescriptor{
		.stages = {
				{
					.type = RGL::ShaderStageDesc::Type::Vertex,
					.shaderModule = LoadShaderByFilename("transparency_apply_vsh", device),
				},
				{
					.type = RGL::ShaderStageDesc::Type::Fragment,
					.shaderModule = LoadShaderByFilename("transparency_apply_fsh", device),
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
		.rasterizerConfig = {
			.windingOrder = RGL::WindingOrder::Counterclockwise,
		},
		.colorBlendConfig = {
			.attachments = {
				{
					.format = colorTexFormat,
                    .sourceColorBlendFactor = RGL::BlendFactor::SourceAlpha,
                    .destinationColorBlendFactor = RGL::BlendFactor::OneMinusSourceAlpha,
                    .alphaBlendOperation = RGL::BlendOperation::Add,
                    .colorWriteMask = RGL::ColorWriteMask::RGB,
                    .blendEnabled = true,

				},
			}
		},
		.depthStencilConfig = {
			.depthTestEnabled = false,
			.depthWriteEnabled = false,
		},
		.pipelineLayout = transparencyApplyLayout,
	});

	transparencyApplyPass = RGL::CreateRenderPass({
		.attachments = {
			{
				.format = colorTexFormat,
				.loadOp = RGL::LoadAccessOperation::Load,
				.storeOp = RGL::StoreAccessOperation::Store,
			},
		},
	});

	auto navDebugLayout = device->CreatePipelineLayout({
		.constants = {{sizeof(navDebugUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Vertex)}}
	});

	auto recastDebugVSH = LoadShaderByFilename("debugNav_vsh", device);
	auto recastDebugFSH = LoadShaderByFilename("debugNav_fsh", device);
	auto createDebugNavPipeline = [navDebugLayout, device, recastDebugFSH, recastDebugVSH](RGL::PolygonOverride drawMode, RGL::PrimitiveTopology topology) {
		RGL::RenderPipelineDescriptor recastDebugDesc{
		.stages = {
				{
					.type = RGL::ShaderStageDesc::Type::Vertex,
					.shaderModule = recastDebugVSH,
				},
				{
					.type = RGL::ShaderStageDesc::Type::Fragment,
					.shaderModule = recastDebugFSH,
				}
		},
		.vertexConfig = {
			.vertexBindings = {
				{
					.binding = 0,
					.stride = sizeof(VertexColorUV),
				},
			},
			.attributeDescs = {
				{
					.location = 0,
					.binding = 0,
					.offset = 0,
					.format = RGL::VertexAttributeFormat::R32G32B32_SignedFloat,
				},
				{
					.location = 1,
					.binding = 0,
					.offset = offsetof(VertexColorUV,uv),
					.format = RGL::VertexAttributeFormat::R32G32_SignedFloat,
				},
				{
					.location = 2,
					.binding = 0,
					.offset = offsetof(VertexColorUV,color),
					.format = RGL::VertexAttributeFormat::R32_Uint,
				},
			}
		},
		.inputAssembly = {
			.topology = topology,
		},
		.rasterizerConfig = {
			.polygonOverride = drawMode,
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
			.depthFormat = depthFormat,
			.depthTestEnabled = true,
			.depthWriteEnabled = false,
			.depthFunction = RGL::DepthCompareFunction::Greater
		},
		.pipelineLayout = navDebugLayout,
		};

		return device->CreateRenderPipeline(recastDebugDesc);
	};
	
	recastLinePipeline = createDebugNavPipeline(RGL::PolygonOverride::Line, RGL::PrimitiveTopology::LineList);
	recastPointPipeline = createDebugNavPipeline(RGL::PolygonOverride::Line, RGL::PrimitiveTopology::PointList);
	recastTrianglePipeline = createDebugNavPipeline(RGL::PolygonOverride::Fill, RGL::PrimitiveTopology::TriangleList);

	auto particleCreateShader = LoadShaderByFilename("create_particle_csh", device);
	auto particleCreateLayout = device->CreatePipelineLayout({
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
				.writable = true
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
		.constants = {
			{sizeof(ParticleCreationPushConstants), 0, RGL::StageVisibility(RGL::StageVisibility::Compute)}
		}
	});
	particleCreatePipeline = device->CreateComputePipeline({
		.stage = {
			.type = RGL::ShaderStageDesc::Type::Compute,
			.shaderModule = particleCreateShader
		},
		.pipelineLayout = particleCreateLayout,
	});

	auto particleDispatchLayout = device->CreatePipelineLayout({
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
				.writable = true
			},
			{
				.binding = 2,
				.type = RGL::BindingType::StorageBuffer,
				.stageFlags = RGL::BindingVisibility::Compute,
				.writable = true
			},
		},
		.constants = {}
	});
	particleDispatchSetupPipeline = device->CreateComputePipeline({
		.stage = {
			.type = RGL::ShaderStageDesc::Type::Compute,
			.shaderModule = LoadShaderByFilename("particle_dispatch_setup_csh",device)
		},
		.pipelineLayout = particleDispatchLayout
	});

	auto particleDispatchLayoutIndexed = device->CreatePipelineLayout({
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
				.writable = true
			},
		},
		.constants = {}
	});

	particleDispatchSetupPipelineIndexed = device->CreateComputePipeline({
		.stage = {
			.type = RGL::ShaderStageDesc::Type::Compute,
			.shaderModule = LoadShaderByFilename("particle_dispatch_setup_indexed_csh",device)
		},
		.pipelineLayout = particleDispatchLayoutIndexed
	});

	auto particleKillLayout = device->CreatePipelineLayout({
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
				.writable = true
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
		.constants = {{sizeof(KillParticleUBO), 0, RGL::StageVisibility(RGL::StageVisibility::Compute)}}
	});
	particleKillPipeline = device->CreateComputePipeline({
		.stage = {
			.type = RGL::ShaderStageDesc::Type::Compute,
			.shaderModule = LoadShaderByFilename("kill_particle_csh",device)
		},
		.pipelineLayout = particleKillLayout
	});
}

RenderTargetCollection RavEngine::RenderEngine::CreateRenderTargetCollection(dim size, bool createDepth)
{
	uint32_t width = size.width;
	uint32_t height = size.height;

	RenderTargetCollection collection;

    if (createDepth) {
        collection.depthStencil = device->CreateTexture({
            .usage = {.Sampled = true, .DepthStencilAttachment = true },
            .aspect = {.HasDepth = true },
            .width = width,
            .height = height,
            .format = RGL::TextureFormat::D32SFloat,
            .debugName = "Depth Texture"
        });
        
        auto dim = std::min(width, height);
        
        collection.depthPyramid = {static_cast<uint16_t>(dim)};
        
    }

    RGL::TextureConfig lightingConfig{
        .usage = {.Sampled = true, .ColorAttachment = true },
        .aspect = {.HasColor = true },
        .width = width,
        .height = height,
        .format = colorTexFormat,
        .initialLayout = RGL::ResourceLayout::Undefined,
        .debugName = "Lighting texture Swap 1"
    };
    
	collection.lightingTexture = device->CreateTexture(lightingConfig);
    lightingConfig.debugName = "Lighting texture Swap 2";
    collection.lightingScratchTexture = device->CreateTexture(lightingConfig);

    for(const auto& [i, format] : Enumerate(RenderTargetCollection::formats)){
        collection.mlabAccum[i] = device->CreateTexture({
            .usage = {.Sampled = true, .Storage = true, .ColorAttachment = true },
            .aspect = {.HasColor = true },
            .width = width,
            .height = height,
            .format = format,
            .initialLayout = RGL::ResourceLayout::Undefined,
            .debugName = "MLAB Accumulation"
        });
    }
   collection.mlabDepth = device->CreateTexture({
        .usage = {.Sampled = true, .Storage = true, .ColorAttachment = true},
        .aspect = {.HasColor = true },
        .width = width,
        .height = height,
        .format = RenderTargetCollection::mlabDepthFormat,
        .initialLayout = RGL::ResourceLayout::Undefined,
        .debugName = "MLAB Depth"
    });

	return collection;
}

void RavEngine::RenderEngine::ResizeRenderTargetCollection(RenderTargetCollection& collection, dim size)
{
	gcTextures.enqueue(collection.depthStencil);
	gcTextures.enqueue(collection.lightingTexture);
    gcTextures.enqueue(collection.depthPyramid.pyramidTexture);
    gcTextures.enqueue(collection.lightingScratchTexture);
    gcTextures.enqueue(collection.mlabDepth);
    
    for(const auto tx : collection.mlabAccum){
        gcTextures.enqueue(tx);
    }

	auto newcol = CreateRenderTargetCollection(size);
	collection = newcol;
}

RavEngine::RenderEngine::~RenderEngine()
{
	mainCommandBuffer->BlockUntilCompleted();
	mainCommandQueue->WaitUntilCompleted();
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

/**
@return the name of the current rendering API
*/
const string_view RenderEngine::GetCurrentBackendName(){
	
	return RGL::APIToString(RGL::CurrentAPI());
}

#ifndef NDEBUG
void RenderEngine::InitDebugger() const{
	if (!debuggerContext){
		
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


size_t RavEngine::RenderEngine::GetCurrentVRAMUse()
{
	return device->GetCurrentVRAMInUse() / 1024 / 1024;
}

size_t RavEngine::RenderEngine::GetTotalVRAM()
{
	return device->GetTotalVRAM() / 1024 / 1024;
}

void RavEngine::RenderEngine::SyncVideoSettings()
{
	//TODO: implement
}
#endif
