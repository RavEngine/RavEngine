#pragma once
#include "Queryable.hpp"
#include "CTTI.hpp"
#include "RGL/Types.hpp"
#include "Ref.hpp"
#include "ComponentWithOwner.hpp"

namespace RavEngine {

	struct BillboardParticleRenderMaterialInstance;
	struct MeshParticleRenderMaterialInstance;
	struct ParticleUpdateMaterialInstance;

	struct ParticleState {
		uint32_t aliveParticleCount = 0;
		uint32_t freeListCount = 0;
		uint32_t particlesCreatedThisFrame = 0;
		entity_t emitterOwnerID;
	};

	using ParticleRenderMaterialVariant = std::variant<Ref<BillboardParticleRenderMaterialInstance>, Ref<MeshParticleRenderMaterialInstance>>;

	struct ParticleEmitter : public ComponentWithOwner, public Queryable<ParticleEmitter>{
		friend class RenderEngine;
		enum class Mode : uint8_t {
			Stream,
			Burst
		} mode = Mode::Stream;

		ParticleEmitter(entity_t owner, uint32_t maxParticles, uint16_t sizeOfEachParticle, const Ref<ParticleUpdateMaterialInstance> updateMat, const ParticleRenderMaterialVariant& mat);

		void Destroy();

		const auto GetRenderMaterial() const {
			return renderMaterial;
		}

		const auto GetUpdateMaterial() const {
			return updateMaterial;
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

		void SetEmissionRate(uint32_t rate);

		// this function modifies internal state.
		// for internal use only
		uint32_t GetNextParticleSpawnCount();

	private:
		RGLBufferPtr
			particleDataBuffer = nullptr,
			particleReuseFreelist = nullptr,
			spawnedThisFrameList = nullptr,
			activeParticleIndexBuffer = nullptr,
			indirectComputeBuffer = nullptr,
			indirectDrawBuffer = nullptr,
			emitterStateBuffer = nullptr,
			particleLifeBuffer = nullptr,
			indirectDrawBufferStaging = nullptr, // not initialized in the Emitter constructor
			meshAliveParticleIndexBuffer = nullptr;		// not initialized in the Emitter constructor

		ParticleRenderMaterialVariant renderMaterial;
		Ref<ParticleUpdateMaterialInstance> updateMaterial;

		bool emittingThisFrame = false;
		uint32_t maxParticleCount;

		uint32_t spawnRate = 10;

		double lastSpawnTime = 0;

		// this is icky and nasty, we need a better solution than this
		struct RenderState {
			uint32_t maxTotalParticlesOffset = 0;
		} renderState;
	};

}