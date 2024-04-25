#pragma once
#include <Ref.hpp>
#include "CTTI.hpp"

namespace RavEngine {
	struct AudioMeshAsset;

	struct AudioMeshComponent : public AutoCTTI {
		AudioMeshComponent(Ref<AudioMeshAsset>);

		const auto GetAsset() const {
			return meshAsset;
		}
	private:
		Ref<AudioMeshAsset> meshAsset;
	};
}
