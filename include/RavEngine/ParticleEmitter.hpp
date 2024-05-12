#pragma once
#include "Queryable.hpp"
#include "CTTI.hpp"
#include "RGL/Types.hpp"
#include "Ref.hpp"

namespace RavEngine {

	struct ParticleMaterial;

	struct ParticleState {
		uint32_t aliveParticleCount = 0;
		uint32_t freeListCount = 0;
	};

	struct ParticleEmitter : public Queryable<ParticleEmitter>, public AutoCTTI {

		enum class Mode : uint8_t {
			Stream,
			Burst
		} mode = Mode::Stream;

		ParticleEmitter(uint32_t maxParticles, Ref<ParticleMaterial> mat);

		void Destroy();

		const auto GetMaterial() const {
			return material;
		}

		/**
		If mode is set to Stream, then the particle emitter will emit continuously until it is stopped. 
		If mode is set to Burst, then the particle emitter will emit a single burst during the next rendered frame. 
		*/
		void Play();

		/**
		If the emitter is set to stream mode, then this will prevent creation of new particles.
		*/
		void Stop();

		/**
		Set active particles to 0, and kill all existing active particles
		*/
		void Reset();

		bool IsEmitting() const {
			return emittingThisFrame;
		}

	private:
		RGLBufferPtr
			particleDataBuffer = nullptr,
			particleReuseFreelist = nullptr,
			activeParticleIndexBuffer = nullptr,
			indirectDrawBuffer = nullptr,
			particleStateBuffer = nullptr;

		Ref<ParticleMaterial> material;

		uint16_t TotalParticleData() const;

		bool emittingThisFrame = false;
	};

}