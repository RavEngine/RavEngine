#pragma once

namespace RavEngine {
	enum class OpacityMode : uint8_t {
		Opaque,
		Transparent
	};

	struct MeshAttributes {
		struct {
			bool position : 1 = false;
			bool normal : 1 = false;
			bool tangent : 1 = false;
			bool bitangent : 1 = false;
			bool uv0 : 1 = false;
			bool lightmapUV : 1 = false;
		};

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

		/**
		* `this` is compatible with `other` if for every bit set to true in `this`, the corresponding bit in `other` is also set to true.
		* It does not matter if `other` has additional bits set.
		*/
		bool CompatibleWith(const MeshAttributes& other) const{
			// someone PLEASE refactor this to be less stupid.
			if (position) {
				if (not other.position) {
					return false;
				}
			}
			if (normal) {
				if (not other.normal) {
					return false;
				}
			}
			if (tangent) {
				if (not other.tangent) {
					return false;
				}
			}
			if (bitangent) {
				if (not other.bitangent) {
					return false;
				}
			}
			if (uv0) {
				if (not other.uv0) {
					return false;
				}
			}
			if (lightmapUV) {
				if (not other.lightmapUV) {
					return false;
				}
			}
			return true;
		}
	};

}