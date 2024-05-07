#if !RVE_SERVER
#include "AudioMeshComponent.hpp"
namespace RavEngine {
	AudioMeshComponent::AudioMeshComponent(entity_t ownerID, Ref<AudioMeshAsset> asset) : meshAsset(asset), ComponentWithOwner(ownerID)
	{
	}
}
#endif