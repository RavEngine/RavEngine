#pragma once
#include "Queryable.hpp"
#include "CTTI.hpp"
#include "RGL/Types.hpp"
#include "Ref.hpp"
#include "ComponentWithOwner.hpp"

namespace RavEngine {

	struct BillboardParticleMaterial;
	struct MeshParticleMaterial;

	struct ParticleState {
		uint32_t aliveParticleCount = 0;
		uint32_t freeListCount = 0;
		uint32_t particlesCreatedThisFrame = 0;
		entity_t emitterOwnerID;
	};

	using ParticleMaterialVariant = std::variant<Ref<BillboardParticleMaterial>, Ref<MeshParticleMaterial>>;

	struct ParticleEmitter : public ComponentWithOwner, public Queryable<ParticleEmitter>{
		friend class RenderEngine;
		enum class Mode : uint8_t {
			Stream,
			Burst
		} mode = Mode::Stream;

		ParticleEmitter(entity_t owner, uint32_t maxParticles, const ParticleMaterialVariant& mat);

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

		auto GetMaxParticles() const {
			return maxParticleCount;
		}

		uint32_t spawnRate = 10;

		uint32_t numParticlesToSpawn() const {
			return spawnRate;
		}

	private:
		RGLBufferPtr
			particleDataBuffer = nullptr,
			particleReuseFreelist = nullptr,
			spawnedThisFrameList = nullptr,
			activeParticleIndexBuffer = nullptr,
			indirectComputeBuffer = nullptr,
			indirectDrawBuffer = nullptr,
			emitterStateBuffer = nullptr,
			particleLifeBuffer = nullptr;

		ParticleMaterialVariant material;

		uint16_t TotalParticleData() const;

		bool emittingThisFrame = false;
		uint32_t maxParticleCount;
	};

}