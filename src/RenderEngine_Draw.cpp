#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1
#include "RenderEngine.hpp"
#include <RGL/CommandBuffer.hpp>
#include <RGL/Swapchain.hpp>
#include <RGL/RenderPass.hpp>
#include "World.hpp"
#include "CameraComponent.hpp"
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

namespace RavEngine {

#ifndef NDEBUG
	static DebugDrawer dbgdraw;	//for rendering debug primitives
#endif

	/**
 Render one frame using the current state of every object in the world
 */
	void RenderEngine::Draw(Ref<RavEngine::World> worldOwning) {
		auto start = std::chrono::high_resolution_clock::now();
		transientOffset = 0;

		// queue up the next swapchain image as soon as possible, 
		// it will become avaiable in the background
		RGL::SwapchainPresentConfig presentConfig{
		};
		swapchain->GetNextImage(&presentConfig.imageIndex);

		// execute when render fence says its ok
		// did we get the swapchain image yet? if not, block until we do

		swapchainFence->Wait();
		swapchainFence->Reset();
		DestroyUnusedResources();
		mainCommandBuffer->Reset();
		mainCommandBuffer->Begin();

		auto nextimg = swapchain->ImageAtIndex(presentConfig.imageIndex);
		auto nextImgSize = nextimg->GetSize();

		auto allCameras = worldOwning->GetAllComponentsOfType<CameraComponent>();
		if (!allCameras)
		{
			Debug::Fatal("Cannot render: World does not have a camera!");
		}
		auto& cam = worldOwning->GetComponent<CameraComponent>();
		const auto viewproj = cam.GenerateProjectionMatrix(nextImgSize.width, nextImgSize.height) * cam.GenerateViewMatrix();
		const auto invviewproj = glm::inverse(viewproj);
		const auto camPos = cam.GetOwner().GetTransform().GetWorldPosition();
		auto worldTransformBuffer = worldOwning->renderData->worldTransforms.buffer;

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

			for (auto& [materialInstance, drawcommand] : worldOwning->renderData->skinnedMeshRenderData) {
				uint32_t totalEntitiesForThisCommand = 0;
				for (auto& command : drawcommand.commands) {
					auto subCommandEntityCount = command.entities.DenseSize();
					totalObjectsToSkin += subCommandEntityCount;
					totalEntitiesForThisCommand += subCommandEntityCount;

					if (auto mesh = command.mesh.lock()) {
						totalVertsToSkin += mesh->GetNumVerts();
					}

					if (auto skeleton = command.skeleton.lock()) {
						totalJointsToSkin += skeleton->GetSkeleton()->num_joints();
					}
				}

				resizeSkeletonBuffer(drawcommand.indirectBuffer, sizeof(RGL::IndirectIndexedCommand), totalEntitiesForThisCommand, { .StorageBuffer = true, .IndirectBuffer = true }, RGL::BufferAccess::Private, { .debugName = "Skeleton per-material IndirectBuffer" });
				//TODO: skinned meshes do not support LOD groups
				resizeSkeletonBuffer(drawcommand.cullingBuffer, sizeof(entity_t), totalEntitiesForThisCommand, { .StorageBuffer = true, .VertexBuffer = true }, RGL::BufferAccess::Private, { .debugName = "Skeleton per-material culingBuffer" });
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
			SkinningPrepareUBO ubo;
			uint32_t baseInstance = 0;
			for (auto& [materialInstance, drawcommand] : worldOwning->renderData->skinnedMeshRenderData) {

				mainCommandBuffer->BindComputeBuffer(drawcommand.indirectBuffer, 0, 0);
				for (auto& command : drawcommand.commands) {
					const auto objectCount = command.entities.DenseSize();
					const auto mesh = command.mesh.lock();
					const auto vertexCount = mesh->GetNumVerts();

					ubo.nVerticesInThisMesh = vertexCount;
					ubo.nTotalObjects = objectCount;
					ubo.indexBufferOffset = mesh->meshAllocation.indexRange->start / sizeof(uint32_t);
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
		auto cullSkeletalMeshes = [this,&worldTransformBuffer,&worldOwning](matrix4 viewproj) {
			// the culling shader will decide for each draw if the draw should exist (and set its instance count to 1 from 0).

			mainCommandBuffer->BeginComputeDebugMarker("Cull Skinned Meshes");
			mainCommandBuffer->BeginCompute(defaultCullingComputePipeline);
			mainCommandBuffer->BindComputeBuffer(worldTransformBuffer, 1);
			for (auto& [materialInstance, drawcommand] : worldOwning->renderData->skinnedMeshRenderData) {
				CullingUBO cubo{
					.viewProj = viewproj,
					.indirectBufferOffset = 0,
				};
				for (auto& command : drawcommand.commands) {
					mainCommandBuffer->BindComputeBuffer(drawcommand.cullingBuffer, 2);
					mainCommandBuffer->BindComputeBuffer(drawcommand.indirectBuffer, 3);

					if (auto mesh = command.mesh.lock()) {
						uint32_t lodsForThisMesh = 1;	// TODO: skinned meshes do not support LOD groups 

						cubo.numObjects = command.entities.DenseSize();
						mainCommandBuffer->BindComputeBuffer(command.entities.GetDense().get_underlying().buffer, 0);
						mainCommandBuffer->SetComputeBytes(cubo, 0);
						mainCommandBuffer->DispatchCompute(std::ceil(cubo.numObjects / 64.f), 1, 1, 64, 1, 1);
						cubo.indirectBufferOffset += lodsForThisMesh;
						cubo.cullingBufferOffset += lodsForThisMesh * command.entities.DenseSize();
					}
				}

			}
			mainCommandBuffer->EndComputeDebugMarker();
			mainCommandBuffer->EndCompute();
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
			for (auto& [materialInstance, drawcommand] : worldOwning->renderData->skinnedMeshRenderData) {
				for (auto& command : drawcommand.commands) {
					auto skeleton = command.skeleton.lock();
					auto mesh = command.mesh.lock();
					auto& entities = command.entities;
					mainCommandBuffer->BindComputeBuffer(mesh->GetWeightsBuffer(), 3);

					subo.numObjects = command.entities.DenseSize();
					subo.numVertices = mesh->GetNumVerts();
					subo.numBones = skeleton->GetSkeleton()->num_joints();
					subo.vertexReadOffset = mesh->meshAllocation.vertRange->start / sizeof(VertexNormalUV);

					// write joint transform matrices into buffer and update uniform offset
					for (const auto& ownerid : command.entities.reverse_map) {
						auto& animator = worldOwning->GetComponent<AnimatorComponent>(ownerid);
						const auto& skinningMats = animator.GetSkinningMats();
						std::copy(skinningMats.begin(), skinningMats.end(), matbufMem.begin() + subo.boneReadOffset);
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
		
		// don't do operations if there's nothing to skin
		// these operations run once per frame since the results
		// are the same for all future passes
		if (skeletalPrepareResult.skeletalMeshesExist) {
			poseSkeletalMeshes();

			prepareSkeletalCullingBuffer();
		}

		// render all the static meshes
		deferredRenderPass->SetAttachmentTexture(0, diffuseTexture.get());
		deferredRenderPass->SetAttachmentTexture(1, normalTexture.get());
		deferredRenderPass->SetDepthAttachmentTexture(depthStencil.get());

		mainCommandBuffer->SetViewport({
			.width = static_cast<float>(nextImgSize.width),
			.height = static_cast<float>(nextImgSize.height),
		});
		mainCommandBuffer->SetScissor({
			.extent = {nextImgSize.width, nextImgSize.height}
		});

		mainCommandBuffer->BeginRenderDebugMarker("Deferred Pass");

		mainCommandBuffer->TransitionResources({
			{
				.texture = diffuseTexture.get(),
				.from = RGL::ResourceLayout::ShaderReadOnlyOptimal,
				.to = RGL::ResourceLayout::ColorAttachmentOptimal,
			},
			{
				.texture = normalTexture.get(),
				.from = RGL::ResourceLayout::ShaderReadOnlyOptimal,
				.to = RGL::ResourceLayout::ColorAttachmentOptimal,
			},
			{
				.texture = depthStencil.get(),
				.from = RGL::ResourceLayout::DepthReadOnlyOptimal,
				.to = RGL::ResourceLayout::DepthAttachmentOptimal
			},
			/*
			{
				.texture = shadowTexture.get(),
				.from = RGL::ResourceLayout::DepthReadOnlyOptimal,
				.to = RGL::ResourceLayout::DepthAttachmentOptimal,
			}*/
			}, RGL::TransitionPosition::Top
		);


		auto renderFromPerspective = [this,&worldTransformBuffer,&worldOwning,&skeletalPrepareResult,&cullSkeletalMeshes](matrix4 viewproj, RGLRenderPassPtr renderPass, auto&& pipelineSelectorFunction, RGL::Dimension viewportScissorSize) {
			
			auto cullTheRenderData = [this, &viewproj, &worldTransformBuffer](auto&& renderData) {
				for (auto& [materialInstance, drawcommand] : renderData) {
					//prepass: get number of LODs and entities
					uint32_t numLODs = 0, numEntities = 0;
					for (const auto& command : drawcommand.commands) {
						if (auto mesh = command.mesh.lock()) {
							numLODs += mesh->GetNumLods();
							numEntities += command.entities.DenseSize();
						}
					}

					auto reallocBuffer = [this](RGLBufferPtr& buffer, uint32_t size_count, uint32_t stride, RGL::BufferAccess access, RGL::BufferConfig::Type type, RGL::BufferFlags flags) {
						if (buffer == nullptr || buffer->getBufferSize() < size_count * stride) {
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
									initData = {
										.indexCount = uint32_t(mesh->totalIndices),
										.instanceCount = 0,
										.indexStart = uint32_t(mesh->meshAllocation.indexRange->start / sizeof(uint32_t)),
										.baseVertex = uint32_t(mesh->meshAllocation.vertRange->start / sizeof(VertexNormalUV)),
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

					mainCommandBuffer->SetResourceBarrier({
						.buffers = {drawcommand.indirectBuffer}
						});

					mainCommandBuffer->BeginCompute(defaultCullingComputePipeline);
					mainCommandBuffer->BindComputeBuffer(worldTransformBuffer, 1);
					CullingUBO cubo{
						.viewProj = viewproj,
						.indirectBufferOffset = 0,
					};
					for (auto& command : drawcommand.commands) {
						mainCommandBuffer->BindComputeBuffer(drawcommand.cullingBuffer, 2);
						mainCommandBuffer->BindComputeBuffer(drawcommand.indirectBuffer, 3);

						if (auto mesh = command.mesh.lock()) {
							uint32_t lodsForThisMesh = mesh->GetNumLods();

							cubo.numObjects = command.entities.DenseSize();
							mainCommandBuffer->BindComputeBuffer(command.entities.GetDense().get_underlying().buffer, 0);
							mainCommandBuffer->SetComputeBytes(cubo, 0);
							mainCommandBuffer->DispatchCompute(std::ceil(cubo.numObjects / 64.f), 1, 1, 64, 1, 1);
							cubo.indirectBufferOffset += lodsForThisMesh;
							cubo.cullingBufferOffset += lodsForThisMesh * command.entities.DenseSize();
						}
					}
					mainCommandBuffer->EndCompute();
					mainCommandBuffer->SetResourceBarrier({ .buffers = {drawcommand.cullingBuffer, drawcommand.indirectBuffer} });
				}
			};
			auto renderTheRenderData = [this, &viewproj, &worldTransformBuffer, &pipelineSelectorFunction,&viewportScissorSize](auto&& renderData, RGLBufferPtr vertexBuffer) {
				// do static meshes
				mainCommandBuffer->SetViewport({
					.width = static_cast<float>(viewportScissorSize.width),
					.height = static_cast<float>(viewportScissorSize.height),
					});
				mainCommandBuffer->SetScissor({
					.extent = {viewportScissorSize.width, viewportScissorSize.height}
				});
				mainCommandBuffer->SetVertexBuffer(vertexBuffer);
				mainCommandBuffer->SetIndexBuffer(sharedIndexBuffer);
				for (auto& [materialInstance, drawcommand] : renderData) {
					// bind the pipeline
					auto pipeline = pipelineSelectorFunction(materialInstance->GetMat());
					mainCommandBuffer->BindRenderPipeline(pipeline);

					// set push constant data
					auto pushConstantData = materialInstance->GetPushConstantData();

					auto pushConstantTotalSize = sizeof(viewproj) + pushConstantData.size();

					stackarray(totalPushConstantBytes, std::byte, pushConstantTotalSize);
					std::memcpy(totalPushConstantBytes, &viewproj, sizeof(viewproj));
					if (pushConstantData.size() > 0 && pushConstantData.data() != nullptr) {
						std::memcpy(totalPushConstantBytes + sizeof(viewproj), pushConstantData.data(), pushConstantData.size());
					}

					mainCommandBuffer->SetVertexBytes({ totalPushConstantBytes ,pushConstantTotalSize }, 0);
					mainCommandBuffer->SetFragmentBytes({ totalPushConstantBytes ,pushConstantTotalSize }, 0);

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
							mainCommandBuffer->SetFragmentTexture(texture->GetRHITexturePointer().get(), i);
						}
					}

					// bind the culling buffer and the transform buffer
					mainCommandBuffer->SetVertexBuffer(drawcommand.cullingBuffer, { .bindingPosition = 1 });
					mainCommandBuffer->BindBuffer(worldTransformBuffer, 2);

					// do the indirect command
					mainCommandBuffer->ExecuteIndirectIndexed({
						.indirectBuffer = drawcommand.indirectBuffer,
						.nDraws = uint32_t(drawcommand.indirectBuffer->getBufferSize() / sizeof(RGL::IndirectIndexedCommand))	// the number of structs in the buffer
						});
				}
			};

			// do culling operations
			mainCommandBuffer->BeginComputeDebugMarker("Cull Static Meshes");
			cullTheRenderData(worldOwning->renderData->staticMeshRenderData);
			mainCommandBuffer->EndComputeDebugMarker();
			if (skeletalPrepareResult.skeletalMeshesExist) {
				cullSkeletalMeshes(viewproj);
			}

			if (sharedSkinnedMeshVertexBuffer) {
				mainCommandBuffer->SetResourceBarrier({
					.buffers = {
						sharedSkinnedMeshVertexBuffer,
					}
				});
			}

			// do rendering operations
			mainCommandBuffer->BeginRendering(renderPass);
			mainCommandBuffer->BeginRenderDebugMarker("Render Static Meshes");
			renderTheRenderData(worldOwning->renderData->staticMeshRenderData, sharedVertexBuffer);
			mainCommandBuffer->EndRenderDebugMarker();
			if (skeletalPrepareResult.skeletalMeshesExist) {
				mainCommandBuffer->BeginRenderDebugMarker("Render Skinned Meshes");
				renderTheRenderData(worldOwning->renderData->skinnedMeshRenderData, sharedSkinnedMeshVertexBuffer);
				mainCommandBuffer->EndRenderDebugMarker();
			}
			mainCommandBuffer->EndRendering();
		};

		renderFromPerspective(viewproj, deferredRenderPass, [](Ref<Material>&& mat) {
			return mat->GetMainRenderPipeline();
		}, nextImgSize);

		mainCommandBuffer->TransitionResources({
			{
				.texture = diffuseTexture.get(),
				.from = RGL::ResourceLayout::ColorAttachmentOptimal,
				.to = RGL::ResourceLayout::ShaderReadOnlyOptimal,
			},
			{
				.texture = normalTexture.get(),
				.from = RGL::ResourceLayout::ColorAttachmentOptimal,
				.to = RGL::ResourceLayout::ShaderReadOnlyOptimal,
			},
			{
				.texture = lightingTexture.get(),
				.from = RGL::ResourceLayout::ShaderReadOnlyOptimal,
				.to = RGL::ResourceLayout::ColorAttachmentOptimal,
			},
			{
				.texture = depthStencil.get(),
				.from = RGL::ResourceLayout::DepthAttachmentOptimal,
				.to = RGL::ResourceLayout::DepthReadOnlyOptimal,
			}
			}, RGL::TransitionPosition::Top
		);

		// do lighting pass
		AmbientLightUBO ambientUBO{
			.viewRect = {0,0,nextImgSize.width,nextImgSize.height}
		};

		LightingUBO lightUBO{
			.viewProj = viewproj,
			.viewRect = {0,0,nextImgSize.width,nextImgSize.height}
		};
		PointLightUBO pointLightUBO{
			.viewProj = viewproj,
			.invViewProj =invviewproj,
			.viewRect = lightUBO.viewRect
		};
		lightingRenderPass->SetDepthAttachmentTexture(depthStencil.get());
		lightingRenderPass->SetAttachmentTexture(0, lightingTexture.get());

		mainCommandBuffer->SetRenderPipelineBarrier({
			.Fragment = true
		});

		mainCommandBuffer->BeginRenderDebugMarker("Lighting Pass");
		// ambient lights
        if (worldOwning->renderData->ambientLightData.DenseSize() > 0){
			mainCommandBuffer->BeginRendering(lightingRenderPass);
			mainCommandBuffer->BeginRenderDebugMarker("Render Ambient Lights");
            mainCommandBuffer->BindRenderPipeline(ambientLightRenderPipeline);
			mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
            mainCommandBuffer->SetFragmentTexture(diffuseTexture.get(), 1);
            
            mainCommandBuffer->SetVertexBuffer(screenTriVerts);
            mainCommandBuffer->SetVertexBytes(ambientUBO, 0);
            mainCommandBuffer->SetFragmentBytes(ambientUBO, 0);
            mainCommandBuffer->SetVertexBuffer(worldOwning->renderData->ambientLightData.GetDense().get_underlying().buffer, {
                .bindingPosition = 1
            });
            mainCommandBuffer->Draw(3, {
                .nInstances = worldOwning->renderData->ambientLightData.DenseSize()
            });
			mainCommandBuffer->EndRenderDebugMarker();
			mainCommandBuffer->EndRendering();
        }

		// directional lights
        if (worldOwning->renderData->directionalLightData.DenseSize() > 0){
			// render shadows for directional lights
			shadowRenderPass->SetDepthAttachmentTexture(shadowTexture.get());
			mainCommandBuffer->BeginRenderDebugMarker("Render Directional Lights");
			auto& dirlightStore = worldOwning->renderData->directionalLightData;
			for (uint32_t i = 0; i < dirlightStore.DenseSize(); i++) {
				const auto& light = dirlightStore.GetDense()[i];
				auto dirvec = light.direction;

				struct  {
					glm::mat4 invViewProj;
					glm::mat4 lightViewProj;
				} dirlightExtras;

				constexpr auto lightArea = 40;

				auto lightProj = glm::ortho<float>(-lightArea, lightArea, -lightArea, lightArea, -100, 100);
				auto lightView = glm::lookAt(dirvec, { 0,0,0 }, { 0,1,0 });
				const vector3 reposVec{ std::round(-camPos.x), std::round(camPos.y), std::round(-camPos.z)};
				lightView = glm::translate(lightView, reposVec);
				auto lightSpaceMatrix = lightProj * lightView;

				mainCommandBuffer->TransitionResource(shadowTexture.get(), RGL::ResourceLayout::DepthReadOnlyOptimal, RGL::ResourceLayout::DepthAttachmentOptimal, RGL::TransitionPosition::Top);

				renderFromPerspective(lightSpaceMatrix, shadowRenderPass, [](Ref<Material>&& mat) {
					return mat->GetShadowRenderPipeline();
					}, { 2048,2048 });

				dirlightExtras.lightViewProj = lightSpaceMatrix;
				dirlightExtras.invViewProj = invviewproj;

				auto transientOffset = WriteTransient(dirlightExtras);

				mainCommandBuffer->TransitionResource(shadowTexture.get(), RGL::ResourceLayout::DepthAttachmentOptimal, RGL::ResourceLayout::DepthReadOnlyOptimal, RGL::TransitionPosition::Top);
				mainCommandBuffer->BeginRendering(lightingRenderPass);
                //reset viewport and scissor
                mainCommandBuffer->SetViewport({
                    .width = static_cast<float>(nextImgSize.width),
                    .height = static_cast<float>(nextImgSize.height),
                    });
                mainCommandBuffer->SetScissor({
                    .extent = {nextImgSize.width, nextImgSize.height}
                });
				mainCommandBuffer->BindRenderPipeline(dirLightRenderPipeline);
				mainCommandBuffer->SetFragmentSampler(textureSampler,0);
				mainCommandBuffer->SetFragmentSampler(shadowSampler, 1);

				mainCommandBuffer->SetFragmentTexture(diffuseTexture.get(), 2);
				mainCommandBuffer->SetFragmentTexture(normalTexture.get(), 3);
				mainCommandBuffer->SetFragmentTexture(depthStencil.get(), 4);
				mainCommandBuffer->SetFragmentTexture(shadowTexture.get(), 5);
				mainCommandBuffer->BindBuffer(transientBuffer, 8, transientOffset);
				mainCommandBuffer->SetVertexBuffer(screenTriVerts);
				mainCommandBuffer->SetVertexBytes(lightUBO, 0);
				mainCommandBuffer->SetFragmentBytes(lightUBO, 0);
				mainCommandBuffer->SetVertexBuffer(worldOwning->renderData->directionalLightData.GetDense().get_underlying().buffer, {
					.bindingPosition = 1,
					.offsetIntoBuffer = uint32_t(sizeof(World::DirLightUploadData) * i)
				});
				mainCommandBuffer->Draw(3, {
					.nInstances = 1
				});
				mainCommandBuffer->EndRenderDebugMarker();
				mainCommandBuffer->EndRendering();
			}

			//TODO: submit unshadowed dirlights
#if 0
			mainCommandBuffer->BeginRendering(lightingRenderPass);
            mainCommandBuffer->BindRenderPipeline(dirLightRenderPipeline);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, diffuseTexture.get(), 0);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, normalTexture.get(), 1);
            mainCommandBuffer->SetVertexBuffer(screenTriVerts);
            mainCommandBuffer->SetVertexBytes(lightUBO, 0);
            mainCommandBuffer->SetFragmentBytes(lightUBO, 0);
            mainCommandBuffer->SetVertexBuffer(worldOwning->renderData->directionalLightData.GetDense().get_underlying().buffer, {
                .bindingPosition = 1
            });
            mainCommandBuffer->Draw(3, {
                .nInstances = worldOwning->renderData->directionalLightData.DenseSize()
            });
			mainCommandBuffer->EndRenderDebugMarker();
			mainCommandBuffer->EndRendering();
#endif
        }

		// point lights
        if (worldOwning->renderData->pointLightData.DenseSize() > 0){
			mainCommandBuffer->BeginRendering(lightingRenderPass);
			mainCommandBuffer->BeginRenderDebugMarker("Render Point Lights");
            mainCommandBuffer->BindRenderPipeline(pointLightRenderPipeline);
			mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
            mainCommandBuffer->SetFragmentTexture(diffuseTexture.get(), 2);
            mainCommandBuffer->SetFragmentTexture(normalTexture.get(), 3);
            mainCommandBuffer->SetFragmentTexture(depthStencil.get(), 4);
            mainCommandBuffer->SetVertexBytes(pointLightUBO, 0);
            mainCommandBuffer->SetFragmentBytes(pointLightUBO, 0);
            mainCommandBuffer->SetVertexBuffer(pointLightVertexBuffer);
            mainCommandBuffer->SetIndexBuffer(pointLightIndexBuffer);
            mainCommandBuffer->SetVertexBuffer(worldOwning->renderData->pointLightData.GetDense().get_underlying().buffer, {
                .bindingPosition = 1
            });
            mainCommandBuffer->DrawIndexed(nPointLightIndices, {
                .nInstances = worldOwning->renderData->pointLightData.DenseSize()
            });
			mainCommandBuffer->EndRenderDebugMarker();
			mainCommandBuffer->EndRendering();
        }

		// spot lights
		if (worldOwning->renderData->spotLightData.DenseSize() > 0) {
			mainCommandBuffer->BeginRendering(lightingRenderPass);
			mainCommandBuffer->BeginRenderDebugMarker("Render Spot Lights");
			mainCommandBuffer->BindRenderPipeline(spotLightRenderPipeline);

			mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
			mainCommandBuffer->SetFragmentTexture(diffuseTexture.get(), 2);
			mainCommandBuffer->SetFragmentTexture(normalTexture.get(), 3);
			mainCommandBuffer->SetFragmentTexture(depthStencil.get(), 4);
			mainCommandBuffer->SetVertexBytes(pointLightUBO, 0);
			mainCommandBuffer->SetFragmentBytes(pointLightUBO, 0);
			mainCommandBuffer->SetVertexBuffer(spotLightVertexBuffer);
			mainCommandBuffer->SetIndexBuffer(spotLightIndexBuffer);
			mainCommandBuffer->SetVertexBuffer(worldOwning->renderData->spotLightData.GetDense().get_underlying().buffer, {
				.bindingPosition = 1
			});
			mainCommandBuffer->DrawIndexed(nSpotLightIndices, {
				.nInstances = worldOwning->renderData->spotLightData.DenseSize()
			});
			mainCommandBuffer->EndRenderDebugMarker();
			mainCommandBuffer->EndRendering();
		}

		mainCommandBuffer->EndRenderDebugMarker();

		// the on-screen render pass
		// contains the results of the previous stages, as well as the UI, skybox and any debugging primitives
		finalRenderPass->SetAttachmentTexture(0, nextimg);
		finalRenderPass->SetDepthAttachmentTexture(depthStencil.get());
		mainCommandBuffer->BeginRenderDebugMarker("Forward Pass");
		mainCommandBuffer->BeginRenderDebugMarker("Transition Lighting texture");
		mainCommandBuffer->TransitionResource(lightingTexture.get(), RGL::ResourceLayout::ColorAttachmentOptimal, RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::TransitionPosition::Bottom);
		mainCommandBuffer->TransitionResource(nextimg, RGL::ResourceLayout::Undefined, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::TransitionPosition::Top);
		mainCommandBuffer->EndRenderDebugMarker();
		

		mainCommandBuffer->BeginRendering(finalRenderPass);
		mainCommandBuffer->BeginRenderDebugMarker("Blit and Skybox");
		// start with the results of lighting
		mainCommandBuffer->BindRenderPipeline(lightToFBRenderPipeline);
		mainCommandBuffer->SetVertexBuffer(screenTriVerts);
		mainCommandBuffer->SetVertexBytes(lightUBO,0);
		mainCommandBuffer->SetFragmentBytes(lightUBO, 0);
		mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
		mainCommandBuffer->SetFragmentTexture(lightingTexture.get(), 1);
		mainCommandBuffer->Draw(3);

		// then do the skybox, if one is defined.
		if (worldOwning->skybox && worldOwning->skybox->skyMat && worldOwning->skybox->skyMat->GetMat()->renderPipeline) {
			mainCommandBuffer->BindRenderPipeline(worldOwning->skybox->skyMat->GetMat()->renderPipeline);
			uint32_t totalIndices = 0;
			// if a custom mesh is supplied, render that. Otherwise, render the builtin icosphere.
			if (worldOwning->skybox->skyMesh) {
				mainCommandBuffer->SetVertexBuffer(worldOwning->skybox->skyMesh->vertexBuffer);
				mainCommandBuffer->SetIndexBuffer(worldOwning->skybox->skyMesh->indexBuffer);
				totalIndices = worldOwning->skybox->skyMesh->totalIndices;
			}
			else {
				mainCommandBuffer->SetVertexBuffer(pointLightVertexBuffer);
				mainCommandBuffer->SetIndexBuffer(pointLightIndexBuffer);
				totalIndices = nPointLightIndices;
			}
			mainCommandBuffer->SetVertexBytes(viewproj, 0);
			mainCommandBuffer->DrawIndexed(totalIndices);
			mainCommandBuffer->EndRenderDebugMarker();
		}

		mainCommandBuffer->BeginRenderDebugMarker("GUI");
		worldOwning->Filter([](GUIComponent& gui) {
			gui.Render();	// kicks off commands for rendering UI
		});
#ifndef NDEBUG
			// process debug shapes
		worldOwning->FilterPolymorphic([](PolymorphicGetResult<IDebugRenderable, World::PolymorphicIndirection> dbg, const PolymorphicGetResult<Transform, World::PolymorphicIndirection> transform) {
			for (int i = 0; i < dbg.size(); i++) {
				auto& ptr = dbg[i];
				if (ptr.debugEnabled) {
					ptr.DebugDraw(dbgdraw, transform[0]);
				}
			}
		});
		mainCommandBuffer->BeginRenderDebugMarker("Debug Wireframes");
		Im3d::AppData& data = Im3d::GetAppData();
		data.m_appData = &lightUBO.viewProj;

		Im3d::GetContext().draw();
		mainCommandBuffer->EndRenderDebugMarker();
		
		if (debuggerContext) {
			auto& dbg = *debuggerContext;
			dbg.SetDimensions(bufferdims.width, bufferdims.height);
			dbg.SetDPIScale(GetDPIScale());
			dbg.Update();
			dbg.Render();
		}
		
		mainCommandBuffer->EndRenderDebugMarker();
		mainCommandBuffer->EndRenderDebugMarker();
		Im3d::NewFrame();
#endif
		mainCommandBuffer->EndRendering();
		mainCommandBuffer->TransitionResource(nextimg, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::ResourceLayout::Present, RGL::TransitionPosition::Bottom);
		mainCommandBuffer->End();

		// show the results to the user
		RGL::CommitConfig commitconfig{
				.signalFence = swapchainFence,
		};
		mainCommandBuffer->Commit(commitconfig);

		swapchain->Present(presentConfig);

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = duration_cast<std::chrono::microseconds>(end - start);
		currentFrameTime = duration.count();
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

	auto vertBuffer = device->CreateBuffer({
		uint32_t(nverts),
		{.VertexBuffer = true},
		sizeof(Im3d::VertexData),
		RGL::BufferAccess::Private,
	});
	vertBuffer->SetBufferData({ vertexdata, nverts * sizeof(Im3d::VertexData) });

	auto viewProj = *static_cast<glm::mat4*>(Im3d::GetAppData().m_appData);
    
    LightingUBO ubo{
        .viewProj = viewProj
    };

	mainCommandBuffer->SetVertexBytes(ubo,0);
	mainCommandBuffer->SetVertexBuffer(vertBuffer);
	mainCommandBuffer->Draw(nverts);

	// trash the buffer now that we're done with it
	gcBuffers.enqueue(vertBuffer);

#endif

}
