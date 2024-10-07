#if !RVE_SERVER
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#include "RenderEngine.hpp"
#include <RGL/CommandBuffer.hpp>
#include <RGL/Swapchain.hpp>
#include <RGL/RenderPass.hpp>
#include "World.hpp"
#include "Skybox.hpp"
#include "DebugDraw.h"
#include "DebugDrawer.hpp"
#include <im3d.h>
#include <GUI.hpp>
#include <AnimatorComponent.hpp>
#include "MeshAssetSkinned.hpp"
#include "MeshAsset.hpp"
#include "SkeletonAsset.hpp"
#include "Texture.hpp"
#include "Debug.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include "Transform.hpp"
#include "ParticleEmitter.hpp"
#include "ParticleMaterial.hpp"
#include "CaseAnalysis.hpp"
#include "Profile.hpp"
#include "MeshCollection.hpp"

#if __APPLE__ || __EMSCRIPTEN__
#define OCCLUSION_CULLING_UNAVAILABLE
#endif


namespace RavEngine {

	

struct LightingType{
    bool Lit: 1 = false;
    bool Unlit: 1 = false;
	bool FilterLightBlockers : 1 = false;
	bool Transparent : 1 = false;
	bool Opaque : 1 = false;
};

#ifndef NDEBUG
	static DebugDrawer dbgdraw;	//for rendering debug primitives
#endif

	struct ParticleRenderFilterResult {
		Ref<ParticleRenderMaterial> material;
		bool isLit;
	};

	template <template<LightingMode> typename T>
	static ParticleRenderFilterResult particleRenderFilter(LightingType currentLightingType, auto&& inMat) {

		Ref<ParticleRenderMaterial> material;
		bool isLit = false;
		std::visit(CaseAnalysis{
				[&currentLightingType, &material,&isLit](const Ref<T<LightingMode::Lit>>& mat) {
					if (currentLightingType.Lit) {
						material = mat;
						isLit = true;
					}
				},
				[&currentLightingType, &material](const Ref<T<LightingMode::Unlit>>& mat) {
					if (currentLightingType.Unlit) {
						material = mat;
					}
				}
			}, inMat->GetMaterial());

		// transparency vs opaque 
		std::visit([&currentLightingType,&material](auto&& mat) {
		if (
			(mat->IsTransparent() && currentLightingType.Transparent)
			|| (!mat->IsTransparent() && currentLightingType.Opaque)
			) 
		{
			// if it was not set to true earlier, then do nothing
		}
		else {
			material = nullptr;
		}
		}, inMat->GetMaterial());

		return { material, isLit };

	};

	/**
 Render one frame using the current state of every object in the world
 */
	RGLCommandBufferPtr RenderEngine::Draw(Ref<RavEngine::World> worldOwning, const std::span<RenderViewCollection> screenTargets, float guiScaleFactor) {
		transientOffset = 0;

		RVE_PROFILE_FN_N("RenderEngine::Draw");
		DestroyUnusedResources();
		mainCommandBuffer->Reset();
		mainCommandBuffer->Begin();
		
		auto worldTransformBuffer = worldOwning->renderData.worldTransforms.buffer;

		// do skeletal operations
		struct skeletalMeshPrepareResult {
			bool skeletalMeshesExist = false;
		};
		auto prepareSkeletalMeshBuffers = [this, &worldOwning,&worldTransformBuffer]() -> skeletalMeshPrepareResult {
			// count objects
			uint32_t totalVertsToSkin = 0;
			uint32_t totalJointsToSkin = 0;
			uint32_t totalObjectsToSkin = 0;	// also the number of draw calls in the indirect buffer


			// resize buffers if they need to be resized
			auto resizeSkeletonBuffer = [this](RGLBufferPtr& buffer, uint32_t stride, uint32_t neededSize, RGL::BufferConfig::Type type, RGL::BufferAccess access, RGL::BufferFlags options = {}) {
				uint32_t currentCount = 0;
				if (!buffer || buffer->getBufferSize() / stride < neededSize) {
					if (buffer) {
						currentCount = buffer->getBufferSize() / stride;
						gcBuffers.enqueue(buffer);
					}
					auto newSize = closest_power_of(neededSize, 2);
					if (newSize == 0) {
						return;
					}
					buffer = device->CreateBuffer({
						newSize,
						type,
						stride,
						access,
						options
						});
					if (access == RGL::BufferAccess::Shared) {
						buffer->MapMemory();
					}
				}
			};

			for (auto& [materialInstance, drawcommand] : worldOwning->renderData.skinnedMeshRenderData) {
				uint32_t totalEntitiesForThisCommand = 0;
				for (auto& command : drawcommand.commands) {
					auto subCommandEntityCount = command.entities.DenseSize();
					totalObjectsToSkin += subCommandEntityCount;
					totalEntitiesForThisCommand += subCommandEntityCount;

					if (auto mesh = command.mesh.lock()) {
						totalVertsToSkin += mesh->GetNumVerts() * subCommandEntityCount;
					}

					if (auto skeleton = command.skeleton.lock()) {
						totalJointsToSkin += skeleton->GetSkeleton()->num_joints() * subCommandEntityCount;
					}
				}

				resizeSkeletonBuffer(drawcommand.indirectBuffer, sizeof(RGL::IndirectIndexedCommand), totalEntitiesForThisCommand, { .StorageBuffer = true, .IndirectBuffer = true }, RGL::BufferAccess::Private, { .Writable = true, .debugName = "Skeleton per-material IndirectBuffer" });
				//TODO: skinned meshes do not support LOD groups
				resizeSkeletonBuffer(drawcommand.cullingBuffer, sizeof(entity_t), totalEntitiesForThisCommand, { .StorageBuffer = true, .VertexBuffer = true }, RGL::BufferAccess::Private, { .Writable = true, .debugName = "Skeleton per-material cullingBuffer" });
			}

			resizeSkeletonBuffer(sharedSkeletonMatrixBuffer, sizeof(matrix4), totalJointsToSkin, { .StorageBuffer = true }, RGL::BufferAccess::Shared, { .debugName = "sharedSkeletonMatrixBuffer" });
			resizeSkeletonBuffer(sharedSkinnedMeshVertexBuffer, sizeof(VertexNormalUV), totalVertsToSkin, { .StorageBuffer = true, .VertexBuffer = true }, RGL::BufferAccess::Private, { .Writable = true, .debugName = "sharedSkinnedMeshVertexBuffer" });

			return {
				.skeletalMeshesExist = totalObjectsToSkin > 0 && totalVertsToSkin > 0
			};
		};
		auto skeletalPrepareResult = prepareSkeletalMeshBuffers();

		auto prepareSkeletalCullingBuffer = [this,&worldOwning]() {
			// dispatch compute to build the indirect buffer for finally rendering the skinned meshes
			// each skinned mesh gets its own 1-instance draw in the buffer. The instance count starts at 0.
			mainCommandBuffer->BeginComputeDebugMarker("Prepare Skinned Indirect Draw buffer");
			mainCommandBuffer->BeginCompute(skinningDrawCallPreparePipeline);
			for (auto& [materialInstance, drawcommand] : worldOwning->renderData.skinnedMeshRenderData) {
                SkinningPrepareUBO ubo;
				mainCommandBuffer->BindComputeBuffer(drawcommand.indirectBuffer, 0, 0);
				for (auto& command : drawcommand.commands) {
					const auto objectCount = command.entities.DenseSize();
					const auto mesh = command.mesh.lock();
					const auto vertexCount = mesh->GetNumVerts();

					ubo.nVerticesInThisMesh = vertexCount;
					ubo.nTotalObjects = objectCount;
					ubo.indexBufferOffset = mesh->GetAllocation().indexRange->start / sizeof(uint32_t);
					ubo.nIndicesInThisMesh = mesh->GetNumIndices();

					mainCommandBuffer->SetComputeBytes(ubo, 0);
					mainCommandBuffer->DispatchCompute(std::ceil(objectCount / 32.0f), 1, 1, 32, 1, 1);

					ubo.vertexBufferOffset += vertexCount;
					ubo.drawCallBufferOffset += objectCount;
					ubo.baseInstanceOffset += objectCount;
				}
			}
			mainCommandBuffer->EndCompute();
			mainCommandBuffer->EndComputeDebugMarker();
		};
		

		auto poseSkeletalMeshes = [this,&worldOwning]() {
			mainCommandBuffer->BeginComputeDebugMarker("Pose Skinned Meshes");
			mainCommandBuffer->BeginCompute(skinnedMeshComputePipeline);
			mainCommandBuffer->BindComputeBuffer(sharedSkinnedMeshVertexBuffer, 0);
			mainCommandBuffer->BindComputeBuffer(sharedVertexBuffer, 1);
			mainCommandBuffer->BindComputeBuffer(sharedSkeletonMatrixBuffer, 2);
			using mat_t = glm::mat4;
			std::span<mat_t> matbufMem{ static_cast<mat_t*>(sharedSkeletonMatrixBuffer->GetMappedDataPtr()), sharedSkeletonMatrixBuffer->getBufferSize() / sizeof(mat_t) };
			SkinningUBO subo;
			for (const auto& [materialInstance, drawcommand] : worldOwning->renderData.skinnedMeshRenderData) {
				for (auto& command : drawcommand.commands) {
					auto skeleton = command.skeleton.lock();
					auto mesh = command.mesh.lock();
					auto& entities = command.entities;
					mainCommandBuffer->BindComputeBuffer(mesh->GetWeightsBuffer(), 3);

					subo.numObjects = command.entities.DenseSize();
					subo.numVertices = mesh->GetNumVerts();
					subo.numBones = skeleton->GetSkeleton()->num_joints();
					subo.vertexReadOffset = mesh->GetAllocation().vertRange->start / sizeof(VertexNormalUV);

					// write joint transform matrices into buffer and update uniform offset
					{
						uint32_t object_id = 0;
						for (const auto& ownerid : command.entities.reverse_map) {
							auto& animator = worldOwning->GetComponent<AnimatorComponent>(ownerid);
							const auto& skinningMats = animator.GetSkinningMats();
							std::copy(skinningMats.begin(), skinningMats.end(), (matbufMem.begin() + subo.boneReadOffset) + object_id * skinningMats.size());
							object_id++;
						}
					}

					mainCommandBuffer->SetComputeBytes(subo, 0);
					mainCommandBuffer->DispatchCompute(std::ceil(subo.numObjects / 8.0f), std::ceil(subo.numVertices / 32.0f), 1, 8, 32, 1);
					subo.boneReadOffset += subo.numBones * subo.numObjects;
					subo.vertexWriteOffset += subo.numVertices * subo.numObjects;	// one copy of the vertex data per object
				}
			}
			mainCommandBuffer->EndCompute();
			mainCommandBuffer->EndComputeDebugMarker();
		};

		auto tickParticles = [this, worldOwning]() {
			mainCommandBuffer->BeginComputeDebugMarker("Particle Update");

			worldOwning->Filter([this, worldOwning](ParticleEmitter& emitter, const Transform& transform) {
				// frozen particle systems are not ticked
				if (emitter.GetFrozen()) {
					return;
				}

				Ref<ParticleRenderMaterialInstance> renderMat;
				auto updateMat = emitter.GetUpdateMaterial();

				Ref<MeshParticleMeshSelectionMaterialInstance> meshSelFn = nullptr;
				bool isMeshPipeline = false;
				uint32_t numMeshes = 0;
				
				std::visit(CaseAnalysis{
                        [&renderMat](const Ref <BillboardParticleRenderMaterialInstance> &billboardMat) {
                            renderMat = billboardMat;
                        },
                        [&renderMat, &meshSelFn, &isMeshPipeline](const Ref <MeshParticleRenderMaterialInstance> &meshMat) {
                            renderMat = meshMat;
							meshSelFn = meshMat->customSelectionFunction;
							isMeshPipeline = true;
                        }
                }, emitter.GetRenderMaterial());

				auto worldTransform = transform.GetWorldMatrix();

				auto dispatchSizeUpdate = [this, &emitter, &isMeshPipeline,&renderMat, &numMeshes, &meshSelFn] {

					if (isMeshPipeline) {
						// allocate indirect buffer
						auto asMeshInstance = std::static_pointer_cast<MeshParticleRenderMaterialInstance>(renderMat);
						auto meshCollection = asMeshInstance->meshes;

						auto nMeshes = meshCollection->GetNumLods();
						numMeshes = nMeshes;
						auto nCurrentCommands = emitter.indirectDrawBuffer->getBufferSize() / sizeof(RGL::IndirectIndexedCommand);
						if (nCurrentCommands != nMeshes || emitter.indirectDrawBuffer == nullptr || emitter.indirectDrawBufferStaging == nullptr) {
							gcBuffers.enqueue(emitter.indirectDrawBuffer);
							gcBuffers.enqueue(emitter.indirectDrawBufferStaging);
							emitter.indirectDrawBuffer = device->CreateBuffer({
								nMeshes, {.StorageBuffer = true, .IndirectBuffer = true}, sizeof(RGL::IndirectIndexedCommand), RGL::BufferAccess::Private, {.TransferDestination = true, .Writable = true, .debugName = "Particle indirect draw buffer"}
								});

							emitter.indirectDrawBufferStaging = device->CreateBuffer({ nMeshes, {.StorageBuffer = true}, sizeof(RGL::IndirectIndexedCommand), RGL::BufferAccess::Shared, {.Transfersource = true, .debugName = "Particle indirect draw buffer staging"} });
						}
						emitter.indirectDrawBufferStaging->MapMemory();
						auto ptr = static_cast<RGL::IndirectIndexedCommand*>(emitter.indirectDrawBufferStaging->GetMappedDataPtr());
						for (uint32_t i = 0; i < nMeshes; i++) {
							auto mesh = meshCollection->GetMeshForLOD(i);
							auto allocation = mesh->GetAllocation();
							*(ptr + i) = {
								.indexCount = uint32_t(mesh->GetNumIndices()),
								.instanceCount = 0,
								.indexStart = uint32_t(allocation.indexRange->start / sizeof(uint32_t)),
								.baseVertex = uint32_t(allocation.vertRange->start / sizeof(VertexNormalUV)),
								.baseInstance = i
							};
						}

						emitter.indirectDrawBufferStaging->UnmapMemory();
						mainCommandBuffer->CopyBufferToBuffer(
							{
								.buffer = emitter.indirectDrawBufferStaging,
								.offset = 0,
							},
							{
								.buffer = emitter.indirectDrawBuffer,
								.offset = 0,
							},
							emitter.indirectDrawBufferStaging->getBufferSize());
					}

					// setup dispatch sizes
					// we always need to run this because the Update shader may kill particles, changing the number of active particles
					if (isMeshPipeline) {
						mainCommandBuffer->BeginCompute(particleDispatchSetupPipelineIndexed);
					}
					else{ 
						mainCommandBuffer->BeginCompute(particleDispatchSetupPipeline);
					}
					mainCommandBuffer->BindComputeBuffer(emitter.emitterStateBuffer, 0);
					mainCommandBuffer->BindComputeBuffer(emitter.indirectComputeBuffer, 1);
					if (isMeshPipeline) {
						mainCommandBuffer->DispatchCompute(1, 1, 1, 1, 1, 1);
					}
					else {
						mainCommandBuffer->BindComputeBuffer(emitter.indirectDrawBuffer, 2);
						mainCommandBuffer->DispatchCompute(1, 1, 1, 1, 1, 1);	// this is kinda terrible...
					}
					mainCommandBuffer->EndCompute();

					// if there's no mesh selector function, or we have 1 mesh total,
					// sidestep the selector function and populate the count directly
					if (isMeshPipeline && (meshSelFn == nullptr || numMeshes == 1)) {
						// put the particle count into the indirect draw buffer
						mainCommandBuffer->CopyBufferToBuffer(
							{
								.buffer = emitter.emitterStateBuffer,
								.offset = offsetof(EmitterState,fields) + offsetof(EmitterStateNumericFields,aliveParticleCount)
							},
							{
								.buffer = emitter.indirectDrawBuffer,
								.offset = offsetof(RGL::IndirectIndexedCommand,instanceCount),
							},
							sizeof(EmitterStateNumericFields::aliveParticleCount)
						);
					}
				};

				bool hasCalculatedSizes = false;

				if (emitter.resetRequested) {
					
					EmitterStateNumericFields resetState{};

					emitter.emitterStateBuffer->SetBufferData(resetState);	// this will leave the emitter ID value untouched

					emitter.ClearReset();
				}

				// spawning particles?
				auto spawnCount = emitter.GetNextParticleSpawnCount();
				if (spawnCount > 0 && emitter.IsEmitting()) {
					ParticleCreationPushConstants constants{
						.particlesToSpawn = spawnCount,
						.maxParticles = emitter.GetMaxParticles(),
					};
					mainCommandBuffer->BeginComputeDebugMarker("Create and Init");
					mainCommandBuffer->BeginCompute(particleCreatePipeline);
					mainCommandBuffer->SetComputeBytes(constants, 0);

					mainCommandBuffer->BindComputeBuffer(emitter.activeParticleIndexBuffer, 0);
					mainCommandBuffer->BindComputeBuffer(emitter.particleReuseFreelist, 1);
					mainCommandBuffer->BindComputeBuffer(emitter.emitterStateBuffer, 2);
					mainCommandBuffer->BindComputeBuffer(emitter.spawnedThisFrameList, 3);

					mainCommandBuffer->DispatchCompute(std::ceil(spawnCount / 64.0f), 1, 1, 64, 1, 1);
					mainCommandBuffer->EndCompute();

					dispatchSizeUpdate();
					hasCalculatedSizes = true;

					// init particles
					mainCommandBuffer->BeginCompute(updateMat->mat->userInitPipeline);

					mainCommandBuffer->BindComputeBuffer(emitter.emitterStateBuffer,0);
					mainCommandBuffer->BindComputeBuffer(emitter.spawnedThisFrameList, 1);
					mainCommandBuffer->BindComputeBuffer(emitter.particleDataBuffer, 2);
					mainCommandBuffer->BindComputeBuffer(emitter.particleLifeBuffer, 3);
					mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.worldTransforms.buffer, 4);

					mainCommandBuffer->DispatchIndirect({
						.indirectBuffer = emitter.indirectComputeBuffer,
						.offsetIntoBuffer = 0,
                        .blocksizeX = 64, .blocksizeY = 1, .blocksizeZ = 1
					});

					mainCommandBuffer->EndCompute();
					mainCommandBuffer->EndComputeDebugMarker();
				}

				// burst mode
				if (emitter.mode == ParticleEmitter::Mode::Burst && emitter.IsEmitting()) {
					emitter.Stop();
				}

				if (!hasCalculatedSizes) {
					dispatchSizeUpdate();
				}

				// tick particles
				mainCommandBuffer->BeginComputeDebugMarker("Update, Kill");
				mainCommandBuffer->BeginCompute(updateMat->mat->userUpdatePipeline);

				mainCommandBuffer->BindComputeBuffer(emitter.emitterStateBuffer, 0);
				mainCommandBuffer->BindComputeBuffer(emitter.activeParticleIndexBuffer, 1);
				mainCommandBuffer->BindComputeBuffer(emitter.particleDataBuffer, 2);
				mainCommandBuffer->BindComputeBuffer(emitter.particleLifeBuffer, 3);

				ParticleUpdateUBO ubo{
					.fpsScale = GetApp()->GetCurrentFPSScale()
				};

				mainCommandBuffer->SetComputeBytes(ubo, 0);
				mainCommandBuffer->DispatchIndirect({
					.indirectBuffer = emitter.indirectComputeBuffer,
					.offsetIntoBuffer = sizeof(RGL::ComputeIndirectCommand),
                    .blocksizeX = 64, .blocksizeY = 1, .blocksizeZ = 1
				});

				mainCommandBuffer->EndCompute();

				// kill particles
				mainCommandBuffer->BeginCompute(particleKillPipeline);

				KillParticleUBO kubo{
					.maxTotalParticles = emitter.GetMaxParticles()
				};

				mainCommandBuffer->SetComputeBytes(kubo,0);

				mainCommandBuffer->BindComputeBuffer(emitter.emitterStateBuffer, 0);
				mainCommandBuffer->BindComputeBuffer(emitter.activeParticleIndexBuffer, 1);
				mainCommandBuffer->BindComputeBuffer(emitter.particleReuseFreelist, 2);
				mainCommandBuffer->BindComputeBuffer(emitter.particleLifeBuffer, 3);

				mainCommandBuffer->DispatchIndirect({
					.indirectBuffer = emitter.indirectComputeBuffer,
					.offsetIntoBuffer = sizeof(RGL::ComputeIndirectCommand),	// uses the same indirect command as the update shader, because it works on the alive set
                    .blocksizeX = 64, .blocksizeY = 1, .blocksizeZ = 1
				});

				mainCommandBuffer->EndCompute();
				mainCommandBuffer->EndComputeDebugMarker();

				if (isMeshPipeline && meshSelFn) {
					//custom mesh selection 

					// if the buffer doesn't exist yet, create it
					if (emitter.meshAliveParticleIndexBuffer == nullptr) {
						emitter.meshAliveParticleIndexBuffer = device->CreateBuffer({
							numMeshes * emitter.GetMaxParticles(), {.StorageBuffer = true}, sizeof(RGL::IndirectIndexedCommand), RGL::BufferAccess::Private, {.Writable = true, .debugName = "Alive particle index buffer (for meshes)"}
						});
					}

					struct {
						uint32_t numMeshes;
						uint32_t maxTotalParticles;
					} engineData{
						.numMeshes = numMeshes,
						.maxTotalParticles = emitter.GetMaxParticles()
					};

					auto transientOffset = WriteTransient(engineData);
					emitter.renderState.maxTotalParticlesOffset = transientOffset;

					// setup rendering
					auto selMat = meshSelFn->material;
					mainCommandBuffer->BeginComputeDebugMarker("Select meshes");
					mainCommandBuffer->BeginCompute(selMat->userSelectionPipeline);

					mainCommandBuffer->BindComputeBuffer(emitter.meshAliveParticleIndexBuffer, 10);
					mainCommandBuffer->BindComputeBuffer(emitter.indirectDrawBuffer, 11);
					mainCommandBuffer->BindComputeBuffer(transientBuffer, 12, transientOffset);
					mainCommandBuffer->BindComputeBuffer(emitter.emitterStateBuffer, 13);
					mainCommandBuffer->BindComputeBuffer(emitter.activeParticleIndexBuffer, 14);
					mainCommandBuffer->BindComputeBuffer(emitter.particleDataBuffer, 15);

					mainCommandBuffer->DispatchIndirect({
						.indirectBuffer = emitter.indirectComputeBuffer,
						.offsetIntoBuffer = sizeof(RGL::ComputeIndirectCommand),
						.blocksizeX = 64, .blocksizeY = 1, .blocksizeZ = 1
					});
					mainCommandBuffer->EndCompute();

					mainCommandBuffer->EndComputeDebugMarker();
				}
				
			});
			mainCommandBuffer->EndComputeDebugMarker();
		};

		tickParticles();
		
		// don't do operations if there's nothing to skin
		// these operations run once per frame since the results
		// are the same for all future passes
		if (skeletalPrepareResult.skeletalMeshesExist) {
			poseSkeletalMeshes();

			prepareSkeletalCullingBuffer();
		}

		auto renderFromPerspective = [this, &worldTransformBuffer, &worldOwning, &skeletalPrepareResult]<bool includeLighting = true, bool transparentMode = false>(const matrix4& viewproj, const matrix4& viewonly, const matrix4& projOnly, vector3 camPos, glm::vec2 zNearFar, RGLRenderPassPtr renderPass, auto&& pipelineSelectorFunction, RGL::Rect viewportScissor, LightingType lightingFilter, const DepthPyramid& pyramid, const renderlayer_t layers){
			RVE_PROFILE_FN_N("RenderFromPerspective");
            uint32_t particleBillboardMatrices = 0;

            struct QuadParticleData {
                glm::mat4 viewProj;
                glm::mat3 billboard;
            };
                
            glm::mat3 rotComp = viewonly;
                
            QuadParticleData quadData{
                .viewProj = viewproj,
                .billboard = glm::inverse(rotComp),
            };
                
            particleBillboardMatrices = WriteTransient(quadData);

			if constexpr (includeLighting) {
				// dispatch the lighting binning shaders
				RVE_PROFILE_SECTION(lightBinning,"Light binning");
				mainCommandBuffer->BeginComputeDebugMarker("Light Binning");
				const auto nPointLights = worldOwning->renderData.pointLightData.uploadData.DenseSize();
				const auto nSpotLights = worldOwning->renderData.spotLightData.uploadData.DenseSize();
				if (nPointLights > 0 || nSpotLights > 0) {
					{
						GridBuildUBO ubo{
							.invProj = glm::inverse(projOnly),
							.gridSize = {Clustered::gridSizeX, Clustered::gridSizeY, Clustered::gridSizeZ},
							.zNear = zNearFar.x,
							.screenDim = {viewportScissor.extent[0],viewportScissor.extent[1]},
							.zFar = zNearFar.y
						};

						mainCommandBuffer->BeginCompute(clusterBuildGridPipeline);
						mainCommandBuffer->BindComputeBuffer(lightClusterBuffer, 0);
						mainCommandBuffer->SetComputeBytes(ubo, 0);

						mainCommandBuffer->DispatchCompute(Clustered::gridSizeX, Clustered::gridSizeY, Clustered::gridSizeZ, 1, 1, 1);
						mainCommandBuffer->EndCompute();
					}

					// next assign lights to clusters
					{
						GridAssignUBO ubo{
							.viewMat = viewonly,
							.pointLightCount = nPointLights,
							.spotLightCount = nSpotLights
						};
						mainCommandBuffer->BeginCompute(clusterPopulatePipeline);
						mainCommandBuffer->SetComputeBytes(ubo, 0);
						mainCommandBuffer->BindComputeBuffer(lightClusterBuffer, 0);
						mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.pointLightData.uploadData.GetDense().get_underlying().buffer, 1);
						mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.spotLightData.uploadData.GetDense().get_underlying().buffer, 2);

						constexpr static auto threadGroupSize = 128;

						mainCommandBuffer->DispatchCompute(Clustered::numClusters / threadGroupSize, 1, 1, threadGroupSize, 1, 1);

						mainCommandBuffer->EndCompute();
					}
				}
				RVE_PROFILE_SECTION_END(lightBinning);
				mainCommandBuffer->EndComputeDebugMarker();
			}

#pragma pack(push, 1)
			struct LightData {
				glm::mat4 viewProj;
				glm::mat4 viewOnly;
				glm::mat4 projOnly;
				glm::uvec4 screenDimension;
				glm::vec3 camPos;
				glm::uvec3 gridSize;
				uint32_t ambientLightCount;
				uint32_t directionalLightCount;
				float zNear;
				float zFar;
			}
			lightData{
				.viewProj = viewproj,
				.viewOnly = viewonly,
				.projOnly = projOnly,
				.screenDimension = { viewportScissor.offset[0],viewportScissor.offset[1], viewportScissor.extent[0],viewportScissor.extent[1] },
				.camPos = camPos,
				.gridSize = { Clustered::gridSizeX, Clustered::gridSizeY, Clustered::gridSizeZ },
				.ambientLightCount = worldOwning->renderData.ambientLightData.uploadData.DenseSize(),
				.directionalLightCount = worldOwning->renderData.directionalLightData.uploadData.DenseSize(),
				.zNear = zNearFar.x,
				.zFar = zNearFar.y
			};

#pragma pack(pop)
			const auto lightDataOffset = WriteTransient(lightData);

			auto reallocBuffer = [this](RGLBufferPtr& buffer, uint32_t size_count, uint32_t stride, RGL::BufferAccess access, RGL::BufferConfig::Type type, RGL::BufferFlags flags) {
				if (buffer == nullptr || buffer->getBufferSize() < size_count * stride) {
					RVE_PROFILE_FN_N("Realloc buffer");
					// trash old buffer if it exists
					if (buffer) {
						gcBuffers.enqueue(buffer);
					}
					buffer = device->CreateBuffer({
						size_count,
						type,
						stride,
						access,
						flags
						});
					if (access == RGL::BufferAccess::Shared) {
						buffer->MapMemory();
					}
				}
				};

			

            auto cullSkeletalMeshes = [this, &worldTransformBuffer, &worldOwning, &reallocBuffer, layers, lightingFilter](matrix4 viewproj, const DepthPyramid pyramid) {
				RVE_PROFILE_FN_N("Cull Skeletal Meshes");
			// first reset the indirect buffers
				uint32_t skeletalVertexOffset = 0;
			for (auto& [materialInstance, drawcommand] : worldOwning->renderData.skinnedMeshRenderData) {

				uint32_t total_entities = 0;
				for (const auto& command : drawcommand.commands) {
					total_entities += command.entities.DenseSize();
				}

				reallocBuffer(drawcommand.indirectStagingBuffer, total_entities, sizeof(RGL::IndirectIndexedCommand), RGL::BufferAccess::Shared, { .StorageBuffer = true }, { .Transfersource = true, .Writable = false,.debugName = "Indirect Staging Buffer" });

				for (const auto& command : drawcommand.commands) {
					uint32_t meshID = 0;

					const auto nEntitiesInThisCommand = command.entities.DenseSize();
					RGL::IndirectIndexedCommand initData;
					if (auto mesh = command.mesh.lock()) {
						Debug::Assert(mesh->GetNumLods() == 1, "Skeletal meshes cannot have more than 1 LOD currently");
						for(uint32_t i = 0; i < nEntitiesInThisCommand; i++){
							for (uint32_t lodID = 0; lodID < mesh->GetNumLods(); lodID++) {
								const auto indexRange = mesh->GetAllocation().indexRange;
								initData = {
									.indexCount = uint32_t(mesh->GetNumIndices()),
									.instanceCount = 0,
									.indexStart = uint32_t((indexRange->start) / sizeof(uint32_t)),
									.baseVertex = skeletalVertexOffset,
									.baseInstance = i
								};
								drawcommand.indirectStagingBuffer->UpdateBufferData(initData, ((meshID * mesh->GetNumLods() + lodID + i)) * sizeof(RGL::IndirectIndexedCommand));
								//TODO: this increment needs to account for the LOD size
								skeletalVertexOffset += mesh->GetNumVerts();
							}
						}
						meshID++;
					}

					mainCommandBuffer->CopyBufferToBuffer(
						{
							.buffer = drawcommand.indirectStagingBuffer,
							.offset = 0
						},
						{
							.buffer = drawcommand.indirectBuffer,
							.offset = 0
						}, drawcommand.indirectStagingBuffer->getBufferSize()
					);
				}
			}

			// the culling shader will decide for each draw if the draw should exist (and set its instance count to 1 from 0).

			mainCommandBuffer->BeginComputeDebugMarker("Cull Skinned Meshes");
			mainCommandBuffer->BeginCompute(defaultCullingComputePipeline);
			mainCommandBuffer->BindComputeBuffer(worldTransformBuffer, 1);
            mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.renderLayers.buffer, 5);
			mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.perObjectAttributes.buffer, 6);
			for (auto& [materialInstance, drawcommand] : worldOwning->renderData.skinnedMeshRenderData) {
				CullingUBO cubo{
					.viewProj = viewproj,
					.indirectBufferOffset = 0,
					.singleInstanceModeAndShadowMode = 1u | (lightingFilter.FilterLightBlockers ? (1 << 1) : 0u),
					.numLODs = 1,
                    .cameraRenderLayers = layers
				};
				for (auto& command : drawcommand.commands) {
					mainCommandBuffer->BindComputeBuffer(drawcommand.cullingBuffer, 2);
					mainCommandBuffer->BindComputeBuffer(drawcommand.indirectBuffer, 3);

					if (auto mesh = command.mesh.lock()) {
						uint32_t lodsForThisMesh = 1;	// TODO: skinned meshes do not support LOD groups 

						cubo.numObjects = command.entities.DenseSize();
						mainCommandBuffer->BindComputeBuffer(command.entities.GetDense().get_underlying().buffer, 0);
						mainCommandBuffer->BindComputeBuffer(mesh->lodDistances.buffer, 4);
						cubo.radius = mesh->GetRadius();
#if __APPLE__
						constexpr size_t byte_size = closest_multiple_of<ssize_t>(sizeof(cubo), 16);
						std::byte bytes[byte_size]{};
						std::memcpy(bytes, &cubo, sizeof(cubo));
						mainCommandBuffer->SetComputeBytes({ bytes, sizeof(bytes) }, 0);
#else
						mainCommandBuffer->SetComputeBytes(cubo, 0);
#endif
						mainCommandBuffer->SetComputeTexture(pyramid.pyramidTexture->GetDefaultView(), 7);
						mainCommandBuffer->SetComputeSampler(depthPyramidSampler, 8);
						mainCommandBuffer->DispatchCompute(std::ceil(cubo.numObjects / 64.f), 1, 1, 64, 1, 1);
						cubo.indirectBufferOffset += lodsForThisMesh;
						cubo.cullingBufferOffset += lodsForThisMesh * command.entities.DenseSize();
					}
				}

			}
			mainCommandBuffer->EndComputeDebugMarker();
			mainCommandBuffer->EndCompute();
			};

			constexpr static auto filterRenderData = [](LightingType lightingFilter, auto& materialInstance) {
				bool shouldKeep = false;

				std::visit(CaseAnalysis{
					[lightingFilter, &shouldKeep](const Ref<LitMaterial>& mat) {
						if (lightingFilter.Lit) {
							shouldKeep = true;
						}
					},
					[lightingFilter, &shouldKeep](const Ref<UnlitMaterial>& mat) {
						if (lightingFilter.Unlit) {
							shouldKeep = true;
						}
					} }
				, materialInstance->GetMat()->variant);

				// transparency vs opaque 
				std::visit([&shouldKeep, &lightingFilter](auto&& mat) {
					if (
						(mat->IsTransparent() && lightingFilter.Transparent)
						|| (!mat->IsTransparent() && lightingFilter.Opaque)
						) {
						// if it was not set to true earlier, then do nothing
					}
					else {
						shouldKeep = false;
					}
					}, materialInstance->GetMat()->variant);

				return shouldKeep;
			};


            auto cullTheRenderData = [this, &viewproj, &worldTransformBuffer, &camPos, &pyramid, &lightingFilter, &reallocBuffer, layers, &worldOwning](auto&& renderData) {
				for (auto& [materialInstance, drawcommand] : renderData) {
					RVE_PROFILE_FN_N("Cull RenderData");
					bool shouldKeep = filterRenderData(lightingFilter, materialInstance);

					
					// is this the correct material type? if not, skip
					if (!shouldKeep) {
						continue;
					}

					//prepass: get number of LODs and entities
					uint32_t numLODs = 0, numEntities = 0;
					for (const auto& command : drawcommand.commands) {
						if (auto mesh = command.mesh.lock()) {
							numLODs += mesh->GetNumLods();
							numEntities += command.entities.DenseSize();
						}
					}

				
					const auto cullingbufferTotalSlots = numEntities * numLODs;
					reallocBuffer(drawcommand.cullingBuffer, cullingbufferTotalSlots, sizeof(entity_t), RGL::BufferAccess::Private, { .StorageBuffer = true, .VertexBuffer = true }, { .Writable = true, .debugName = "Culling Buffer" });
					reallocBuffer(drawcommand.indirectBuffer, numLODs, sizeof(RGL::IndirectIndexedCommand), RGL::BufferAccess::Private, { .StorageBuffer = true, .IndirectBuffer = true }, { .Writable = true, .debugName = "Indirect Buffer" });
					reallocBuffer(drawcommand.indirectStagingBuffer, numLODs, sizeof(RGL::IndirectIndexedCommand), RGL::BufferAccess::Shared, { .StorageBuffer = true }, { .Transfersource = true, .Writable = false,.debugName = "Indirect Staging Buffer" });

					// initial populate of drawcall buffer
					// we need one command per mesh per LOD
					{
						uint32_t meshID = 0;
						uint32_t baseInstance = 0;
						for (const auto& command : drawcommand.commands) {			// for each mesh
							const auto nEntitiesInThisCommand = command.entities.DenseSize();
							RGL::IndirectIndexedCommand initData;
							if (auto mesh = command.mesh.lock()) {
								for (uint32_t lodID = 0; lodID < mesh->GetNumLods(); lodID++) {
									const auto meshInst = mesh->GetMeshForLOD(lodID);
									initData = {
										.indexCount = uint32_t(meshInst->totalIndices),
										.instanceCount = 0,
										.indexStart = uint32_t(meshInst->meshAllocation.indexRange->start / sizeof(uint32_t)),
										.baseVertex = uint32_t(meshInst->meshAllocation.vertRange->start / sizeof(VertexNormalUV)),
										.baseInstance = baseInstance,	// sets the offset into the material-global culling buffer (and other per-instance data buffers). we allocate based on worst-case here, so the offset is known.
									};
									baseInstance += nEntitiesInThisCommand;
									drawcommand.indirectStagingBuffer->UpdateBufferData(initData, (meshID + lodID) * sizeof(RGL::IndirectIndexedCommand));
								}

							}
							meshID++;
						}
					}
					mainCommandBuffer->CopyBufferToBuffer(
						{
							.buffer = drawcommand.indirectStagingBuffer,
							.offset = 0
						},
				{
					.buffer = drawcommand.indirectBuffer,
					.offset = 0
				}, drawcommand.indirectStagingBuffer->getBufferSize());

					mainCommandBuffer->BeginCompute(defaultCullingComputePipeline);
					mainCommandBuffer->BindComputeBuffer(worldTransformBuffer, 1);
                    mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.renderLayers.buffer, 5);
					mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.perObjectAttributes.buffer, 6);
					CullingUBO cubo{
						.viewProj = viewproj,
						.camPos = camPos,
						.indirectBufferOffset = 0,
						.singleInstanceModeAndShadowMode = (lightingFilter.FilterLightBlockers ? (1 << 1) : 0u),
                        .cameraRenderLayers = layers
					};
					static_assert(sizeof(cubo) <= 128, "CUBO is too big!");
					for (auto& command : drawcommand.commands) {
						mainCommandBuffer->BindComputeBuffer(drawcommand.cullingBuffer, 2);
						mainCommandBuffer->BindComputeBuffer(drawcommand.indirectBuffer, 3);

						if (auto mesh = command.mesh.lock()) {
							uint32_t lodsForThisMesh = mesh->GetNumLods();

							cubo.numObjects = command.entities.DenseSize();
							mainCommandBuffer->BindComputeBuffer(command.entities.GetDense().get_underlying().buffer, 0);
							mainCommandBuffer->BindComputeBuffer(mesh->lodDistances.buffer, 4);
							cubo.radius = mesh->GetRadius();
							cubo.numLODs = lodsForThisMesh;

#if __APPLE__
							constexpr size_t byte_size = closest_multiple_of<ssize_t>(sizeof(cubo), 16);
							std::byte bytes[byte_size]{};
							std::memcpy(bytes, &cubo, sizeof(cubo));
							mainCommandBuffer->SetComputeBytes({ bytes, sizeof(bytes) }, 0);
#else
							mainCommandBuffer->SetComputeBytes(cubo, 0);
#endif
							mainCommandBuffer->SetComputeTexture(pyramid.pyramidTexture->GetDefaultView(), 7);
							mainCommandBuffer->SetComputeSampler(depthPyramidSampler, 8);
							mainCommandBuffer->DispatchCompute(std::ceil(cubo.numObjects / 64.f), 1, 1, 64, 1, 1);
							cubo.indirectBufferOffset += lodsForThisMesh;
							cubo.cullingBufferOffset += lodsForThisMesh * command.entities.DenseSize();
						}
					}
					mainCommandBuffer->EndCompute();
				}
				};
			auto renderTheRenderData = [this, &viewproj, &viewonly,&projOnly, &worldTransformBuffer, &pipelineSelectorFunction, &viewportScissor, &worldOwning, particleBillboardMatrices, &lightDataOffset,&layers](auto&& renderData, RGLBufferPtr vertexBuffer, LightingType currentLightingType) {
				// do static meshes
				RVE_PROFILE_FN_N("RenderTheRenderData");
				mainCommandBuffer->SetViewport({
					.x = float(viewportScissor.offset[0]),
					.y = float(viewportScissor.offset[1]),
					.width = static_cast<float>(viewportScissor.extent[0]),
					.height = static_cast<float>(viewportScissor.extent[1]),
					});
				mainCommandBuffer->SetScissor(viewportScissor);
				mainCommandBuffer->SetVertexBuffer(vertexBuffer);
				mainCommandBuffer->SetIndexBuffer(sharedIndexBuffer);
				for (auto& [materialInstance, drawcommand] : renderData) {

					bool shouldKeep = filterRenderData(currentLightingType, materialInstance);

					// is this the correct material type? if not, skip
					if (!shouldKeep) {
						continue;
					}

					// bind the pipeline
					auto pipeline = pipelineSelectorFunction(materialInstance->GetMat());
					mainCommandBuffer->BindRenderPipeline(pipeline);

					// this is always needed
					mainCommandBuffer->BindBuffer(transientBuffer, 11, lightDataOffset);

					if constexpr (includeLighting) {
						// make textures resident and put them in the right format
						worldOwning->Filter([this](const DirectionalLight& light, const Transform& t) {
							mainCommandBuffer->UseResource(light.shadowData.shadowMap->GetDefaultView());
						});
						worldOwning->Filter([this](const SpotLight& light, const Transform& t) {
							mainCommandBuffer->UseResource(light.shadowData.shadowMap->GetDefaultView());
						});

						mainCommandBuffer->BindBuffer(worldOwning->renderData.ambientLightData.uploadData.GetDense().get_underlying().buffer,12);
						mainCommandBuffer->BindBuffer(worldOwning->renderData.directionalLightData.uploadData.GetDense().get_underlying().buffer,13);
						mainCommandBuffer->SetFragmentSampler(shadowSampler, 14);
						mainCommandBuffer->BindBuffer(worldOwning->renderData.pointLightData.uploadData.GetDense().get_underlying().buffer, 15);
						mainCommandBuffer->BindBuffer(worldOwning->renderData.spotLightData.uploadData.GetDense().get_underlying().buffer, 17);
                        mainCommandBuffer->BindBuffer(worldOwning->renderData.renderLayers.buffer, 28);
						mainCommandBuffer->BindBuffer(worldOwning->renderData.perObjectAttributes.buffer, 29);
						mainCommandBuffer->BindBuffer(lightClusterBuffer, 16);
						mainCommandBuffer->SetFragmentTexture(device->GetGlobalBindlessTextureHeap(), 1);
						mainCommandBuffer->SetFragmentTexture(device->GetGlobalBindlessTextureHeap(), 2);
					}

					// set push constant data
					auto pushConstantData = materialInstance->GetPushConstantData();

					// Metal requires 16-byte alignment, so we bake that into the required size
					size_t pushConstantTotalSize =
#if __APPLE__
						closest_multiple_of<ssize_t>(pushConstantData.size(), 16);
#else
						pushConstantData.size();
#endif

					// AMD on vulkan cannot accept push constants > 128 bytes so we cap it there for all platforms
					std::byte totalPushConstantBytes[128]{};
					Debug::Assert(pushConstantTotalSize < std::size(totalPushConstantBytes), "Cannot write push constants, total size ({}) > {}", pushConstantTotalSize, std::size(totalPushConstantBytes));

					if (pushConstantData.size() > 0 && pushConstantData.data() != nullptr) {
						std::memcpy(totalPushConstantBytes, pushConstantData.data(), pushConstantData.size());
					}

					if (pushConstantTotalSize > 0) {
						mainCommandBuffer->SetVertexBytes({ totalPushConstantBytes ,pushConstantTotalSize }, 0);
						mainCommandBuffer->SetFragmentBytes({ totalPushConstantBytes ,pushConstantTotalSize }, 0);
					}

					// bind textures and buffers
					auto& bufferBindings = materialInstance->GetBufferBindings();
					auto& textureBindings = materialInstance->GetTextureBindings();
					for (int i = 0; i < materialInstance->maxBindingSlots; i++) {
						auto& buffer = bufferBindings[i];
						auto& texture = textureBindings[i];
						if (buffer) {
							mainCommandBuffer->BindBuffer(buffer, i);
						}
						if (texture) {
							mainCommandBuffer->SetFragmentSampler(textureSampler, 0); // TODO: don't hardcode this
							mainCommandBuffer->SetFragmentTexture(texture->GetRHITexturePointer()->GetDefaultView(), i);
						}
					}

					// bind the culling buffer and the transform buffer
					mainCommandBuffer->SetVertexBuffer(drawcommand.cullingBuffer, { .bindingPosition = 1 });
					mainCommandBuffer->BindBuffer(worldTransformBuffer, 10);

					// do the indirect command
					mainCommandBuffer->ExecuteIndirectIndexed({
						.indirectBuffer = drawcommand.indirectBuffer,
						.nDraws = uint32_t(drawcommand.indirectBuffer->getBufferSize() / sizeof(RGL::IndirectIndexedCommand))	// the number of structs in the buffer
						});
				}

				// render particles
                worldOwning->Filter([this, &viewproj, &particleBillboardMatrices, &currentLightingType, &pipelineSelectorFunction, &lightDataOffset, &worldOwning, &layers](const ParticleEmitter& emitter, const Transform& t) {
                    // check if the render layers match
                    auto renderLayers = worldOwning->renderData.renderLayers[emitter.GetOwner().GetID()];
                    if ((renderLayers & layers) == 0){
                        return;
                    }

					// check if casting shadows
                    auto attributes = worldOwning->renderData.perObjectAttributes[emitter.GetOwner().GetID()];
					const bool shouldConsider = !currentLightingType.FilterLightBlockers || (currentLightingType.FilterLightBlockers && (attributes & CastsShadowsBit));
					if (!shouldConsider) {
						return;
					}
                    
					if (!emitter.GetVisible()) {
						return;
					}

					auto sharedParticleImpl = [this, &particleBillboardMatrices, &pipelineSelectorFunction, &worldOwning, &lightDataOffset](const ParticleEmitter& emitter, auto&& materialInstance, Ref<ParticleRenderMaterial> material, RGLBufferPtr activeParticleIndexBuffer, bool isLit) {
						auto pipeline = pipelineSelectorFunction(material);


						mainCommandBuffer->BindRenderPipeline(pipeline);
						mainCommandBuffer->BindBuffer(emitter.particleDataBuffer, material->particleDataBufferBinding);
						mainCommandBuffer->BindBuffer(activeParticleIndexBuffer, material->particleAliveIndexBufferBinding);
                        mainCommandBuffer->BindBuffer(emitter.emitterStateBuffer, material->particleEmitterStateBufferBinding);
						mainCommandBuffer->BindBuffer(transientBuffer, material->particleMatrixBufferBinding, particleBillboardMatrices);

						mainCommandBuffer->BindBuffer(transientBuffer, 11, lightDataOffset);
						if (isLit) {
							mainCommandBuffer->BindBuffer(worldOwning->renderData.ambientLightData.uploadData.GetDense().get_underlying().buffer, 12);
							mainCommandBuffer->BindBuffer(worldOwning->renderData.directionalLightData.uploadData.GetDense().get_underlying().buffer, 13);
							mainCommandBuffer->SetFragmentSampler(shadowSampler, 14);
							mainCommandBuffer->BindBuffer(worldOwning->renderData.pointLightData.uploadData.GetDense().get_underlying().buffer, 15);
							mainCommandBuffer->BindBuffer(worldOwning->renderData.spotLightData.uploadData.GetDense().get_underlying().buffer, 17);
                            mainCommandBuffer->BindBuffer(worldOwning->renderData.renderLayers.buffer, 28);
							mainCommandBuffer->BindBuffer(worldOwning->renderData.perObjectAttributes.buffer, 29);
							mainCommandBuffer->BindBuffer(lightClusterBuffer, 16);
							mainCommandBuffer->SetFragmentTexture(device->GetGlobalBindlessTextureHeap(), 1);
							mainCommandBuffer->SetFragmentTexture(device->GetGlobalBindlessTextureHeap(), 2);	// redundant on some backends, needed for DX
						}

						std::byte pushConstants[128]{  };

						auto nbytes = materialInstance->SetPushConstantData(pushConstants);

						if (nbytes > 0) {
							mainCommandBuffer->SetVertexBytes({ pushConstants, nbytes }, 0);
							mainCommandBuffer->SetFragmentBytes({ pushConstants, nbytes }, 0);
						}

						// set samplers (currently sampler is not configurable)
						for (uint32_t i = 0; i < materialInstance->samplerBindings.size(); i++) {
							if (materialInstance->samplerBindings[i]) {
								mainCommandBuffer->SetFragmentSampler(textureSampler, i);
							}
						}

						// bind textures
						for (uint32_t i = 0; i < materialInstance->textureBindings.size(); i++) {
							if (materialInstance->textureBindings[i] != nullptr) {
								mainCommandBuffer->SetFragmentTexture(
									materialInstance->textureBindings[i]->GetRHITexturePointer()->GetDefaultView(), i);
							}
						}
						};

						

					std::visit(CaseAnalysis{
							[this, &emitter, &viewproj,&particleBillboardMatrices,&sharedParticleImpl,&currentLightingType](const Ref <BillboardParticleRenderMaterialInstance>& billboardMat) {
									
								// material will be nullptr if we should not render right now

								auto result = particleRenderFilter<BillboardRenderParticleMaterial>(currentLightingType, billboardMat);

								if (!result.material) {
									return;
								}

								sharedParticleImpl(emitter,billboardMat, result.material, emitter.activeParticleIndexBuffer,result.isLit);

								mainCommandBuffer->SetVertexBuffer(quadVertBuffer);

								mainCommandBuffer->ExecuteIndirect(
									{
										.indirectBuffer = emitter.indirectDrawBuffer,
										.offsetIntoBuffer = 0,
										.nDraws = 1,
									});

							},
							[this,&emitter,&sharedParticleImpl, &currentLightingType,&lightDataOffset](const Ref <MeshParticleRenderMaterialInstance>& meshMat) {
							RGLBufferPtr activeIndexBuffer;

								auto result = particleRenderFilter<MeshParticleRenderMaterial>(currentLightingType, meshMat);

								// material will be nullptr if we should not render right now

								if (meshMat->customSelectionFunction) {
									activeIndexBuffer = emitter.meshAliveParticleIndexBuffer;
								}
								else {
									activeIndexBuffer = emitter.activeParticleIndexBuffer;
								}

								if (!result.material) {
									return;
								}

								sharedParticleImpl(emitter, meshMat, result.material, activeIndexBuffer,result.isLit);

								mainCommandBuffer->SetVertexBuffer(sharedVertexBuffer);
								mainCommandBuffer->SetIndexBuffer(sharedIndexBuffer);
								mainCommandBuffer->BindBuffer(transientBuffer, MeshParticleRenderMaterialInstance::kEngineDataBinding, emitter.renderState.maxTotalParticlesOffset);

								mainCommandBuffer->ExecuteIndirectIndexed(
									{
										.indirectBuffer = emitter.indirectDrawBuffer,
										.offsetIntoBuffer = 0,
										.nDraws = meshMat->customSelectionFunction ? meshMat->meshes->GetNumLods() : 1,
									}
								);

							}
						}, emitter.GetRenderMaterial());
					});
				
			};

			// do culling operations
			mainCommandBuffer->BeginComputeDebugMarker("Cull Static Meshes");
			cullTheRenderData(worldOwning->renderData.staticMeshRenderData);
			mainCommandBuffer->EndComputeDebugMarker();
			if (skeletalPrepareResult.skeletalMeshesExist) {
				cullSkeletalMeshes(viewproj, pyramid);
			}


			// do rendering operations
			mainCommandBuffer->BeginRendering(renderPass);
			mainCommandBuffer->BeginRenderDebugMarker("Render Static Meshes");
			renderTheRenderData(worldOwning->renderData.staticMeshRenderData, sharedVertexBuffer, lightingFilter);
			mainCommandBuffer->EndRenderDebugMarker();
			if (skeletalPrepareResult.skeletalMeshesExist) {
				mainCommandBuffer->BeginRenderDebugMarker("Render Skinned Meshes");
				renderTheRenderData(worldOwning->renderData.skinnedMeshRenderData, sharedSkinnedMeshVertexBuffer, lightingFilter);
				mainCommandBuffer->EndRenderDebugMarker();
			}
			mainCommandBuffer->EndRendering();
			};

		struct lightViewProjResult {
			glm::mat4 lightProj, lightView;
			glm::vec3 camPos = glm::vec3{ 0,0,0 };
			DepthPyramid depthPyramid;
			RGLTexturePtr shadowmapTexture;
			glm::mat4 spillData;
		};

		// render shadowmaps only once per light


		// the generic shadowmap rendering function
		Profile::BeginFrame(Profile::RenderEncodeShadowmaps);
		auto renderLightShadowmap = [this, &renderFromPerspective, &worldOwning](auto&& lightStore, uint32_t numShadowmaps, auto&& genLightViewProjAtIndex, auto&& postshadowmapFunction) {
			if (lightStore.uploadData.DenseSize() <= 0) {
				return;
			}
			mainCommandBuffer->BeginRenderDebugMarker("Render shadowmap");
			for (uint32_t i = 0; i < lightStore.uploadData.DenseSize(); i++) {
				auto& light = lightStore.uploadData.GetDense()[i];
				if (!light.castsShadows) {
					continue;	// don't do anything if the light doesn't cast
				}
				auto sparseIdx = lightStore.uploadData.GetSparseIndexForDense(i);
				auto owner = Entity(sparseIdx, worldOwning.get());

				using lightadt_t = std::remove_reference_t<decltype(lightStore)>;

				void* aux_data = nullptr;
				if constexpr (lightadt_t::hasAuxData) {
					aux_data = &lightStore.auxData.GetDense()[i];
				}

				for (uint8_t i = 0; i < numShadowmaps; i++) {
					lightViewProjResult lightMats = genLightViewProjAtIndex(i, light, aux_data, owner);

					auto lightSpaceMatrix = lightMats.lightProj * lightMats.lightView;

					auto shadowTexture = lightMats.shadowmapTexture;

					shadowRenderPass->SetDepthAttachmentTexture(shadowTexture->GetDefaultView());
					auto shadowMapSize = shadowTexture->GetSize().width;
					renderFromPerspective.template operator()<false,false>(lightSpaceMatrix, lightMats.lightView, lightMats.lightProj, lightMats.camPos, {}, shadowRenderPass, [](auto&& mat) {
						return mat->GetShadowRenderPipeline();
						}, { 0, 0, shadowMapSize,shadowMapSize }, { .Lit = true, .Unlit = true, .FilterLightBlockers = true, .Opaque = true }, lightMats.depthPyramid, light.shadowLayers);

				}
				postshadowmapFunction(owner);
			}
			mainCommandBuffer->EndRenderDebugMarker();
		};

		Profile::BeginFrame(Profile::RenderEncodeSpotShadows);
		const auto spotlightShadowMapFunction = [](uint8_t index, RavEngine::World::SpotLightDataUpload& light, auto unusedAux, Entity owner) {

			auto lightProj = RMath::perspectiveProjection<float>(light.coneAngle * 2, 1, 0.1, 100);

			// -y is forward for spot lights, so we need to rotate to compensate
			auto rotmat = glm::toMat4(quaternion(vector3(-3.14159265358 / 2, 0, 0)));
			auto combinedMat = light.worldTransform * rotmat;

			auto viewMat = glm::inverse(combinedMat);

			auto camPos = light.worldTransform * glm::vec4(0, 0, 0, 1);

			auto& origLight = owner.GetComponent<SpotLight>();

			light.lightViewProj = lightProj * viewMat;	// save this because the shader needs it

			return lightViewProjResult{
				.lightProj = lightProj,
				.lightView = viewMat,
				.camPos = camPos,
				.depthPyramid = origLight.shadowData.pyramid,
				.shadowmapTexture = origLight.shadowData.shadowMap,
				.spillData = light.lightViewProj
			};
        };
        
		renderLightShadowmap(worldOwning->renderData.spotLightData, 1,
			spotlightShadowMapFunction,
			[](Entity unused) {}
		);
		Profile::EndFrame(Profile::RenderEncodeSpotShadows);

		Profile::BeginFrame(Profile::RenderEncodePointShadows);
		constexpr auto pointLightShadowmapFunction = [](uint8_t index, const RavEngine::World::PointLightUploadData& light, auto unusedAux, Entity owner) {
			auto lightProj = RMath::perspectiveProjection<float>(deg_to_rad(90), 1, 0.1, 100);

			glm::mat4 viewMat;
            auto lightPos = light.position;

			// rotate view space to each cubemap direction based on the index
            const glm::mat4 rotationMatrices[] = {
                {// +x
                    glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ -1.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f })
                },
                {// -x
                    glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f })
                },
                {// +y
                    glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f })
                },
                {// -y
                    glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, -1.0f })
                },
                // +Z
                {
                    glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f })
               },
                // -z
                    {
                    glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f })
                }
            };
            
            // center around light
            viewMat = glm::translate(rotationMatrices[index], -lightPos);
            
			auto camPos = light.position;

			auto& origLight = owner.GetComponent<PointLight>();

			return lightViewProjResult{
				.lightProj = lightProj,
				.lightView = viewMat,
				.camPos = camPos,
				.depthPyramid = origLight.shadowData.cubePyramids[index],
				.shadowmapTexture = origLight.shadowData.cubeShadowmaps[index],
				.spillData = lightProj
			};
		};

		renderLightShadowmap(worldOwning->renderData.pointLightData, 6,
			pointLightShadowmapFunction,
			[this](Entity owner) {
				auto& origLight = owner.GetComponent<PointLight>();
				for (uint32_t i = 0; i < 6; i++) {
					mainCommandBuffer->CopyTextureToTexture(
						{
							.texture = origLight.shadowData.cubeShadowmaps[i]->GetDefaultView(),
							.mip = 0,
							.layer = 0
						},
						{
							.texture = origLight.shadowData.mapCube->GetDefaultView(),
							.mip = 0,
							.layer = i
						}
					);
				}
		});
		Profile::EndFrame(Profile::RenderEncodePointShadows);
		Profile::EndFrame(Profile::RenderEncodeShadowmaps);

		RVE_PROFILE_SECTION(allViews, "Render Encode All Views");
		for (const auto& view : screenTargets) {
			currentRenderSize = view.pixelDimensions;
			auto nextImgSize = view.pixelDimensions;
			auto& target = view.collection;

			auto renderLitPass_Impl = [this,&target, &renderFromPerspective,&renderLightShadowmap,&worldOwning]<bool transparentMode = false>(auto&& camData, auto&& fullSizeViewport, auto&& fullSizeScissor, auto&& renderArea) {
				// directional light shadowmaps

				if constexpr (!transparentMode) {
					RVE_PROFILE_SECTION(dirShadow, "Render Encode Dirlight shadowmap");
					mainCommandBuffer->BeginRenderDebugMarker("Render Directional Lights");
                    const auto dirlightShadowmapDataFunction = [&camData](uint8_t index, RavEngine::World::DirLightUploadData& light, auto auxDataPtr, Entity owner) {
						auto dirvec = light.direction;

						auto auxdata = static_cast<World::DirLightAuxData*>(auxDataPtr);

						auto lightArea = auxdata->shadowDistance;

						auto lightProj = RMath::orthoProjection<float>(-lightArea, lightArea, -lightArea, lightArea, -100, 100);
						auto lightView = glm::lookAt(dirvec, { 0,0,0 }, { 0,1,0 });
						const vector3 reposVec{ std::round(-camData.camPos.x), std::round(camData.camPos.y), std::round(-camData.camPos.z) };
						lightView = glm::translate(lightView, reposVec);

						auto& origLight = owner.GetComponent<DirectionalLight>();

						light.lightViewProj = lightProj * lightView;	// remember this because the rendering also needs it

						return lightViewProjResult{
							.lightProj = lightProj,
							.lightView = lightView,
							.camPos = camData.camPos,
							.depthPyramid = origLight.shadowData.pyramid,
							.shadowmapTexture = origLight.shadowData.shadowMap,
							.spillData = light.lightViewProj
						};
						};


					renderLightShadowmap(worldOwning->renderData.directionalLightData, 1,
						dirlightShadowmapDataFunction,
						[](Entity unused) {}
					);
					mainCommandBuffer->EndRenderDebugMarker();
					RVE_PROFILE_SECTION_END(dirShadow);
				}

				// render all the static meshes

				renderFromPerspective.template operator()<true, transparentMode>(camData.viewProj, camData.viewOnly, camData.projOnly, camData.camPos, camData.zNearFar, transparentMode ? litTransparentPass : litRenderPass, [](auto&& mat) {
					return mat->GetMainRenderPipeline();
                }, renderArea, {.Lit = true, .Transparent = transparentMode, .Opaque = !transparentMode, }, target.depthPyramid, camData.layers);

				
			};

            auto renderLitPass = [&renderLitPass_Impl](auto&& camData, auto&& fullSizeViewport, auto&& fullSizeScissor, auto&& renderArea) {
				renderLitPass_Impl(camData, fullSizeViewport, fullSizeScissor, renderArea);
			};

			auto renderLitPassTransparent = [&renderLitPass_Impl](auto&& camData, auto&& fullSizeViewport, auto&& fullSizeScissor, auto&& renderArea) {
				renderLitPass_Impl.template operator()<true>(camData, fullSizeViewport, fullSizeScissor, renderArea);
			};

            auto renderFinalPass = [this, &target, &worldOwning, &view, &guiScaleFactor, &nextImgSize, &renderFromPerspective](auto&& camData, auto&& fullSizeViewport, auto&& fullSizeScissor, auto&& renderArea) {
                
                //render unlits
				RVE_PROFILE_SECTION(unlit, "Encode Unlit Opaques");
                unlitRenderPass->SetAttachmentTexture(0, target.lightingTexture->GetDefaultView());
                unlitRenderPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());
				renderFromPerspective.template operator() < false > (camData.viewProj, camData.viewOnly, camData.projOnly, camData.camPos, {}, unlitRenderPass, [](auto&& mat) {
                    return mat->GetMainRenderPipeline();
                }, renderArea, {.Unlit = true, .Opaque = true }, target.depthPyramid, camData.layers);
				RVE_PROFILE_SECTION_END(unlit);

				// render unlits with transparency
				RVE_PROFILE_SECTION(unlittrans, "Encode Unlit Transparents");
				unlitTransparentPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());
				renderFromPerspective.template operator() < false, true > (camData.viewProj, camData.viewOnly, camData.projOnly, camData.camPos, {}, unlitTransparentPass, [](auto&& mat) {
					return mat->GetMainRenderPipeline();
				}, renderArea, { .Unlit = true, .Transparent = true }, target.depthPyramid, camData.layers);
				RVE_PROFILE_SECTION_END(unlittrans);
                
                // then do the skybox, if one is defined.
                if (worldOwning->skybox && worldOwning->skybox->skyMat && worldOwning->skybox->skyMat->GetMat()->renderPipeline) {
                    struct skyboxData{
                        glm::mat3 invView;
                        glm::vec3 camPos;
                        float fov;
                        float aspectRatio;
                    } data {
                        glm::inverse(camData.viewOnly),
                        camData.camPos,
                        deg_to_rad(camData.fov),
                        float(fullSizeViewport.width) / float(fullSizeViewport.height)
                    };
                    
                    auto transientOffset = WriteTransient(data);
                    
                    mainCommandBuffer->BeginRendering(unlitRenderPass);
                    mainCommandBuffer->BeginRenderDebugMarker("Skybox");
                    mainCommandBuffer->SetViewport(fullSizeViewport);
                    mainCommandBuffer->SetScissor(fullSizeScissor);
                    mainCommandBuffer->BindRenderPipeline(worldOwning->skybox->skyMat->GetMat()->renderPipeline);
                    mainCommandBuffer->BindBuffer(transientBuffer, 1, transientOffset);
                    mainCommandBuffer->SetVertexBuffer(screenTriVerts);
                    mainCommandBuffer->Draw(3);
                    mainCommandBuffer->EndRenderDebugMarker();
                    mainCommandBuffer->EndRendering();
                }


				// apply transparency
#if 0
				transparencyApplyPass->SetAttachmentTexture(0, target.lightingTexture->GetDefaultView());

				mainCommandBuffer->BeginRenderDebugMarker("Apply All Transparency");
				mainCommandBuffer->BeginRendering(transparencyApplyPass);

				mainCommandBuffer->BindRenderPipeline(transparencyApplyPipeline);
				mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
				mainCommandBuffer->SetFragmentTexture(target.transparencyAccumulation->GetDefaultView(), 1);
				mainCommandBuffer->SetFragmentTexture(target.transparencyRevealage->GetDefaultView(), 2);
				mainCommandBuffer->SetVertexBuffer(screenTriVerts);

				LightToFBUBO transparencyUBO{
					.viewRect = {renderArea.offset[0], renderArea.offset[1], renderArea.extent[0], renderArea.extent[1]}
				};
				mainCommandBuffer->SetFragmentBytes(transparencyUBO, 0);
				mainCommandBuffer->Draw(3);

				mainCommandBuffer->EndRendering();
				mainCommandBuffer->EndRenderDebugMarker();
#endif

                // afterwards render the post processing effects
				RVE_PROFILE_SECTION(postfx, "Encode Post Processing Effects");
                uint32_t totalPostFXRendered = 0;
                RGL::TextureView currentInput = target.lightingTexture->GetDefaultView();
                RGL::TextureView altInput = target.lightingScratchTexture->GetDefaultView();
                
                for(const auto& effect : camData.postProcessingEffects->effects){
                    if (!effect->enabled){
                        continue;
                    }
					
                    effect->Preamble({ int(fullSizeViewport.width), int(fullSizeViewport.height) });
                    for(const auto pass : effect->passes){
						BasePushConstantUBO baseUbo{
							.dim = {fullSizeViewport.width, fullSizeViewport.height}
						};
						bool isUsingFinalOutput = pass->outputConfiguration == PostProcessOutput::EngineColor;

						auto activePass = pass->clearOutputBeforeRendering ? postProcessRenderPassClear : postProcessRenderPass;

						if (isUsingFinalOutput) {
							activePass->SetAttachmentTexture(0, altInput);
						}
						else {
							activePass->SetAttachmentTexture(0, pass->outputBinding);
							auto size = pass->GetUserDefinedOutputSize();
							baseUbo.dim = { size.width, size.height };
						}
                        mainCommandBuffer->BeginRendering(activePass);
                        mainCommandBuffer->BindRenderPipeline(pass->GetEffect()->GetPipeline());
						mainCommandBuffer->SetViewport({
							.x = 0,
							.y = 0,
							.width = float(baseUbo.dim.x),
							.height = float(baseUbo.dim.y),
						});
						mainCommandBuffer->SetScissor({
							.offset = {
								0,0
							},
							.extent = {
								uint32_t(baseUbo.dim.x), 
								uint32_t(baseUbo.dim.y)
							}
						});
						{
							uint32_t index = 0;
							for (const auto& input : pass->GetinputConfiguration()) {
								if (input == PostProcessTextureInput::EngineColor) {
									mainCommandBuffer->SetFragmentTexture(currentInput, index);
								}
								else if (input == PostProcessTextureInput::UserDefined) {
									auto img = pass->inputBindings.at(index);
									mainCommandBuffer->SetFragmentTexture(img, index);
								}
								index++;
							}
						}
						{
							uint32_t index = 0;
							for (const auto sampler : pass->inputSamplerBindings) {
								if (sampler != nullptr) {
									mainCommandBuffer->SetFragmentSampler(sampler, index);
								}
								index++;
							}
						}
						
                        mainCommandBuffer->SetVertexBuffer(screenTriVerts);
                        
                        // push constants
                        std::array<std::byte, 128> pushConstants{};
                        memcpy(pushConstants.data(), &baseUbo, sizeof(baseUbo));
                        auto userPC = pass->GetPushConstantData();
                        memcpy(pushConstants.data() + sizeof(baseUbo), userPC.data(), userPC.size());
                        mainCommandBuffer->SetFragmentBytes({pushConstants.data(), userPC.size() + sizeof(baseUbo)}, 0);
                        mainCommandBuffer->Draw(3);
                        
                        mainCommandBuffer->EndRendering();
						if (isUsingFinalOutput) {
							std::swap(currentInput, altInput);
							totalPostFXRendered++;
						}
                    }
                }
                
				RVE_PROFILE_SECTION_END(postfx);
                auto blitSource = totalPostFXRendered % 2 == 0 ? target.lightingTexture->GetDefaultView() : target.lightingScratchTexture->GetDefaultView();
                
				// the final on-screen render pass
// contains the results of the previous stages, as well as the UI, skybox and any debugging primitives
				
				glm::ivec4 viewRect {0, 0, nextImgSize.width, nextImgSize.height};

				LightToFBUBO fbubo{
					.viewRect = viewRect
				};

				mainCommandBuffer->BeginRendering(finalRenderPass);
				mainCommandBuffer->BeginRenderDebugMarker("Blit");
				// start with the results of lighting
				mainCommandBuffer->BindRenderPipeline(lightToFBRenderPipeline);
				mainCommandBuffer->SetViewport(fullSizeViewport);
				mainCommandBuffer->SetScissor(fullSizeScissor);
				mainCommandBuffer->SetVertexBuffer(screenTriVerts);
				mainCommandBuffer->SetVertexBytes(fbubo, 0);
				mainCommandBuffer->SetFragmentBytes(fbubo, 0);
				mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
				mainCommandBuffer->SetFragmentTexture(blitSource, 1);
				mainCommandBuffer->Draw(3);
                mainCommandBuffer->EndRendering();

                mainCommandBuffer->BeginRendering(finalRenderPass);
				
				RVE_PROFILE_SECTION(gui, "Encode GUI");
				mainCommandBuffer->BeginRenderDebugMarker("GUI");
				worldOwning->Filter([](GUIComponent& gui) {
					gui.Render();	// kicks off commands for rendering UI
				});
#ifndef NDEBUG
				if (debuggerContext) {
					auto& dbg = *debuggerContext;
					dbg.SetDimensions(view.pixelDimensions.width, view.pixelDimensions.height);
					dbg.SetDPIScale(guiScaleFactor);
					dbg.Update();
					dbg.Render();
				}
				mainCommandBuffer->EndRenderDebugMarker();
#endif
				RVE_PROFILE_SECTION_END(gui);
#ifndef NDEBUG
				// process debug shapes
				RVE_PROFILE_SECTION(debugShapes, "Encode Debug Navigation");
				mainCommandBuffer->BeginRenderDebugMarker("Debug Navigation Mesh");
				currentNavState.viewProj = camData.viewProj;
				worldOwning->FilterPolymorphic([this](PolymorphicGetResult<IDebugRenderable, World::PolymorphicIndirection> dbg, const PolymorphicGetResult<Transform, World::PolymorphicIndirection> transform) {
					for (int i = 0; i < dbg.size(); i++) {
						auto& ptr = dbg[i];
						if (ptr.debugEnabled) {
							currentNavState.model = transform[0].GetWorldMatrix();
							ptr.DebugDraw(dbgdraw, transform[0]);
						}
					}
					});
				mainCommandBuffer->EndRenderDebugMarker();
				RVE_PROFILE_SECTION_END(debugShapes);

				mainCommandBuffer->BeginRenderDebugMarker("Debug Wireframes");
				Im3d::AppData& data = Im3d::GetAppData();

				struct DrawListMetadata {
					uint32_t nverts = 0;
				} im3dMeta;

                const auto& im3dcontext = Im3d::GetContext();
                Im3d::EndFrame();
				if (im3dcontext.getDrawListCount() > 0) {
					RVE_PROFILE_SECTION(wireframes, "Encode Debug Wireframes");
					for (int i = 0; i < im3dcontext.getDrawListCount(); i++) {
						im3dMeta.nverts += im3dcontext.getDrawLists()[i].m_vertexCount;
					}

					// resize buffer
					if (im3dMeta.nverts > debugRenderBufferSize) {
						debugRenderBufferUpload = device->CreateBuffer({
							im3dMeta.nverts,
							{.VertexBuffer = true},
							sizeof(Im3d::VertexData),
							RGL::BufferAccess::Shared,
							});
						debugRenderBufferSize = im3dMeta.nverts;
					}

					data.m_appData = (void*)&camData.viewProj;
					debugRenderBufferOffset = 0;
					data.drawCallback = [](const Im3d::DrawList& list) {
						GetApp()->GetRenderEngine().DebugRender(list);
						};

					mainCommandBuffer->SetViewport(fullSizeViewport);
					mainCommandBuffer->SetScissor(fullSizeScissor);
					Im3d::GetContext().draw();
					RVE_PROFILE_SECTION_END(wireframes);
				}
				mainCommandBuffer->EndRenderDebugMarker();

				Im3d::NewFrame();
				mainCommandBuffer->EndRenderDebugMarker();
#endif
				mainCommandBuffer->EndRendering();
			};

			auto doPassWithCamData = [this,&target,&nextImgSize](auto&& camdata, auto&& function) {
				const auto camPos = camdata.camPos;
				const auto viewportOverride = camdata.viewportOverride;

				RGL::Rect renderArea{
					.offset = { int32_t(nextImgSize.width * viewportOverride.originFactor.x),int32_t(nextImgSize.height * viewportOverride.originFactor.y) },
						.extent = { uint32_t(nextImgSize.width * viewportOverride.sizeFactor.x), uint32_t(nextImgSize.height * viewportOverride.sizeFactor.x) },
				};

				RGL::Viewport fullSizeViewport{
					.x = float(renderArea.offset[0]),
						.y = float(renderArea.offset[1]),
						.width = static_cast<float>(renderArea.extent[0]),
						.height = static_cast<float>(renderArea.extent[1]),
				};

				RGL::Rect fullSizeScissor{
					.offset = { 0,0 },
						.extent = { uint32_t(nextImgSize.width), uint32_t(nextImgSize.height) }
				};

				function(camdata, fullSizeViewport, fullSizeScissor, renderArea);
			};
            
			auto generatePyramid = [this](const DepthPyramid& depthPyramid, RGLTexturePtr depthStencil) {
#ifndef OCCLUSION_CULLING_UNAVAILABLE
				// build the depth pyramid using the depth data from the previous frame
				depthPyramidCopyPass->SetAttachmentTexture(0, depthPyramid.pyramidTexture->GetViewForMip(0));
				mainCommandBuffer->BeginRendering(depthPyramidCopyPass);
				mainCommandBuffer->BeginRenderDebugMarker("First copy of depth pyramid");
				mainCommandBuffer->BindRenderPipeline(depthPyramidCopyPipeline);
				mainCommandBuffer->SetViewport({ 0,0,float(depthPyramid.dim) ,float(depthPyramid.dim) });
				mainCommandBuffer->SetScissor({ 0,0,depthPyramid.dim,depthPyramid.dim });
				PyramidCopyUBO pubo{ .size = depthPyramid.dim };
				mainCommandBuffer->SetFragmentBytes(pubo, 0);
				mainCommandBuffer->SetFragmentTexture(depthStencil->GetDefaultView(), 0);
				mainCommandBuffer->SetFragmentSampler(depthPyramidSampler, 1);
				mainCommandBuffer->SetVertexBuffer(screenTriVerts);
				mainCommandBuffer->Draw(3);
				mainCommandBuffer->EndRenderDebugMarker();
				mainCommandBuffer->EndRendering();

				mainCommandBuffer->BeginCompute(depthPyramidPipeline);
				mainCommandBuffer->BeginComputeDebugMarker("Build depth pyramid");

				{
					float dim = depthPyramid.dim;
					for (int i = 0; i < depthPyramid.numLevels - 1; i++) {
						auto fromTex = depthPyramid.pyramidTexture->GetViewForMip(i);
						auto toTex = depthPyramid.pyramidTexture->GetViewForMip(i + 1);
						mainCommandBuffer->SetComputeTexture(toTex, 0);
						mainCommandBuffer->SetComputeTexture(fromTex, 1);
						mainCommandBuffer->SetComputeSampler(depthPyramidSampler, 2);

						dim /= 2.0;

						mainCommandBuffer->DispatchCompute(std::ceil(dim / 32.f), std::ceil(dim / 32.f), 1, 32, 32, 1);
					}
				}
				mainCommandBuffer->EndComputeDebugMarker();
				mainCommandBuffer->EndCompute();
#endif
			};
			generatePyramid(target.depthPyramid, target.depthStencil);

			// also generate the pyramids for the shadow lights
			auto genPyramidForLight = [&generatePyramid,&worldOwning](auto&& lightStore, auto* lightType, uint32_t nMaps, auto&& getMapDataForIndex) -> void {
				for (uint32_t i = 0; i < lightStore.uploadData.DenseSize(); i++) {
					const auto& light = lightStore.uploadData.GetDense()[i];
					auto sparseIdx = lightStore.uploadData.GetSparseIndexForDense(i);
					auto owner = Entity(sparseIdx, worldOwning.get());

					using LightType = std::remove_pointer_t<decltype(lightType)>;

					auto& origLight = Entity(owner).GetComponent<LightType>();
					if (origLight.CastsShadows()) {
						for (uint32_t i = 0; i < nMaps; i++) {
							const auto mapData = getMapDataForIndex(i, origLight);
							generatePyramid(mapData.pyramid, mapData.shadowMap);
						}
					}
				}
			};
			mainCommandBuffer->BeginRenderDebugMarker("Light depth pyramids");
			{
				DirectionalLight* ptr = nullptr;
				genPyramidForLight(worldOwning->renderData.directionalLightData, ptr, 1, [](uint32_t index, auto&& origLight) {
					return origLight.GetShadowMap();
				});
			}
			{
				SpotLight* ptr = nullptr;
				genPyramidForLight(worldOwning->renderData.spotLightData, ptr,1, [](uint32_t index, auto&& origLight) {
					return origLight.GetShadowMap();
				});
			}
			{
				PointLight* ptr = nullptr;
				genPyramidForLight(worldOwning->renderData.pointLightData, ptr, 6, [](uint32_t index, auto&& origLight) {
					struct ReturnData {
						DepthPyramid pyramid;
						RGLTexturePtr shadowMap;
					};
					return ReturnData{origLight.shadowData.cubePyramids[index], origLight.shadowData.cubeShadowmaps[index]};
				});
			}

			mainCommandBuffer->EndRenderDebugMarker();
		

			// lit pass
			litRenderPass->SetAttachmentTexture(0, target.lightingTexture->GetDefaultView());
			litRenderPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());

			litClearRenderPass->SetAttachmentTexture(0, target.lightingTexture->GetDefaultView());
			litClearRenderPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());

			mainCommandBuffer->BeginRenderDebugMarker("Lit Pass Opaque");

			mainCommandBuffer->BeginRendering(litClearRenderPass);
			mainCommandBuffer->EndRendering();
			
			RVE_PROFILE_SECTION(lit, "Encode Lit Pass Opaque");
			for (const auto& camdata : view.camDatas) {
				doPassWithCamData(camdata, renderLitPass);
			}
			mainCommandBuffer->EndRenderDebugMarker();
			RVE_PROFILE_SECTION_END(lit);

#if 0
			transparentClearPass->SetAttachmentTexture(0, target.transparencyAccumulation->GetDefaultView());
			transparentClearPass->SetAttachmentTexture(1, target.transparencyRevealage->GetDefaultView());

			mainCommandBuffer->BeginRenderDebugMarker("Lit Pass Transparent");
			mainCommandBuffer->BeginRendering(transparentClearPass);
			mainCommandBuffer->EndRendering();
#endif

            litTransparentPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());
			RVE_PROFILE_SECTION(littrans, "Encode Lit Pass Transparent");
			for (const auto& camdata : view.camDatas) {
				doPassWithCamData(camdata, renderLitPassTransparent);
			}
			mainCommandBuffer->EndRenderDebugMarker();
			RVE_PROFILE_SECTION_END(littrans);
            
            if (VideoSettings.ssao){

				stackarray(offsets, uint32_t, view.camDatas.size());
				uint32_t offset_index = 0;

				auto renderSSAOPass = [this, &target, &nextImgSize, &worldOwning, &offsets, &offset_index](auto&& camData, auto&& fullsizeViewport, auto&& fullSizeScissor, auto&& renderArea) {

					ssaoUBO pushConstants{
						.viewProj = camData.viewProj,
						.viewRect = {0,0, nextImgSize.width, nextImgSize.height},
						.viewRegion = {renderArea.offset[0], renderArea.offset[1], renderArea.extent[0], renderArea.extent[1]}
					};

					mainCommandBuffer->SetViewport(fullsizeViewport);
					mainCommandBuffer->SetScissor(renderArea);

					mainCommandBuffer->SetVertexBuffer(screenTriVerts);
					mainCommandBuffer->SetFragmentBytes(pushConstants,0);
					mainCommandBuffer->BindBuffer(transientBuffer, 7, offsets[offset_index]);
					mainCommandBuffer->Draw(3);
				};

				ssaoPass->SetAttachmentTexture(0, view.collection.ssaoTexture->GetDefaultView());

				// because vulkan doesn't allow vkcmdcopybuffer inside of a render pass for some reason
				{
					uint32_t i = 0;
					for (const auto& camdata : view.camDatas) {
						struct ssaoSpill {
							glm::mat4 projOnly;
							glm::mat4 invProj;
							glm::mat4 viewOnly;
						} constants
						{
							.projOnly = camdata.projOnly,
							.invProj = glm::inverse(camdata.projOnly),
							.viewOnly = camdata.viewOnly
						};
						auto offset = WriteTransient(constants);
						offsets[i] = offset;
						i++;
					}
				}

				mainCommandBuffer->BeginRendering(ssaoPass);
				mainCommandBuffer->BindRenderPipeline(ssaoPipeline);
				mainCommandBuffer->BeginRenderDebugMarker("SSAO");
				mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
				mainCommandBuffer->SetFragmentTexture(view.collection.depthStencil->GetDefaultView(), 1);
				mainCommandBuffer->BindBuffer(ssaoSamplesBuffer, 8);

                for (const auto& camdata : view.camDatas) {
					doPassWithCamData(camdata, renderSSAOPass);
					offset_index++;
                }
				mainCommandBuffer->EndRendering();
				mainCommandBuffer->EndRenderDebugMarker();
            }

			
			// final render pass
			RVE_PROFILE_SECTION(forward, "Render Encode Forward Pass");
			finalRenderPass->SetAttachmentTexture(0, target.finalFramebuffer->GetDefaultView());
			finalRenderPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());

			finalClearRenderPass->SetAttachmentTexture(0, target.finalFramebuffer->GetDefaultView());
			finalClearRenderPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());

			mainCommandBuffer->BeginRenderDebugMarker("Forward Pass");

			mainCommandBuffer->BeginRendering(finalClearRenderPass);
			mainCommandBuffer->EndRendering();

			for (const auto& camdata : view.camDatas) {
				doPassWithCamData(camdata, renderFinalPass);
			}
			mainCommandBuffer->EndRenderDebugMarker();
			RVE_PROFILE_SECTION_END(forward);

		}
		RVE_PROFILE_SECTION_END(allViews);
		mainCommandBuffer->End();

		return mainCommandBuffer;
	}
}

void RavEngine::RenderEngine::DebugRender(const Im3d::DrawList& drawList)
{
#ifndef NDEBUG
	switch (drawList.m_primType) {
	case Im3d::DrawPrimitive_Triangles:
		mainCommandBuffer->BindRenderPipeline(im3dTriangleRenderPipeline);
		break;
	case Im3d::DrawPrimitive_Lines:
		mainCommandBuffer->BindRenderPipeline(im3dLineRenderPipeline);
		break;
	case Im3d::DrawPrimitive_Points:
		mainCommandBuffer->BindRenderPipeline(im3dPointRenderPipeline);
		break;
	default:
		Debug::Fatal("Invalid Im3d state");
		break;
	}
	//perform drawing here
	const Im3d::VertexData* vertexdata = drawList.m_vertexData;
	const auto nverts = drawList.m_vertexCount;

	const auto dataSize = nverts * sizeof(Im3d::VertexData);
	debugRenderBufferUpload->UpdateBufferData({ vertexdata,  dataSize}, debugRenderBufferOffset);


	auto viewProj = *static_cast<glm::mat4*>(Im3d::GetAppData().m_appData);
    
    DebugUBO ubo{
        .viewProj = viewProj
    };

	mainCommandBuffer->SetVertexBytes(ubo,0);
	mainCommandBuffer->SetVertexBuffer(debugRenderBufferUpload, {.offsetIntoBuffer = debugRenderBufferOffset});
	mainCommandBuffer->Draw(nverts);

	debugRenderBufferOffset += dataSize;


#endif

}
#endif
