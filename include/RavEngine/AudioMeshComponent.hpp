#pragma once
#include <Ref.hpp>

namespace RavEngine {
	struct AudioMeshAsset;

	struct AudioMeshComponent {
		AudioMeshComponent(Ref<AudioMeshAsset>);

	private:
		Ref<AudioMeshAsset> meshAsset;
	};
}