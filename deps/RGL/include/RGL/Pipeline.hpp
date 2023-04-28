#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <string>
#include "TextureFormat.hpp"
#include <RGL/ShaderLibrary.hpp>
#include <RGL/Types.hpp>

namespace RGL {

	enum class DepthCompareFunction : uint8_t {
		Never = 0,			// matches the values of VkCompareOp
		Less = 1,
		Equal = 2,
		LessOrEqual = 3,
		Greater = 4,
		NotEqual = 5,
		GreaterOrEqual = 6,
		Always = 7,
	};

	enum class StencilOperation : uint8_t {
		Keep = 0,			// matches the values of VkStencilOp
		Zero = 1,
		Replace = 2,
		IncrementClamp = 3,
		DecrementClamp = 4,
		Invert = 5,
		IncrementWrap = 6,
		DecrementWrap = 7,
	};

	enum StageVisibility : uint8_t {
		Vertex		= 1,
		Fragment	= 1 << 1,
		Compute		= 1 << 2,
	};
	enum class PolygonOverride : uint8_t {
		Fill, Line, Point,							// lines, points, or use the existing config
	};

	enum class InputRate : uint8_t {
		Vertex, Instance
	};

	struct PipelineLayoutDescriptor {
		struct LayoutBindingDesc {
			uint32_t binding = 0;
			enum class Type : uint8_t {
				Sampler, CombinedImageSampler, SampledImage, StorageImage, UniformTexelBuffer, StorageTexelBuffer, UniformBuffer, StorageBuffer, UniformBufferDynamic, StorageBufferDynamic, InputAttachment
			} type;
			enum class StageFlags {
				Vertex = 0x00000001,
				Fragment = 0x00000010,
				Compute = 0x00000020,
			} stageFlags;
            bool writable = false;
			//TODO: support image samplers
		};
		std::vector<LayoutBindingDesc> bindings;

		std::vector<RGLSamplerPtr> boundSamplers;

		struct ConstantConfig {
			size_t size_bytes;
			uint8_t n_register = 0;
			StageVisibility visibility;

			ConstantConfig(decltype(size_bytes) sb, decltype(n_register) nr, StageVisibility visibility) : size_bytes(sb), n_register(nr), visibility(visibility){}

			template<typename T>
			ConstantConfig(const T& value, decltype(n_register) nr, StageVisibility visibility) : ConstantConfig(sizeof(T), (nr), visibility) {}

			template<typename T>
			ConstantConfig(decltype(n_register) nr, StageVisibility visibility) : ConstantConfig(sizeof(T), (nr), visibility) {}

		};
		std::vector<ConstantConfig> constants;
	};

	//TODO: support non-uniformbuffers (see VkDescriptorType)
	struct LayoutConfig {
		struct BufferConfig {
			RGLBufferPtr buffer;
			uint32_t offset = 0;
			uint32_t size = 0;
		};
		std::vector<BufferConfig> boundBuffers;
		
		struct TextureAndSampler {
			RGLTexturePtr texture;
			RGLSamplerPtr sampler;
		};
		std::vector<TextureAndSampler> boundTextures;
	};

	struct IPipelineLayout {
	};

	enum class PrimitiveTopology : uint8_t {
		PointList, LineList, LineStrip, TriangleList, TriangleStrip,TriangleFan,
		LineListAdjacency, LineStripAdjacency, TriangleListAdjacency, TriangleStripAdjacency,
		PatchList
	};

    enum class BlendFactor : uint8_t {
        Zero =0,
        One = 1,
        SourceColor = 2,
        OneMinusSourceColor = 3,
        DestColor = 4,
        OneMinusDestColor = 5,
        SourceAlpha = 6,
        OneMinusSourceAlpha = 7,
        DestAlpha = 8,
        OneMinusDestAlpha = 9,
        ConstantColor = 10,
        OneMinusConstantColor = 11,
        ConstantAlpha = 12,
        OneMinusConstantAlpha,
        SourceAlphaSaturate,
        Source1Color,
        OneMinusSource1Color,
        Source1Alpha,
        OneMinusSource1Alpha
    };

    enum class BlendOperation : uint8_t {
        Add, Subtract, ReverseSubtract, Min, Max
    };

    struct ShaderStageDesc {
        enum class Type : uint8_t {
            Vertex, Fragment, Compute
        } type;
        RGLShaderLibraryPtr shaderModule;
    };

    enum class VertexAttributeFormat {
        Undefined = 0,
        R32_Uint = 98,
        R32G32_SignedFloat = 103,
        R32G32B32_SignedFloat = 106
    };

    enum class WindingOrder : uint8_t {
        Clockwise, Counterclockwise
    };

    enum class CullMode : uint8_t {
        None = 0,
        Front = 0b01,
        Back = 0b10,
        Both = Front | Back
    };

	struct RenderPipelineDescriptor {
		
		std::vector<ShaderStageDesc> stages;

		// vertex info
		struct VertexConfig {
			struct VertexBindingDesc {
				uint32_t binding, stride;
				InputRate inputRate = InputRate::Vertex;
			} ;
			std::vector< VertexBindingDesc> vertexBindings;

			struct VertexAttributeDesc {
				uint32_t location, binding, offset;
				VertexAttributeFormat format;
			};
			std::vector<VertexAttributeDesc> attributeDescs;

		} vertexConfig;

		struct InputAssemblyDesc {
			PrimitiveTopology topology;
			bool primitiveRestartEnabled = false;
		} inputAssembly;

		struct ViewportDesc {		// we only support one viewport and one scissor for now
			float x = 0, y = 0,
				width = 0, height = 0,
				minDepth = 0, maxDepth = 1;  // depth values must be within [0,1] but minDepth does not need to be lower than maxDepth
		} viewport;

		struct ScissorDesc {
			std::pair<int, int> offset = { 0, 0 };
			std::pair<uint32_t, uint32_t>	extent = { 0, 0 };
		} scissor;

		struct RasterizerConfig {
			bool depthClampEnable : 1 = false;				 // if set to true, fragments out of range will be clamped instead of clipped, which we rarely want (example: shadow volumes, no need for end caps)
			bool rasterizerDiscardEnable : 1 = false;		// if true, output to the framebuffer is disabled

			PolygonOverride polygonOverride = PolygonOverride::Fill;

			CullMode cullMode = CullMode::Back;

			WindingOrder windingOrder : 1 = WindingOrder::Counterclockwise;

			struct DepthBias {
				float clamp = 0, constantFactor = 0, slopeFactor = 0;
				bool enable = false;
			} depthBias;

		} rasterizerConfig;

		struct MultisampleConfig {
			constexpr static MSASampleCount sampleCount = MSASampleCount::C1;	// configuring multisample is currently not supported
			constexpr static bool sampleShadingEnabled = false;	
			constexpr static bool alphaToCoverageEnabled = false;
			constexpr static bool alphaToOneEnabled = false;
			
		} multisampleConfig;

		// depth stencil states (not yet supported)
		struct ColorBlendConfig {
			enum class LogicalOperation : uint8_t {
				Clear, AND, AND_Reverse, Copy, ANDInverted, Noop, XOR, OR, NOR, Equivalent, Invert, ORReverse, CopyInverted, ORInverted, NAND, Set
			} logicalOperation = LogicalOperation::Copy;
			bool logicalOpEnabled : 1 = false;
			struct ColorAttachmentConfig {
				TextureFormat format;
				BlendFactor sourceColorBlendFactor = BlendFactor::One,
					destinationColorBlendFactor = BlendFactor::Zero,
					sourceAlphaBlendFactor = BlendFactor::One,
					destinationAlphaBlendFactor = BlendFactor::Zero;

				BlendOperation colorBlendOperation = BlendOperation::Add, alphaBlendOperation = BlendOperation::Add;

				enum class ColorWriteMask : uint8_t{
					Red		= 0b0001,
					Green	= 0b0010,
					Blue	= 0b0100,
					Alpha	= 0b1000,
					RGB = Red | Green | Blue,
					RGBA = RGB | Alpha
				} colorWriteMask = ColorWriteMask::RGBA;

				bool blendEnabled : 1 = false;
			};
			std::vector<ColorAttachmentConfig> attachments;	// create one for every attachment in the pass

			

			float blendconstants[4] = { 0,0,0,0 };
		} colorBlendConfig;

		struct DepthStencilConfig {
			TextureFormat depthFormat = TextureFormat::Undefined;
			TextureFormat stencilFormat = TextureFormat::Undefined;
			bool depthTestEnabled = false;
			bool depthWriteEnabled = false;
			DepthCompareFunction depthFunction = DepthCompareFunction::Never;

			bool stencilTestEnabled = false;
			StencilOperation stencilFrontOperation = StencilOperation::Keep;
			StencilOperation stencilBackOperation = StencilOperation::Keep;
		} depthStencilConfig;

		// list of what states are dynamic (TODO)
		RGLPipelineLayoutPtr pipelineLayout;
	};

	struct ComputePipelineDescriptor {
		ShaderStageDesc stage;
		RGLPipelineLayoutPtr pipelineLayout;
	};

	struct IRenderPipeline {

	};

	struct IComputePipeline {

	};
}
