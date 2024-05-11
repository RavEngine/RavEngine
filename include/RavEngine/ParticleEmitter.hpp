#pragma once
#include "Queryable.hpp"
#include "CTTI.hpp"
#include "RGL/Types.hpp"
#include "Ref.hpp"

namespace RavEngine {

	struct ParticleMaterial;

	struct ParticleEmitter : public Queryable<ParticleEmitter>, public AutoCTTI {

		ParticleEmitter(uint32_t maxParticles, Ref<ParticleMaterial> mat);

		void Destroy();

		const auto GetMaterial() const {
			return material;
		}

	private:
		RGLBufferPtr
			particleDataBuffer = nullptr,
			particleReuseFreelist = nullptr,
			activeParticleIndexBuffer = nullptr;	// also used for indirect draws

		Ref<ParticleMaterial> material;
	};

}