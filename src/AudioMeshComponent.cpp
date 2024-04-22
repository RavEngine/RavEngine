#if !RVE_SERVER
#include "AudioMeshComponent.hpp"
namespace RavEngine {
	AudioMeshComponent::AudioMeshComponent(Ref<AudioMeshAsset> asset) : meshAsset(asset)
	{
	}
}
#endif