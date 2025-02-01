#pragma once

namespace RavEngine {
	enum class OpacityMode : uint8_t {
		Opaque,
		Transparent
	};

	struct MeshAttributes {
		bool position : 1 = false;
		bool normal : 1 = false;
		bool tangent : 1 = false;
		bool bitangent : 1 = false;
		bool uv0 : 1 = false;
		bool lightmapUV : 1 = false;

		bool operator==(const MeshAttributes& other) const {
			return
				position == other.position &&
				normal == other.normal &&
				tangent == other.tangent &&
				bitangent == other.bitangent &&
				uv0 == other.uv0 &&
				lightmapUV == other.lightmapUV
				;
		}
	};

}