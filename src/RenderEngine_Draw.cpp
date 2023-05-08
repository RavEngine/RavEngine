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

		auto& cam = worldOwning->GetComponent<CameraComponent>();
		auto viewproj = cam.GenerateProjectionMatrix() * cam.GenerateViewMatrix();

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
		for (const auto& [_,drawdata] : worldOwning->skinnedMeshRenderData) {
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

				bufferBegin += emptySpace;

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
				mainCommandBuffer->BindComputeBuffer(skinningOutputBuffer, 2);
				mainCommandBuffer->BindComputeBuffer(skinningPoseBuffer, 3);
				mainCommandBuffer->BindComputeBuffer(mesh->GetWeightsBuffer(), 4);
				mainCommandBuffer->DispatchCompute(std::ceil(numobjects / 8.0), std::ceil(numverts / 32.0), 1);
				mainCommandBuffer->EndCompute();

			}

			// was executed between every call to DispatchCompute on the old renderer:
			/*
			values[3] = static_cast<float>(computeOffsetIndex);
			numRowsUniform.SetValues(&values, 1);
			bgfx::setBuffer(11, skinningComputeBuffer.GetHandle(), bgfx::Access::Read);
			*/
			
		}
		
		auto transitionGbuffers = [this](RGL::ResourceLayout from, RGL::ResourceLayout to) {
			for (const auto& ptr : { diffuseTexture, normalTexture }) {
				mainCommandBuffer->TransitionResource(ptr.get(), from, to, RGL::TransitionPosition::Top);
			}
		};

		transitionGbuffers(RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::ResourceLayout::ColorAttachmentOptimal);
		mainCommandBuffer->TransitionResource(depthStencil.get(), RGL::ResourceLayout::DepthReadOnlyOptimal, RGL::ResourceLayout::DepthAttachmentOptimal, RGL::TransitionPosition::Top);

		// do static meshes
		mainCommandBuffer->BeginRendering(deferredRenderPass);
		for (auto& [materialInstance, drawcommand] : worldOwning->staticMeshRenderData) {
			// bind the pipeline
			mainCommandBuffer->BindRenderPipeline(materialInstance->GetMat()->renderPipeline);
			mainCommandBuffer->SetVertexBytes(viewproj, 0);
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

		transitionGbuffers(RGL::ResourceLayout::ColorAttachmentOptimal, RGL::ResourceLayout::ShaderReadOnlyOptimal);
		mainCommandBuffer->TransitionResource(depthStencil.get(), RGL::ResourceLayout::DepthAttachmentOptimal, RGL::ResourceLayout::DepthReadOnlyOptimal, RGL::TransitionPosition::Top);

		// do lighting pass
		mainCommandBuffer->TransitionResource(lightingTexture.get(), RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::TransitionPosition::Top);
		lightingRenderPass->SetDepthAttachmentTexture(depthStencil.get());
		lightingRenderPass->SetAttachmentTexture(0, lightingTexture.get());

		mainCommandBuffer->SetRenderPipelineBarrier({
			.Fragment = true
		});

		mainCommandBuffer->BeginRendering(lightingRenderPass);
		// ambient lights
        if (worldOwning->ambientLightData.DenseSize() > 0){
            mainCommandBuffer->BindRenderPipeline(ambientLightRenderPipeline);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, diffuseTexture.get(), 0);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, normalTexture.get(), 1);
            
            mainCommandBuffer->SetVertexBuffer(screenTriVerts);
            mainCommandBuffer->SetVertexBytes(lightUBO, 0);
            mainCommandBuffer->SetFragmentBytes(lightUBO, 0);
            mainCommandBuffer->SetVertexBuffer(worldOwning->ambientLightData.GetDense().get_underlying().buffer, {
                .bindingPosition = 1
            });
            mainCommandBuffer->Draw(3, {
                .nInstances = worldOwning->ambientLightData.DenseSize()
            });
        }

		// directional lights
        if (worldOwning->directionalLightData.DenseSize() > 0){
            mainCommandBuffer->BindRenderPipeline(dirLightRenderPipeline);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, diffuseTexture.get(), 0);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, normalTexture.get(), 1);
            mainCommandBuffer->SetVertexBuffer(screenTriVerts);
            mainCommandBuffer->SetVertexBytes(lightUBO, 0);
            mainCommandBuffer->SetFragmentBytes(lightUBO, 0);
            mainCommandBuffer->SetVertexBuffer(worldOwning->directionalLightData.GetDense().get_underlying().buffer, {
                .bindingPosition = 1
            });
            mainCommandBuffer->Draw(3, {
                .nInstances = worldOwning->directionalLightData.DenseSize()
            });
        }

		// point lights
        if (worldOwning->pointLightData.DenseSize() > 0){
            mainCommandBuffer->BindRenderPipeline(pointLightRenderPipeline);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, diffuseTexture.get(), 0);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, normalTexture.get(), 1);
            mainCommandBuffer->SetCombinedTextureSampler(textureSampler, depthStencil.get(), 2);
            mainCommandBuffer->SetVertexBytes(pointLightUBO, 0);
            mainCommandBuffer->SetFragmentBytes(pointLightUBO, 0);
            mainCommandBuffer->SetVertexBuffer(pointLightVertexBuffer);
            mainCommandBuffer->SetIndexBuffer(pointLightIndexBuffer);
            mainCommandBuffer->SetVertexBuffer(worldOwning->pointLightData.GetDense().get_underlying().buffer, {
                .bindingPosition = 1
            });
            mainCommandBuffer->DrawIndexed(nPointLightIndices, {
                .nInstances = worldOwning->pointLightData.DenseSize()
            });
        }

		// spot lights
		if (worldOwning->spotLightData.DenseSize() > 0) {
			mainCommandBuffer->BindRenderPipeline(spotLightRenderPipeline);
			mainCommandBuffer->SetCombinedTextureSampler(textureSampler, diffuseTexture.get(), 0);
			mainCommandBuffer->SetCombinedTextureSampler(textureSampler, normalTexture.get(), 1);
			mainCommandBuffer->SetCombinedTextureSampler(textureSampler, depthStencil.get(), 2);
			mainCommandBuffer->SetVertexBytes(pointLightUBO, 0);
			mainCommandBuffer->SetFragmentBytes(pointLightUBO, 0);
			mainCommandBuffer->SetVertexBuffer(spotLightVertexBuffer);
			mainCommandBuffer->SetIndexBuffer(spotLightIndexBuffer);
			mainCommandBuffer->SetVertexBuffer(worldOwning->spotLightData.GetDense().get_underlying().buffer, {
				.bindingPosition = 1
			});
			mainCommandBuffer->DrawIndexed(nSpotLightIndices, {
				.nInstances = worldOwning->spotLightData.DenseSize()
			});
		}

		mainCommandBuffer->EndRendering();
		mainCommandBuffer->TransitionResource(lightingTexture.get(), RGL::ResourceLayout::ColorAttachmentOptimal, RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::TransitionPosition::Bottom);

		// the on-screen render pass
		// contains the results of the previous stages, as well as the UI, skybox and any debugging primitives
		finalRenderPass->SetAttachmentTexture(0, nextimg);
		finalRenderPass->SetDepthAttachmentTexture(depthStencil.get());

		mainCommandBuffer->TransitionResource(nextimg, RGL::ResourceLayout::Undefined, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::TransitionPosition::Top);

		mainCommandBuffer->BeginRendering(finalRenderPass);

		// start with the results of lighting
		mainCommandBuffer->BindRenderPipeline(lightToFBRenderPipeline);
		mainCommandBuffer->SetVertexBuffer(screenTriVerts);
		mainCommandBuffer->SetVertexBytes(lightUBO,0);
		mainCommandBuffer->SetFragmentBytes(lightUBO, 0);
		mainCommandBuffer->SetCombinedTextureSampler(textureSampler, lightingTexture.get(), 0);
		mainCommandBuffer->Draw(3);

		// then do the skybox
		mainCommandBuffer->BindRenderPipeline(worldOwning->skybox->skyMat->mat->renderPipeline);
		mainCommandBuffer->SetVertexBuffer(worldOwning->skybox->skyMesh->vertexBuffer);
		mainCommandBuffer->SetIndexBuffer(worldOwning->skybox->skyMesh->indexBuffer);

		mainCommandBuffer->SetVertexBytes(viewproj, 0);
		mainCommandBuffer->DrawIndexed(worldOwning->skybox->skyMesh->totalIndices);
	
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
		Im3d::AppData& data = Im3d::GetAppData();
		data.m_appData = &lightUBO.viewProj;
		Im3d::GetContext().draw();

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
		RGL::BufferAccess::Shared,
	});
	vertBuffer->SetBufferData({ vertexdata, nverts });

	auto viewProj = *static_cast<glm::mat4*>(Im3d::GetAppData().m_appData);

	mainCommandBuffer->SetVertexBytes(viewProj,0);
	mainCommandBuffer->SetVertexBuffer(vertBuffer);
	mainCommandBuffer->Draw(nverts);

	// trash the buffer now that we're done with it
	gcBuffers.enqueue(vertBuffer);

#endif

}