#if !RVE_SERVER
#include "ParticleEmitter.hpp"
#include "ParticleMaterial.hpp"
#include "App.hpp"


namespace RavEngine {
	RavEngine::ParticleEmitter::ParticleEmitter(uint32_t maxParticles, Ref<ParticleMaterial> mat) : material(mat)
	{
		auto device = GetApp()->GetDevice();

	}

	void RavEngine::ParticleEmitter::Destroy()
	{

	}
}

#endif