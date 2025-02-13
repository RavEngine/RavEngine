#if !RVE_SERVER
#include "ParticleEmitter.hpp"
#include "ParticleMaterial.hpp"
#include "App.hpp"
#include <RGL/CommandBuffer.hpp>
#include "RenderEngine.hpp"
#include "CaseAnalysis.hpp"
#include "Debug.hpp"

namespace RavEngine {
	RavEngine::ParticleEmitter::ParticleEmitter(Entity ownerID, uint32_t maxParticles, uint16_t sizeOfEachParticle, const Ref<ParticleUpdateMaterialInstance> updateMat, const ParticleRenderMaterialVariant& mat) : ComponentWithOwner(ownerID), renderMaterial(mat), updateMaterial(updateMat), maxParticleCount(maxParticles)
	{

		// create buffers
		auto device = GetApp()->GetDevice();
		particleDataBuffer =  device->CreateBuffer({
				maxParticles, {.StorageBuffer = true}, sizeOfEachParticle, RGL::BufferAccess::Private, {.Writable = true, .debugName = "Particle Data Buffer"}
		});

		particleReuseFreelist = device->CreateBuffer({
			maxParticles, {.StorageBuffer = true}, sizeof(uint32_t), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Particle freelist"}
		});

		spawnedThisFrameList = device->CreateBuffer({
			maxParticles, {.StorageBuffer = true}, sizeof(uint32_t), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Particle Created This Frame Buffer"}
		});

		activeParticleIndexBuffer = device->CreateBuffer({
			maxParticles, {.StorageBuffer = true}, sizeof(uint32_t), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Alive particle index Buffer"}
		});

		indirectComputeBuffer = device->CreateBuffer({
			2, {.StorageBuffer = true, .IndirectBuffer = true}, sizeof(RGL::ComputeIndirectCommand), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Particle indirect compute buffer"}
		});

		indirectDrawBuffer = device->CreateBuffer({
			1, {.StorageBuffer = true, .IndirectBuffer = true}, sizeof(RGL::IndirectCommand), RGL::BufferAccess::Private, {.TransferDestination = true, .Writable = true, .debugName = "Particle indirect draw buffer"}
		});
		RGL::IndirectCommand initData{
			.vertexCount = 4,
			.instanceCount = 0,
			.firstVertex = 0,
			.firstInstance = 0,
		};
		indirectDrawBuffer->SetBufferData(initData);

		emitterStateBuffer = device->CreateBuffer({
			1, {.StorageBuffer = true}, sizeof(EmitterState), RGL::BufferAccess::Private, {.Transfersource = true, .Writable = true, .debugName = "Particle state buffer"}
		});
		emitterStateBuffer->SetBufferData(EmitterState{
            .emitterOwnerID = GetOwner().GetID()
		});

		particleLifeBuffer = device->CreateBuffer({
			maxParticles, {.StorageBuffer = true}, sizeof(float), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Particle life buffer"}
		});
	}

	void RavEngine::ParticleEmitter::Destroy()
	{
		auto& gcBuffers = GetApp()->GetRenderEngine().gcBuffers;
		for (const auto buffer : { particleDataBuffer, particleReuseFreelist, spawnedThisFrameList, activeParticleIndexBuffer, indirectComputeBuffer, indirectDrawBuffer, emitterStateBuffer, particleLifeBuffer, indirectDrawBufferStaging, meshAliveParticleIndexBuffer }) {
			gcBuffers.enqueue(buffer);
		}
	}
	void ParticleEmitter::Play()
	{
		emittingThisFrame = true;	// in burst mode, this will be set to false on the next frame
		lastSpawnTime = GetApp()->GetCurrentTime();
	}
	void ParticleEmitter::Stop()
	{
		emittingThisFrame = false;
	}
	void ParticleEmitter::SetEmissionRate(uint32_t rate)
	{
		spawnRate = rate;
	}
	uint32_t ParticleEmitter::GetNextParticleSpawnCount()
	{
		if (mode == ParticleEmitter::Mode::Burst) {
			return spawnRate;
		}
		else {
			// rate is particles per second
			auto now = GetApp()->GetCurrentTime();
			auto delta = now - lastSpawnTime;
			uint32_t nParticles = std::round(delta * spawnRate);
			if (nParticles > 0) {
				lastSpawnTime = now;
			}

			return nParticles;
		}
		
	}
}

#endif
