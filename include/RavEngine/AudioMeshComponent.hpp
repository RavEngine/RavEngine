#pragma once
#include <Ref.hpp>
#include "ComponentWithOwner.hpp"

namespace RavEngine {
	struct AudioMeshAsset;

	struct AudioMeshComponent : public ComponentWithOwner {
		AudioMeshComponent(entity_t ownerID, Ref<AudioMeshAsset>);

		const auto GetAsset() const {
			return meshAsset;
		}
	private:
		Ref<AudioMeshAsset> meshAsset;
	};
}
