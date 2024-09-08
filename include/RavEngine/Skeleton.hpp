#pragma once
#include "Array.hpp"
#include "Vector.hpp"
#include "Map.hpp"
#include "mathtypes.hpp"

class aiNode;
class aiBone;
class aiScene;

namespace RavEngine {
	struct SerializedSkeletonDataHeader {
		Array<char, 4> header = { 'r','v','e','s' };
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
		Vector<Vector<uint16_t>> childrenMap;
		uint16_t IndexForBoneName(const std::string_view name);
	};

	struct SkeletonData {
		struct Bone {
			BoneTransform transform;
			std::string name;
			Vector<Bone> children;
		};
		Bone root;
	};


	struct NameToBoneResult { 
		UnorderedMap<std::string_view, aiBone*> bones; 
		aiNode* rootBone;
	};
	NameToBoneResult NameToBone(const aiScene* scene);
	SkeletonData CreateSkeleton(const NameToBoneResult& result);
	SerializedSkeleton FlattenSkeleton(const SkeletonData& skeleton);

	struct VertexWeights {
		struct vweights {
			struct vw {
				uint32_t joint_idx = 0;
				float influence = 0;
			};
			Vector<vw> weights;
		};

		vweights::vw w[4];
	};
}