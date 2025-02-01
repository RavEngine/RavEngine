#pragma once

namespace RavEngine {
	enum class OpacityMode : uint8_t {
		Opaque,
		Transparent
	};

	struct MeshAttributes {
		union {
			struct {
				bool position : 1;
				bool normal : 1;
				bool tangent : 1;
				bool bitangent : 1;
				bool uv0 : 1;
				bool lightmapUV : 1;
				bool padding : 2;
			};
			uint8_t bits = 0;
		};

		bool operator==(const MeshAttributes& other) const {
			return bits == other.bits;
		}


		/**
		* `this` is compatible with `other` if for every bit set to true in `this`, the corresponding bit in `other` is also set to true.
		* It does not matter if `other` has additional bits set.
		*/
		bool CompatibleWith(const MeshAttributes& other) const {
			return (bits & other.bits) == bits;
		}
	};
		
}