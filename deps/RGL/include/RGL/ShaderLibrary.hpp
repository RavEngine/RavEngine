#pragma once

namespace RGL {

    enum class ShaderStage : uint8_t {
        Vertex,
        Fragment,
        Compute
    };

    struct FromSourceConfig{
        ShaderStage stage;
    };

	struct IShaderLibrary {

	};

}
