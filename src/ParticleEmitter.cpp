#if !RVE_SERVER
#include "ParticleEmitter.hpp"
#include "ParticleMaterial.hpp"
#include "App.hpp"
#include <RGL/CommandBuffer.hpp>
#include "RenderEngine.hpp"

namespace RavEngine {
	RavEngine::ParticleEmitter::ParticleEmitter(uint32_t maxParticles, Ref<ParticleMaterial> mat) : material(mat), maxParticleCount(maxParticles)
	{

		// create buffers
		auto device = GetApp()->GetDevice();
		particleDataBuffer =  device->CreateBuffer({
				maxParticles, {.StorageBuffer = true}, TotalParticleData(), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Particle Data Buffer"}
		});

		particleReuseFreelist = device->CreateBuffer({
			maxParticles, {.StorageBuffer = true}, sizeof(uint32_t), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Particle freelist"}
		});

		spawnedThisFrameList = device->CreateBuffer({
			maxParticles, {.StorageBuffer = true}, sizeof(uint32_t), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Particle Created This Frame Buffer"}
		});

		activeParticleIndexBuffer = device->CreateBuffer({
			maxParticles, {.StorageBuffer = true}, sizeof(uint32_t), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Particle freelist"}
		});

		indirectDrawBuffer = device->CreateBuffer({
			2, {.StorageBuffer = true, .IndirectBuffer = true}, sizeof(RGL::ComputeIndirectCommand), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Particle indirect draw buffer"}
		});

		particleStateBuffer = device->CreateBuffer({
			1, {.StorageBuffer = true}, sizeof(ParticleState), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Particle state buffer"}
		});
	}

	void RavEngine::ParticleEmitter::Destroy()
	{
		auto& gcBuffers = GetApp()->GetRenderEngine().gcBuffers;

		for (const auto buffer : { particleDataBuffer, particleReuseFreelist, spawnedThisFrameList, activeParticleIndexBuffer, indirectDrawBuffer, particleStateBuffer }) {
			gcBuffers.enqueue(buffer);
		}
	}
	void ParticleEmitter::Play()
	{
		emittingThisFrame = true;	// in burst mode, this will be set to false on the next frame
	}
	void ParticleEmitter::Stop()
	{
		emittingThisFrame = false;
	}
	void ParticleEmitter::Reset()
	{

	}
	uint16_t ParticleEmitter::TotalParticleData() const
	{
		return sizeof(ParticleEngineData) + material->ParticleUserDataSize();
	}
}

#endif