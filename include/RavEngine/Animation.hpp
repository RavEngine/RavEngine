#pragma once
#include "Vector.hpp"
#include "mathtypes.hpp"
#include "Array.hpp"

namespace RavEngine {

	template<typename T>
	struct TKey {
		T value;
		float time = 0;
	};

	using TranslationKey = TKey<glm::vec3>;
	using ScaleKey = TranslationKey;
	using RotationKey = TKey<glm::quat>;

	struct JointAnimationTrack {
		Vector<TranslationKey> translations;
		Vector<RotationKey> rotations;
		Vector<ScaleKey> scales;
	};

	struct JointAnimation {
		Vector<JointAnimationTrack> tracks;
		std::string name;
		float duration = 0;
        float ticksPerSecond = 0;
	};

	struct SerializedJointAnimationHeader {
		const Array<char, 4> header = { 'r','v','e','a' };
		float duration = 0;
        float ticksPerSecond = 0;
		uint32_t numTracks = 0;
		uint16_t nameLength = 0;
	};

	struct SerializedJointAnimationTrackHeader {
		uint32_t numTranslations = 0;
		uint32_t numRotations = 0;
		uint32_t numScales = 0;
	};
}
