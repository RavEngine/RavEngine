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

namespace RavEngine {

#ifndef NDEBUG
	static DebugDrawer dbgdraw;	//for rendering debug primitives
#endif

	/**
 Render one frame using the current state of every object in the world
 */
	void RenderEngine::Draw(Ref<RavEngine::World> worldOwning) {

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

		// render all the static meshes
		deferredRenderPass->SetAttachmentTexture(0, diffuseTexture.get());
		deferredRenderPass->SetAttachmentTexture(1, normalTexture.get());
		deferredRenderPass->SetDepthAttachmentTexture(depthStencil.get());

		auto nextimg = swapchain->ImageAtIndex(presentConfig.imageIndex);
		auto nextImgSize = nextimg->GetSize();

		auto allCameras = worldOwning->GetAllComponentsOfType<CameraComponent>();
		if (!allCameras)
		{
			Debug::Fatal("Cannot render: World does not have a camera!");
		}
		auto& cam = worldOwning->GetComponent<CameraComponent>();
		auto viewproj = cam.GenerateProjectionMatrix(nextImgSize.width, nextImgSize.height) * cam.GenerateViewMatrix();

		mainCommandBuffer->SetViewport({
		.width = static_cast<float>(nextImgSize.width),
		.height = static_cast<float>(nextImgSize.height),
			});
		mainCommandBuffer->SetScissor({
			.extent = {nextImgSize.width, nextImgSize.height}
			});

		LightingUBO lightUBO{
			.viewProj = viewproj,
			.viewRect = {0,0,nextImgSize.width,nextImgSize.height}
		};
		PointLightUBO pointLightUBO{
			.viewProj = lightUBO.viewProj,
			.invViewProj = glm::inverse(lightUBO.viewProj),
			.viewRect = lightUBO.viewRect
		};
		
		// dispatch skinning shaders
		mainCommandBuffer->BeginComputeDebugMarker("Skinning Compute Shader");
		for (const auto& [_,drawdata] : worldOwning->renderData->skinnedMeshRenderData) {
			uint32_t computeOffsetIndex = 0;
			uint32_t bufferBegin = 0;
			float values[4];
			for (const auto& command : drawdata.commands) {
				// seed compute shader for skinning
				// input buffer A: skeleton bind pose
				auto skeleton = command.skeleton.lock();
				// input buffer B: vertex weights by bone ID
				auto mesh = command.mesh.lock();
				// input buffer C: unposed vertices in mesh

				// output buffer A: posed output transformations for vertices
				auto numverts = mesh->GetNumVerts();
				const auto numobjects = command.transforms.DenseSize();

				auto emptySpace = numverts * numobjects * sizeof(glm::mat4);
				assert(emptySpace < std::numeric_limits<uint32_t>::max());


				computeOffsetIndex = bufferBegin;

				//pose SOA values
				//convert to float from double
				size_t totalsize = 0;
				for (const auto& ownerid : command.transforms.reverse_map) {
					auto& animator = worldOwning->GetComponent<AnimatorComponent>(ownerid);
					totalsize += animator.GetSkinningMats().size();
				}
				typedef Array<float, 16> arrtype;
				stackarray(pose_float, arrtype, totalsize);
				size_t index = 0;
				for (const auto& ownerid : command.transforms.reverse_map) {
					auto& animator = worldOwning->GetComponent<AnimatorComponent>(ownerid);
					auto& array = animator.GetSkinningMats();
					//in case of double mode, need to convert to float
					for (int i = 0; i < array.size(); i++) {
						//populate stack array values
						auto ptr = glm::value_ptr(array[i]);
						for (int offset = 0; offset < 16; offset++) {
							pose_float[index][offset] = static_cast<float>(ptr[offset]);
						}
						index++;
					}
				}
				assert(totalsize < std::numeric_limits<uint32_t>::max());	// pose buffer is too big!

				auto totalByteSize = totalsize * sizeof(glm::mat4);

				std::memcpy(static_cast<char*>(skinningPoseBuffer->GetMappedDataPtr()) + bufferBegin, pose_float, totalByteSize);

				// set skinning uniform
				SkinningUBO ubo{
					.NumObjects = {
						numobjects,
						numverts,
						skeleton->GetBindposes().size(),
						bufferBegin
					},
					.ComputeOffsets = {
						computeOffsetIndex,
						0,
						0,
						0
					}
				};

				mainCommandBuffer->BeginCompute(skinnedMeshComputePipeline);
				mainCommandBuffer->SetComputeBytes(ubo, 0);
				mainCommandBuffer->BindComputeBuffer(skinningOutputBuffer, 0);
				mainCommandBuffer->BindComputeBuffer(skinningPoseBuffer, 1);
				mainCommandBuffer->BindComputeBuffer(mesh->GetWeightsBuffer(), 2);
				mainCommandBuffer->DispatchCompute(std::ceil(numobjects / 8.0), std::ceil(numverts / 32.0), 1);
				mainCommandBuffer->EndCompute();

				bufferBegin += emptySpace;	// for the next iteration

			}

			// was executed between every call to DispatchCompute on the old renderer:
			/*
			values[3] = static_cast<float>(computeOffsetIndex);
			numRowsUniform.SetValues(&values, 1);
			bgfx::setBuffer(11, skinningComputeBuffer.GetHandle(), bgfx::Access::Read);
			*/
		}
		mainCommandBuffer->EndComputeDebugMarker();
		mainCommandBuffer->BeginRenderDebugMarker("Deferred Pass");
		mainCommandBuffer->BeginRenderDebugMarker("Transition Gbuffers");
		auto transitionGbuffers = [this](RGL::ResourceLayout from, RGL::ResourceLayout to) {
			for (const auto& ptr : { diffuseTexture, normalTexture }) {
				mainCommandBuffer->TransitionResource(ptr.get(), from, to, RGL::TransitionPosition::Top);
			}
		};

		transitionGbuffers(RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::ResourceLayout::ColorAttachmentOptimal);
		mainCommandBuffer->TransitionResource(depthStencil.get(), RGL::ResourceLayout::DepthReadOnlyOptimal, RGL::ResourceLayout::DepthAttachmentOptimal, RGL::TransitionPosition::Top);
		mainCommandBuffer->EndRenderDebugMarker();

		mainCommandBuffer->BeginComputeDebugMarker("Cull Static Meshes");
		for (auto& [materialInstance, drawcommand] : worldOwning->renderData->staticMeshRenderData) {
			//prepass: get number of LODs and entities
			uint32_t numLODs = 0, numEntities = 0;
			for (const auto& command : drawcommand.commands) {
				numLODs += 1;	//TODO: when LOD support is added, increment this by the number of LODs
				numEntities += command.entities.DenseSize();
			}

			auto reallocBuffer = [this](RGLBufferPtr& buffer, uint32_t size_count, uint32_t stride, RGL::BufferAccess access, bool writable) {
				if (buffer == nullptr || buffer->getBufferSize() < size_count * stride) {
					// trash old buffer if it exists
					if (buffer) {
						gcBuffers.enqueue(buffer);
					}
					buffer = device->CreateBuffer({
						size_count,
						{.StorageBuffer = true},
						stride,
						access,
						{.Writable = writable}
					});
					if (access == RGL::BufferAccess::Shared) {
						buffer->MapMemory();
					}
				}
			};

			reallocBuffer(drawcommand.cullingBuffer, numEntities, sizeof(entity_t), RGL::BufferAccess::Private, true);
			reallocBuffer(drawcommand.drawcallBuffer, numLODs, sizeof(RGL::IndirectIndexedCommand), RGL::BufferAccess::Shared, true);

			// initial populate of drawcall buffer
			{
				auto drawCallPtr = static_cast<RGL::IndirectIndexedCommand*>(drawcommand.drawcallBuffer->GetMappedDataPtr());
				for (const auto& command : drawcommand.commands) {
					if (auto mesh = command.mesh.lock()) {
						*drawCallPtr = {
							.indexCount = uint32_t(mesh->totalIndices),
							.instanceCount = 0,
							.indexStart = uint32_t(mesh->meshAllocation.indexRange->start / sizeof(uint32_t)),
							.baseVertex = uint32_t(mesh->meshAllocation.vertRange->start / sizeof(VertexNormalUV)),
							.baseInstance = 0,
						};

					}
					else {
						*drawCallPtr = { 0,0,0,0,0 };
					}
					
					drawCallPtr++;
				}
			}

			//TODO: dispatch culling shaders
			for (const auto& command : drawcommand.commands) {
				if (auto mesh = command.mesh.lock()) {
					uint32_t lodsForThisMesh = 1;	//TODO: when LODs are implemented, update this
					for (uint32_t i = 0; i < lodsForThisMesh; i++) {

					}
				}
			}
			

			
		}
		mainCommandBuffer->EndComputeDebugMarker();

		mainCommandBuffer->BeginRenderDebugMarker("Render Static Meshes");
		// do static meshes
		mainCommandBuffer->BeginRendering(deferredRenderPass);
		mainCommandBuffer->SetVertexBuffer(sharedVertexBuffer);
		mainCommandBuffer->SetIndexBuffer(sharedIndexBuffer);
		for (auto& [materialInstance, drawcommand] : worldOwning->renderData->staticMeshRenderData) {
			// bind the pipeline
			mainCommandBuffer->BindRenderPipeline(materialInstance->GetMat()->renderPipeline);

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
					mainCommandBuffer->SetCombinedTextureSampler(textureSampler, texture->GetRHITexturePointer().get(), i);
				}
			}


			// bind the culling buffer and the transform buffer
			mainCommandBuffer->SetVertexBuffer(drawcommand.cullingBuffer, { .bindingPosition = 1 });
			mainCommandBuffer->SetVertexBuffer(worldOwning->renderData->worldTransforms.buffer, { .bindingPosition = 2 });

			// do the indirect command
			mainCommandBuffer->ExecuteIndirectIndexed({
				.indirectBuffer = drawcommand.drawcallBuffer,
				.nDraws = uint32_t(drawcommand.drawcallBuffer->getBufferSize() / sizeof(RGL::IndirectIndexedCommand))
			});
		}
		mainCommandBuffer->EndRenderDebugMarker();

		// do skinned meshes
		mainCommandBuffer->BeginRenderDebugMarker("Render Skinned Meshes");
		for (auto& [materialInstance, drawcommand] : worldOwning->renderData->skinnedMeshRenderData) {
			// bind the pipeline
			mainCommandBuffer->BindRenderPipeline(materialInstance->GetMat()->renderPipeline);
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
					mainCommandBuffer->SetCombinedTextureSampler(textureSampler, texture->GetRHITexturePointer().get(), i);
				}
			}

			for (auto& command : drawcommand.commands) {
				// submit the draws for this mesh
				if (auto mesh = command.mesh.lock()) {
					mainCommandBuffer->SetVertexBuffer(mesh->vertexBuffer);
					auto& perInstanceDataBuffer = command.transforms.GetDense().get_underlying().buffer;
					mainCommandBuffer->SetVertexBuffer(perInstanceDataBuffer, {
						.bindingPosition = 1
						});
					mainCommandBuffer->SetIndexBuffer(mesh->indexBuffer);
					mainCommandBuffer->DrawIndexed(mesh->totalIndices, {
						.nInstances = command.transforms.DenseSize()
						});
				}
			}
		}
		mainCommandBuffer->EndRendering();
		mainCommandBuffer->EndRenderDebugMarker();
		mainCommandBuffer->EndRenderDebugMarker();

		mainCommandBuffer->BeginRenderDebugMarker("Transition Gbuffers");
		transitionGbuffers(RGL::ResourceLayout::ColorAttachmentOptimal, RGL::ResourceLayout::ShaderReadOnlyOptimal);
		mainCommandBuffer->TransitionResource(depthStencil.get(), RGL::ResourceLayout::DepthAttachmentOptimal, RGL::ResourceLayout::DepthReadOnlyOptimal, RGL::TransitionPosition::Top);

		// do lighting pass
		mainCommandBuffer->TransitionResource(lightingTexture.get(), RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::TransitionPosition::Top);
		mainCommandBuffer->EndRenderDebugMarker();
		lightingRenderPass->SetDepthAttachmentTexture(depthStencil.get());
		lightingRenderPass->SetAttachmentTexture(0, lightingTexture.get());

		mainCommandBuffer->SetRenderPipelineBarrier({
			.Fragment = true
		});

		mainCommandBuffer->BeginRendering(lightingRenderPass);
		mainCommandBuffer->BeginRenderDebugMarker("Lighting Pass");
		// ambient lights
        if (worldOwning->renderData->ambientLightData.DenseSize() > 0){
			mainCommandBuffer->BeginRenderDebugMarker("Render Ambient Lights");
            mainCommandBuffer->BindRenderPipeline(ambientLightRenderPipeline);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, diffuseTexture.get(), 0);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, normalTexture.get(), 1);
            
            mainCommandBuffer->SetVertexBuffer(screenTriVerts);
            mainCommandBuffer->SetVertexBytes(lightUBO, 0);
            mainCommandBuffer->SetFragmentBytes(lightUBO, 0);
            mainCommandBuffer->SetVertexBuffer(worldOwning->renderData->ambientLightData.GetDense().get_underlying().buffer, {
                .bindingPosition = 1
            });
            mainCommandBuffer->Draw(3, {
                .nInstances = worldOwning->renderData->ambientLightData.DenseSize()
            });
			mainCommandBuffer->EndRenderDebugMarker();
        }

		// directional lights
        if (worldOwning->renderData->directionalLightData.DenseSize() > 0){
			mainCommandBuffer->BeginRenderDebugMarker("Render Directional Lights");
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
        }

		// point lights
        if (worldOwning->renderData->pointLightData.DenseSize() > 0){
			mainCommandBuffer->BeginRenderDebugMarker("Render Point Lights");
            mainCommandBuffer->BindRenderPipeline(pointLightRenderPipeline);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, diffuseTexture.get(), 0);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, normalTexture.get(), 1);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, depthStencil.get(), 2);
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
        }

		// spot lights
		if (worldOwning->renderData->spotLightData.DenseSize() > 0) {
			mainCommandBuffer->BeginRenderDebugMarker("Render Spot Lights");
			mainCommandBuffer->BindRenderPipeline(spotLightRenderPipeline);
			mainCommandBuffer->SetCombinedTextureSampler(textureSampler, diffuseTexture.get(), 0);
			mainCommandBuffer->SetCombinedTextureSampler(textureSampler, normalTexture.get(), 1);
			mainCommandBuffer->SetCombinedTextureSampler(textureSampler, depthStencil.get(), 2);
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
		}

		mainCommandBuffer->EndRendering();
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
		mainCommandBuffer->SetCombinedTextureSampler(textureSampler, lightingTexture.get(), 0);
		mainCommandBuffer->Draw(3);

		// then do the skybox, if one is defined.
		if (worldOwning->skybox && worldOwning->skybox->skyMat && worldOwning->skybox->skyMat->mat->renderPipeline) {
			mainCommandBuffer->BindRenderPipeline(worldOwning->skybox->skyMat->mat->renderPipeline);
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

		mainCommandBuffer->BeginRenderDebugMarker("GUI");
		worldOwning->Filter([](GUIComponent& gui) {
			gui.Render();	// kicks off commands for rendering UI
		});
		/*
		if (debuggerContext) {
			auto& dbg = *debuggerContext;
			dbg.SetDimensions(bufferdims.width, bufferdims.height);
			dbg.SetDPIScale(GetDPIScale());
			dbg.Update();
			dbg.Render();
		}
		*/
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
