#pragma once
#include "Array.hpp"
#include "Vector.hpp"
#include "Map.hpp"
#include "mathtypes.hpp"

namespace RavEngine {
	struct SerializedSkeletonDataHeader {
		Array<char, 4> header = {'r','v','e','s'};
		uint32_t numBones = 0;
	};

	struct BoneTransform {
		glm::quat rotation;
		glm::vec3 translation, scale;
	};
	
	struct SerializedSkeleton {
		struct Bone {
			BoneTransform transform;
			std::string name;
		};
		Vector<Bone> allBones;
		UnorderedMap<uint16_t, Vector<uint16_t>> childrenMap;
	};

	struct SkeletonData {
		struct Bone {
			BoneTransform transform;
			std::string name;
			Vector<Bone> children;
		};
		Bone root;
	};
}