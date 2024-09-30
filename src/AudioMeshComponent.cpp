#if !RVE_SERVER
#include "AudioMeshComponent.hpp"
namespace RavEngine {
	AudioMeshComponent::AudioMeshComponent(Entity ownerID, Ref<AudioMeshAsset> asset) : meshAsset(asset), ComponentWithOwner(ownerID)
	{
	}
}
#endif
