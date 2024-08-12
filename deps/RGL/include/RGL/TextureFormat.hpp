#pragma once
#include <cstdint>
#undef None

namespace RGL {
    enum class ResourceLayout : uint8_t {
        Undefined = 0,
        General = 1,
        ColorAttachmentOptimal = 2,
        DepthStencilAttachmentOptimal = 3,
        DepthStencilReadOnlyOptimal = 4,
        ShaderReadOnlyOptimal = 5,
        TransferSourceOptimal = 6,
        TransferDestinationOptimal,
        Reinitialized,
        DepthReadOnlyStencilAttachmentOptimal,
        DepthAttachmentStencilReadOnlyOptimal,
        DepthAttachmentOptimal,
        DepthReadOnlyOptimal,
        StencilAttachmentOptimal,
        StencilReadOnlyOptimal,
        ReadOnlyOptimal,
        AttachmentOptimal,
        Present
    };


    enum class TextureFormat {
        Undefined,
        BGRA8_Unorm,
        RGBA8_Uint,
        RGBA8_Unorm,
        RGBA16_Unorm,
        RGBA16_Snorm,
        RGBA16_Sfloat,
        RGBA32_Sfloat,

        R8_Uint,
        R32_Uint,
        R32_Float,

		D32SFloat,			// 32 bit float
		D24UnormS8Uint,		// 24 bit depth, 8 bit stencil
	};

	enum class MSASampleCount : uint8_t {
            C0 = 0,
			C1 = 1,
			C2 = 2,
			C4 = 4,
			C8 = 8,
			C16 = 16,
			C32 = 32,
            C64 = 64,
	};

	enum class LoadAccessOperation : uint8_t {
		Load = 0,
		Clear,
		DontCare,
		NotAccessed
	};

	enum class StoreAccessOperation : uint8_t {
		Store,
		DontCare,
		None,
	};

    struct TextureUsage {
        bool TransferSource : 1 = false;
        bool TransferDestination : 1 = false;
        bool Sampled : 1 = false;
        bool Storage : 1 = false;
        bool ColorAttachment : 1 = false;
        bool DepthStencilAttachment : 1 = false;
        bool TransientAttachment : 1 = false;
        bool InputAttachment : 1 = false;
    };

    struct TextureAspect {
        bool HasColor : 1 = false;
        bool HasDepth : 1 = false;
        bool HasStencil : 1 = false;
        bool HasMetadata : 1 = false;
    };

}
