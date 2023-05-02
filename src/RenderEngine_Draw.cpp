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
			.viewRect = {0,0,nextImgSize.width,nextImgSize.height}
		};

		auto transitionGbuffers = [this](RGL::ResourceLayout from, RGL::ResourceLayout to) {
			for (const auto& ptr : { diffuseTexture, normalTexture }) {
				mainCommandBuffer->TransitionResource(ptr.get(), from, to, RGL::TransitionPosition::Top);
			}
		};

		transitionGbuffers(RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::ResourceLayout::ColorAttachmentOptimal);

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

		// do lighting pass
		mainCommandBuffer->TransitionResource(lightingTexture.get(), RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::TransitionPosition::Top);
		lightingRenderPass->SetDepthAttachmentTexture(depthStencil.get());
		lightingRenderPass->SetAttachmentTexture(0, lightingTexture.get());

		mainCommandBuffer->BeginRendering(lightingRenderPass);
		mainCommandBuffer->BindRenderPipeline(ambientLightRenderPipeline);
		mainCommandBuffer->SetCombinedTextureSampler(textureSampler, diffuseTexture.get(), 0);
		mainCommandBuffer->SetCombinedTextureSampler(textureSampler, normalTexture.get(), 1);

		mainCommandBuffer->SetVertexBuffer(screenTriVerts);
		mainCommandBuffer->SetVertexBytes(lightUBO, 0);
		mainCommandBuffer->SetVertexBuffer(worldOwning->ambientLightData.GetDense().get_underlying().buffer, {
			.bindingPosition = 1
		});
		mainCommandBuffer->Draw(3, {
			.nInstances = worldOwning->ambientLightData.DenseSize()
		});

		mainCommandBuffer->EndRendering();
		mainCommandBuffer->TransitionResource(lightingTexture.get(), RGL::ResourceLayout::ColorAttachmentOptimal, RGL::ResourceLayout::ShaderReadOnlyOptimal, RGL::TransitionPosition::Top);

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
		mainCommandBuffer->SetCombinedTextureSampler(textureSampler, lightingTexture.get(), 0);
		mainCommandBuffer->Draw(3);

		// then do the skybox
		mainCommandBuffer->BindRenderPipeline(worldOwning->skybox->skyMat->mat->renderPipeline);
		mainCommandBuffer->SetVertexBuffer(worldOwning->skybox->skyMesh->vertexBuffer);
		mainCommandBuffer->SetIndexBuffer(worldOwning->skybox->skyMesh->indexBuffer);

		mainCommandBuffer->SetVertexBytes(viewproj, 0);
		mainCommandBuffer->DrawIndexed(worldOwning->skybox->skyMesh->totalIndices);

		/*
			worldOwning->Filter([](GUIComponent& gui) {
				gui.Render();	// kicks off commands for rendering UI
			});
			*/
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
		Im3d::GetContext().draw();
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