#pragma once
#include "Queryable.hpp"
#include "CTTI.hpp"
#include "RGL/Types.hpp"

namespace RavEngine {

	struct ParticleEmitter : public Queryable<ParticleEmitter>, public AutoCTTI {

		ParticleEmitter(uint32_t maxParticles);

		void Destroy();

	private:
		RGLBufferPtr
			particleDataBuffer = nullptr,
			particleReuseFreelist = nullptr,
			activeParticleIndexBuffer = nullptr;	// also used for indirect draws

	};

}