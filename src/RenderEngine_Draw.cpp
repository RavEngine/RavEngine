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
#include "Tonemap.hpp"
#include "BuiltinTonemap.hpp"
#include <ravengine_shader_defs.h>

#undef near		// for some INSANE reason, Microsoft defines these words and they leak into here only on ARM targets
#undef far

#if __APPLE__ || __EMSCRIPTEN__
//#define OCCLUSION_CULLING_UNAVAILABLE
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

	worldOwning->renderData.stagingBufferPool.Reset();	// release unused buffers
    
    DestroyUnusedResources();
	RVE_PROFILE_SECTION(resetCB, "Reset Command Buffer")
    mainCommandBuffer->Reset();
    mainCommandBuffer->Begin();
	RVE_PROFILE_SECTION_END(resetCB);
   
	RVE_PROFILE_SECTION(enc_sync_transforms,"Encode Sync Private Data");
    
    // sync private buffers
	bool transformSyncCommandBufferNeedsCommit = false;
    {
        auto& wrd = worldOwning->renderData;
        auto gcbuffer = [this](RGLBufferPtr oldPrivateBuffer){
            gcBuffers.enqueue(oldPrivateBuffer);
        };
		RVE_PROFILE_SECTION(transforms,"EncodeSync Transforms")
        wrd.worldTransforms.EncodeSync(device, transformSyncCommandBuffer, gcbuffer, transformSyncCommandBufferNeedsCommit);
		RVE_PROFILE_SECTION_END(transforms);
        // bitwise-or to prevent short-circuiting
        wrd.directionalLightData.EncodeSync(device, transformSyncCommandBuffer, gcbuffer, transformSyncCommandBufferNeedsCommit);
        wrd.pointLightData.EncodeSync(device, transformSyncCommandBuffer, gcbuffer, transformSyncCommandBufferNeedsCommit);
        wrd.spotLightData.EncodeSync(device, transformSyncCommandBuffer, gcbuffer, transformSyncCommandBufferNeedsCommit);
        wrd.ambientLightData.EncodeSync(device, transformSyncCommandBuffer, gcbuffer, transformSyncCommandBufferNeedsCommit);
        
        wrd.renderLayers.EncodeSync(device, transformSyncCommandBuffer, gcbuffer, transformSyncCommandBufferNeedsCommit);
        wrd.perObjectAttributes.EncodeSync(device, transformSyncCommandBuffer, gcbuffer, transformSyncCommandBufferNeedsCommit);
        
        auto syncMeshData = [&](auto&& dataset){
            for(auto& [mat, command] : dataset){
                for(auto& draw : command.commands){
                    draw.entities.EncodeSync(device, transformSyncCommandBuffer, gcbuffer, transformSyncCommandBufferNeedsCommit);
                    draw.mesh.lock()->lodDistances.EncodeSync(device, transformSyncCommandBuffer, gcbuffer, transformSyncCommandBufferNeedsCommit);
                }
            }
        };
        
        // MDIIcommands
		RVE_PROFILE_SECTION(staticmesh, "Encode Static Mesh Transforms");
        syncMeshData(wrd.staticMeshRenderData);
		RVE_PROFILE_SECTION_END(staticmesh);
		RVE_PROFILE_SECTION(skinnedMesh, "Encode Skinned Mesh Trasnforms");
        syncMeshData(wrd.skinnedMeshRenderData);
		RVE_PROFILE_SECTION_END(skinnedMesh);
        
        // directional light computations
		RVE_PROFILE_SECTION(dirlight, "Compute Directional Light Rendering Data");
        uint32_t numVaryingElts = 0;
        for(const auto& target : screenTargets){
            numVaryingElts += target.camDatas.size();
        }
        wrd.directionalLightPassVarying.Resize(numVaryingElts);
        if (wrd.directionalLightPassVaryingHostOnly.size() != numVaryingElts){
            wrd.directionalLightPassVaryingHostOnly.resize(numVaryingElts);
        }
        uint32_t passIndex = 0;
        for(const auto& target : screenTargets){
            for(const auto& camData : target.camDatas){
                // visit all the lights
                for (uint32_t i = 0; i < wrd.directionalLightData.DenseSize(); i++) {
                    auto& light = wrd.directionalLightData.GetHostDenseForWriting(i);
                    if (!light.castsShadows) {
                        continue;    // don't do anything if the light doesn't cast
                    }
                    auto sparseIdx = wrd.directionalLightData.GetSparseIndexForDense(i);
                    
                    const auto& origLight = worldOwning->GetComponent<DirectionalLight>({sparseIdx, worldOwning->VersionForEntity(sparseIdx)});
                    
                    // iterate the cascades
                    for(uint32_t index = 0; index < origLight.numCascades; index++){
                        
#ifndef NDEBUG
                        Debug::Assert(std::is_sorted(std::begin(origLight.shadowCascades), std::end(origLight.shadowCascades)),"Cascades must be in sorted order");
#endif
                        
                        // CSM code adapted from https://learnopengl.com/Guest-Articles/2021/CSM
                        constexpr static auto getFrustumCornersWorldSpace = [](const glm::mat4& proj, const glm::mat4& view)
                        {
                            const auto inv = glm::inverse(proj * view);
                            uint8_t i = 0;
                            Array<glm::vec4, 8> frustumCorners{ };
                            for (unsigned int x = 0; x < 2; ++x)
                            {
                                for (unsigned int y = 0; y < 2; ++y)
                                {
                                    for (unsigned int z = 0; z < 2; ++z)
                                    {
                                        const glm::vec4 ndcpt(
                                                              2.0f * x - 1.0f,
                                                              2.0f * y - 1.0f,
                                                              z,
                                                              1.0f);
                                        const glm::vec4 pt = inv * ndcpt;
                                        frustumCorners.at(i++) = pt / pt.w;
                                    }
                                }
                            }
                            
                            return frustumCorners;
                        };
                        
                        // decide the near and far clips for the cascade
                        float near = camData.zNearFar[0];
                        float far = camData.zNearFar[1];
                        if (index > 0){
                            near = glm::mix(camData.zNearFar[0], camData.zNearFar[1], origLight.shadowCascades[index-1]);
                        }
                        const auto numCascades = std::min<uint8_t>(origLight.numCascades, origLight.shadowCascades.size());
                        if (index < numCascades - 1){
                            far = glm::mix(camData.zNearFar[0], camData.zNearFar[1], origLight.shadowCascades[index]);
                        }
                        
                        //FIXME: the *1.5 is a hack. Without it, the matrices are not placed properly and the edges of the shadowmap cut into the view when the camera is not axis aligned in world space.
                        const auto proj = RMath::perspectiveProjection(deg_to_rad(camData.fov * 1.5), float(camData.targetWidth/camData.targetHeight), near, far);
                        
                        auto corners = getFrustumCornersWorldSpace(proj, camData.viewOnly);
                        
                        glm::vec3 center(0, 0, 0);
                        for (const auto& v : corners)
                        {
                            center += glm::vec3(v);
                        }
                        center /= corners.size();
                        
                        auto dirvec = light.direction;
                        
                        const auto lightView = glm::lookAt(
                                                           center + dirvec,
                                                           center,
                                                           glm::vec3(0.0f, 1.0f, 0.0f)
                                                           );
                        
                        float minX = std::numeric_limits<float>::max();
                        float maxX = std::numeric_limits<float>::lowest();
                        float minY = std::numeric_limits<float>::max();
                        float maxY = std::numeric_limits<float>::lowest();
                        float minZ = std::numeric_limits<float>::max();
                        float maxZ = std::numeric_limits<float>::lowest();
                        for (const auto& v : corners)
                        {
                            const auto trf = lightView * v;
                            minX = std::min(minX, trf.x);
                            maxX = std::max(maxX, trf.x);
                            minY = std::min(minY, trf.y);
                            maxY = std::max(maxY, trf.y);
                            minZ = std::min(minZ, trf.z);
                            maxZ = std::max(maxZ, trf.z);
                        }
                        
                        // TODO: Tune this parameter according to the scene
                        constexpr float zMult = 10.0f;
                        if (minZ < 0)
                        {
                            minZ *= zMult;
                        }
                        else
                        {
                            minZ /= zMult;
                        }
                        if (maxZ < 0)
                        {
                            maxZ /= zMult;
                        }
                        else
                        {
                            maxZ *= zMult;
                        }
                        
                        // calculate the proj centered on the camera
                        auto centerX = (minX + maxX) / 2;
                        
                        auto lightProj = RMath::orthoProjection<float>(minX, maxX, minY, maxY, minZ, maxZ);
                        
                        const uint32_t varyingLightIndex = passIndex + i;
                        
                        wrd.directionalLightPassVarying.GetValueAtForWriting(varyingLightIndex).lightViewProj[index] = lightProj * lightView;
                        wrd.directionalLightPassVaryingHostOnly[varyingLightIndex].lightview[index] = lightView;
                        wrd.directionalLightPassVaryingHostOnly[varyingLightIndex].lightProj[index] = lightProj;
                        
                        light.cascadeDistances[index] = far;
                    }
                }
                passIndex++;
            }
        }
        
        wrd.directionalLightPassVarying.EncodeSync(device, transformSyncCommandBuffer, gcbuffer, transformSyncCommandBufferNeedsCommit);
		RVE_PROFILE_SECTION_END(dirlight);
        
        if (transformSyncCommandBufferNeedsCommit){
            transformSyncCommandBuffer->End();
            RGL::CommitConfig config{

            };
            // this CB does not need to signal a fence because CBs on a given queue are guarenteed to complete before the next one begins
            transformSyncCommandBuffer->Commit(config);
        }
    }

	auto renderSkybox = [this](RGLRenderPipelinePtr skyboxPipeline, const glm::mat4& invView, const glm::vec3 camPos, float fov_rad, RGL::Viewport fullSizeViewport, RGL::Rect fullSizeScissor, auto&& extraFn) {
		struct skyboxData {
			glm::mat3 invView;
			glm::vec3 camPos;
			float fov;
			float aspectRatio;
		} data{
			invView,
			camPos,
			fov_rad,
			float(fullSizeViewport.width) / float(fullSizeViewport.height)
		};

		auto transientOffset = WriteTransient(data);

		
		mainCommandBuffer->SetViewport(fullSizeViewport);
		mainCommandBuffer->SetScissor(fullSizeScissor);
		mainCommandBuffer->BindRenderPipeline(skyboxPipeline);
		mainCommandBuffer->BindBuffer(transientBuffer, 1, transientOffset);
		mainCommandBuffer->SetVertexBuffer(screenTriVerts);
		extraFn();
		mainCommandBuffer->Draw(3);
		
	};
   
	// render skyboxes if there are any
	for (auto& ambientLight : *worldOwning->GetAllComponentsOfType<AmbientLight>()) {
		mainCommandBuffer->BeginRenderDebugMarker("Skybox for environment maps");
		if (ambientLight.environment && ambientLight.environment->environmentNeedsUpdate) {
			ambientLight.environment->environmentNeedsUpdate = false;
			const auto& outputTex = ambientLight.environment->outputTexture;
			
			// render the six faces
			//TODO: this is inefficient as it incurs a copy and wastes memory
			constexpr static vector3 campos(0, 0, 0);
			for (uint8_t i = 0; i < 6; i++){
				const auto dim = outputTex->GetTextureSize();
				RGL::Viewport faceViewport{
					.x = 0,
					.y = 0,
					.width = float(dim.width),
					.height = float(dim.height),
				};
				RGL::Rect faceScissor{
					.offset = {0, 0},
					.extent = {dim.width, dim.height}
				};
				glm::mat4 view = PointLight::CalcViewMatrix(campos, i);
				auto stagingView = ambientLight.environment->stagingTexture->GetRHITexturePointer()->GetDefaultView();
				envSkyboxPass->SetAttachmentTexture(0, stagingView);
				envSkyboxPass->SetDepthAttachmentTexture(ambientLight.environment->stagingDepthTexture->GetDefaultView());
				mainCommandBuffer->BeginRendering(envSkyboxPass);
				renderSkybox(ambientLight.environment->sky->skyMat->GetMat()->renderPipeline, glm::transpose(view), campos, deg_to_rad(90), faceViewport, faceScissor, [] {});
				mainCommandBuffer->EndRendering();

				mainCommandBuffer->CopyTextureToTexture(
					{
						.texture = stagingView,
						.mip = 0,
						.layer = 0,
					},
					{ 
						.texture = outputTex->GetView(),
						.mip = 0,
						.layer = i
					}
				);
			}
			for (uint8_t i = 0; i < 6; i++) {
				// convolve the roughness textures
				glm::mat4 view = PointLight::CalcViewMatrix(campos, i);
				const auto numLevels = ambientLight.environment->stagingDepthTexture->GetNumMips();
				const auto stagingTarget = ambientLight.environment->stagingTexture->GetRHITexturePointer();
				const auto dim = outputTex->GetTextureSize();
				for (uint8_t mip = 1; mip < numLevels; mip++) {
					const uint32_t dimDivisor = std::pow(2, mip);
					RGL::Viewport faceViewport{
						.x = 0,
						.y = 0,
						.width = float(dim.width) / dimDivisor,
						.height = float(dim.height) / dimDivisor,
					};
					RGL::Rect faceScissor{
						.offset = {0, 0},
						.extent = {dim.width / dimDivisor, dim.height / dimDivisor}
					};
                    const auto renderTarget = stagingTarget->GetViewForMip(mip);
                    envSkyboxPass->SetAttachmentTexture(0, renderTarget);
					mainCommandBuffer->BeginRendering(envSkyboxPass);
					renderSkybox(environmentPreFilterPipeline, glm::transpose(view), campos, deg_to_rad(90), faceViewport, faceScissor, [&] {
						mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
						mainCommandBuffer->SetFragmentTexture(outputTex->GetView(), 2);
						SkyboxEnvPrefilterUBO constants{
							.roughness = float(i) / numLevels
						};
						mainCommandBuffer->SetFragmentBytes(constants, 0);
					});
					mainCommandBuffer->EndRendering();
					mainCommandBuffer->CopyTextureToTexture(
						{
							.texture = stagingTarget->GetDefaultView(),
							.mip = mip,
							.layer = 0,
						},
						{
							.texture = outputTex->GetView(),
							.mip = mip,
							.layer = i
						}
					);
				}
			}

			// compute irradiance
			for (uint8_t i = 0; i < 6; i++) {

			}
		}
		mainCommandBuffer->EndRenderDebugMarker();
	}
    	
    auto worldTransformBuffer = worldOwning->renderData.worldTransforms.GetPrivateBuffer();

	RVE_PROFILE_SECTION_END(enc_sync_transforms);
        
	if (transientSubmittedLastFrame) {
		// cannot modify the transient staging buffer until this is done
		transientCommandBuffer->BlockUntilCompleted();
	}
    uint32_t camIdx = 0; // used for selecting directional lights in the lit pass

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
				auto newSize = closest_power_of<uint32_t>(neededSize, 2);
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
		resizeSkeletonBuffer(sharedSkinnedPositionBuffer, sizeof(VertexPosition_t), totalVertsToSkin, { .StorageBuffer = true, .VertexBuffer = true }, RGL::BufferAccess::Private, { .Writable = true, .debugName = "Shared Skinned Position Buffer" });
		resizeSkeletonBuffer(sharedSkinnedNormalBuffer, sizeof(VertexNormal_t), totalVertsToSkin, { .StorageBuffer = true, .VertexBuffer = true }, RGL::BufferAccess::Private, { .Writable = true, .debugName = "Shared Skinned Position Buffer" });
		resizeSkeletonBuffer(sharedSkinnedTangentBuffer, sizeof(VertexTangent_t), totalVertsToSkin, { .StorageBuffer = true, .VertexBuffer = true }, RGL::BufferAccess::Private, { .Writable = true, .debugName = "Shared Skinned Position Buffer" });
		resizeSkeletonBuffer(sharedSkinnedBitangentBuffer, sizeof(VertexBitangent_t), totalVertsToSkin, { .StorageBuffer = true, .VertexBuffer = true }, RGL::BufferAccess::Private, { .Writable = true, .debugName = "Shared Skinned Position Buffer" });
		resizeSkeletonBuffer(sharedSkinnedUV0Buffer, sizeof(VertexUV_t), totalVertsToSkin, { .StorageBuffer = true, .VertexBuffer = true }, RGL::BufferAccess::Private, { .Writable = true, .debugName = "Shared Skinned Position Buffer" });


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
				ubo.indexBufferOffset = mesh->GetAllocation().getIndexRangeStart();
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
		RVE_PROFILE_FN_N("Enc Pose Skinned Meshes");

		mainCommandBuffer->BeginComputeDebugMarker("Pose Skinned Meshes");
		mainCommandBuffer->BeginCompute(skinnedMeshComputePipeline);
		mainCommandBuffer->BindComputeBuffer(sharedSkinnedPositionBuffer, 0);
		mainCommandBuffer->BindComputeBuffer(sharedSkinnedNormalBuffer, 1);
		mainCommandBuffer->BindComputeBuffer(sharedSkinnedTangentBuffer, 2);
		mainCommandBuffer->BindComputeBuffer(sharedSkinnedBitangentBuffer, 3);
		mainCommandBuffer->BindComputeBuffer(sharedSkinnedUV0Buffer, 4);

		mainCommandBuffer->BindComputeBuffer(sharedPositionBuffer, 10);
		mainCommandBuffer->BindComputeBuffer(sharedNormalBuffer, 11);
		mainCommandBuffer->BindComputeBuffer(sharedTangentBuffer, 12);
		mainCommandBuffer->BindComputeBuffer(sharedBitangentBuffer, 13);
		mainCommandBuffer->BindComputeBuffer(sharedUV0Buffer, 14);

		mainCommandBuffer->BindComputeBuffer(sharedSkeletonMatrixBuffer, 20);
		using mat_t = glm::mat4;
		std::span<mat_t> matbufMem{ static_cast<mat_t*>(sharedSkeletonMatrixBuffer->GetMappedDataPtr()), sharedSkeletonMatrixBuffer->getBufferSize() / sizeof(mat_t) };
		SkinningUBO subo;
		for (const auto& [materialInstance, drawcommand] : worldOwning->renderData.skinnedMeshRenderData) {
			for (auto& command : drawcommand.commands) {
				auto skeleton = command.skeleton.lock();
				auto mesh = command.mesh.lock();
				auto& entities = command.entities;
				mainCommandBuffer->BindComputeBuffer(mesh->GetWeightsBuffer(), 21);

				subo.numObjects = command.entities.DenseSize();
				subo.numVertices = mesh->GetNumVerts();
				subo.numBones = skeleton->GetSkeleton()->num_joints();
				subo.vertexReadOffset = mesh->GetAllocation().getVertexRangeStart();

				// write joint transform matrices into buffer and update uniform offset
				{
					uint32_t object_id = 0;
					for (const auto& ownerid : command.entities.GetReverseMap()) {
                        auto& animator = worldOwning->GetComponent<AnimatorComponent>({ownerid, worldOwning->VersionForEntity(ownerid)});
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

	auto tickParticles = [this, worldOwning, worldTransformBuffer]() {
		mainCommandBuffer->BeginComputeDebugMarker("Particle Update");

        worldOwning->Filter([this, worldOwning, worldTransformBuffer](ParticleEmitter& emitter, const Transform& transform) {
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
							.indexStart = allocation.getIndexRangeStart(),
							.baseVertex = allocation.getVertexRangeStart(),
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
				mainCommandBuffer->BindComputeBuffer(worldTransformBuffer, 4);

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
	struct RenderFromPerspectiveSettings {
		bool enableSSAO = false;
	};
    auto renderFromPerspective = [this, &worldTransformBuffer, &worldOwning, &skeletalPrepareResult, &camIdx]<bool includeLighting = true, bool transparentMode = false, bool runCulling = true>(const matrix4& viewproj, const matrix4& viewonly, const matrix4& projOnly, vector3 camPos, glm::vec2 zNearFar, RGLRenderPassPtr renderPass, auto&& pipelineSelectorFunction, RGL::Rect viewportScissor, LightingType lightingFilter, const DepthPyramid& pyramid, const renderlayer_t layers, const RenderTargetCollection* target, const RenderFromPerspectiveSettings extraSettings){
			RVE_PROFILE_FN_N("RenderFromPerspective");
            uint32_t particleBillboardMatrices = 0;

            struct QuadParticleData {
                glm::mat4 viewProj;
                glm::mat3 billboard;
            };
                
            glm::mat3 rotComp = viewonly;
                
            QuadParticleData quadData{
                .viewProj = viewproj,
                .billboard = glm::transpose(rotComp),
            };
                
            particleBillboardMatrices = WriteTransient(quadData);

			if constexpr (includeLighting) {
				// dispatch the lighting binning shaders
				RVE_PROFILE_SECTION(lightBinning,"Light binning");
				mainCommandBuffer->BeginComputeDebugMarker("Light Binning");
				const auto nPointLights = worldOwning->renderData.pointLightData.DenseSize();
				const auto nSpotLights = worldOwning->renderData.spotLightData.DenseSize();
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
						mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.pointLightData.GetPrivateBuffer(), 1);
						mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.spotLightData.GetPrivateBuffer(), 2);

						constexpr static auto threadGroupSize = 128;

						mainCommandBuffer->DispatchCompute(Clustered::numClusters / threadGroupSize, 1, 1, threadGroupSize, 1, 1);

						mainCommandBuffer->EndCompute();
					}
				}
				RVE_PROFILE_SECTION_END(lightBinning);
				mainCommandBuffer->EndComputeDebugMarker();
			}

#pragma pack(push, 1)
			const auto outputDim = viewportScissor.extent;
			bool wantsSSAO = target && extraSettings.enableSSAO;
            const auto ssaoTargetTexture = wantsSSAO ? target->ssaoOutputTexture1 : Texture::Manager::defaultTexture->GetRHITexturePointer();
			struct LightData {
				glm::mat4 viewProj;
				glm::mat4 viewOnly;
				glm::mat4 invView;
				glm::mat4 projOnly;
				glm::uvec4 screenDimension;
				glm::vec3 camPos;
				glm::uvec3 gridSize;
				glm::uvec2 outputSize;
				uint32_t ambientLightCount;
				uint32_t directionalLightCount;
				float zNear;
				float zFar;
				uint32_t aoBindlessIndex = 0;
			}
			lightData{
				.viewProj = viewproj,
				.viewOnly = viewonly,
				.invView = glm::inverse(viewonly),
				.projOnly = projOnly,
				.screenDimension = { viewportScissor.offset[0],viewportScissor.offset[1], viewportScissor.extent[0],viewportScissor.extent[1] },
				.camPos = camPos,
				.gridSize = { Clustered::gridSizeX, Clustered::gridSizeY, Clustered::gridSizeZ },
				.outputSize = {outputDim[0], outputDim[1]},
				.ambientLightCount = worldOwning->renderData.ambientLightData.DenseSize(),
				.directionalLightCount = worldOwning->renderData.directionalLightData.DenseSize(),
				.zNear = zNearFar.x,
				.zFar = zNearFar.y,
                .aoBindlessIndex = ssaoTargetTexture->GetDefaultView().GetReadonlyBindlessTextureHandle()
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
				, materialInstance.mat->GetMat()->variant);

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
					}, materialInstance.mat->GetMat()->variant);

				return shouldKeep;
				};


            auto cullSkeletalMeshes = [this, &worldTransformBuffer, &worldOwning, &reallocBuffer, layers, lightingFilter, &camPos](matrix4 viewproj, const DepthPyramid pyramid) {
				RVE_PROFILE_FN_N("Cull Skeletal Meshes");
				// first reset the indirect buffers
				uint32_t skeletalVertexOffset = 0;
				for (auto& [materialInstance, drawcommand] : worldOwning->renderData.skinnedMeshRenderData) {

					bool shouldKeep = filterRenderData(lightingFilter, materialInstance);


					// is this the correct material type? if not, skip
					if (!shouldKeep) {
						continue;
					}

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
							for (uint32_t i = 0; i < nEntitiesInThisCommand; i++) {
								for (uint32_t lodID = 0; lodID < mesh->GetNumLods(); lodID++) {
									initData = {
										.indexCount = uint32_t(mesh->GetNumIndices()),
										.instanceCount = 0,
										.indexStart = mesh->GetAllocation().getIndexRangeStart(),
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

				// the culling shader will decide for each draw if the draw should exist (and set its instance count to 1 from 0).

				mainCommandBuffer->BeginComputeDebugMarker("Cull Skinned Meshes");
				
				for (auto& [materialInstance, drawcommand] : worldOwning->renderData.skinnedMeshRenderData) {
					bool shouldKeep = filterRenderData(lightingFilter, materialInstance);

					// is this the correct material type? if not, skip
					if (!shouldKeep) {
						continue;
					}

					CullingUBO globalCubo{
						.viewProj = viewproj,
						.camPos = camPos,
						.numCubos = 0,
						.cameraRenderLayers = layers,
						.singleInstanceModeAndShadowMode = 1u | (lightingFilter.FilterLightBlockers ? (1 << 1) : 0u),
					};

					for (const auto& command : drawcommand.commands) {
						if (auto mesh = command.mesh.lock()) {
							globalCubo.numCubos += mesh->GetNumLods();
						}
					}

					reallocBuffer(drawcommand.cuboBuffer, globalCubo.numCubos, sizeof(CullingUBOinstance), RGL::BufferAccess::Private, { .StorageBuffer = true, }, { .Writable = false, .debugName = "CUBO Private buffer" });
					reallocBuffer(drawcommand.cuboStagingBuffer, globalCubo.numCubos, sizeof(CullingUBOinstance), RGL::BufferAccess::Shared, { .StorageBuffer = true, }, { .Transfersource = true, .Writable = false, .debugName = "CUBO Shared buffer" });


					CullingUBOinstance cubo{
						.indirectBufferOffset = 0,
						.numLODs = 1,
					};
					{
						uint32_t i = 0;
						for (auto& command : drawcommand.commands) {

							if (auto mesh = command.mesh.lock()) {
								uint32_t lodsForThisMesh = 1;	// TODO: skinned meshes do not support LOD groups 

								cubo.numObjects = command.entities.DenseSize();
								cubo.radius = mesh->GetRadius();
								cubo.idOutputBufferBindlessHandle = drawcommand.cullingBuffer->GetReadwriteBindlessGPUHandle();
								cubo.indirectOutputBufferBindlessHandle = drawcommand.indirectBuffer->GetReadwriteBindlessGPUHandle();
								cubo.lodDistanceBufferBindlessHandle = mesh->lodDistances.GetPrivateBuffer()->GetReadonlyBindlessGPUHandle();
								cubo.entityIDInputBufferBindlessHandle = command.entities.GetPrivateBuffer()->GetReadonlyBindlessGPUHandle();

								drawcommand.cuboStagingBuffer->UpdateBufferData(cubo, i * sizeof(CullingUBOinstance));
								i++;

								cubo.indirectBufferOffset += lodsForThisMesh;
								cubo.cullingBufferOffset += lodsForThisMesh * command.entities.DenseSize();
							}
						}
					}
					mainCommandBuffer->CopyBufferToBuffer(
						{
							.buffer = drawcommand.cuboStagingBuffer
						},
						{
							.buffer = drawcommand.cuboBuffer
						},
						drawcommand.cuboBuffer->getBufferSize());

					mainCommandBuffer->BeginCompute(defaultCullingComputePipeline);
                    for (auto& [materialInstance, drawcommand] :  worldOwning->renderData.skinnedMeshRenderData) {
                        for (auto& command : drawcommand.commands) {
                            // make buffers resident
                            if (auto mesh = command.mesh.lock()) {
                                mainCommandBuffer->UseResource(drawcommand.cullingBuffer);
                                mainCommandBuffer->UseResource(drawcommand.indirectBuffer);
                                mainCommandBuffer->UseResource(mesh->lodDistances.GetPrivateBuffer());
                                mainCommandBuffer->UseResource(command.entities.GetPrivateBuffer());
                            }
                        }
                    }
                    
					mainCommandBuffer->BindComputeBuffer(drawcommand.cuboBuffer, DefaultCullBindings::Cubo);
					mainCommandBuffer->BindComputeBuffer(worldTransformBuffer, DefaultCullBindings::modelMatrix);
                    mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.renderLayers.GetPrivateBuffer(), DefaultCullBindings::renderLayer);
					mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.perObjectAttributes.GetPrivateBuffer(), DefaultCullBindings::perObject);
					mainCommandBuffer->BindBindlessBufferDescriptorSet(3);
					mainCommandBuffer->BindBindlessBufferDescriptorSet(4);
					mainCommandBuffer->BindBindlessBufferDescriptorSet(5);
					mainCommandBuffer->BindBindlessBufferDescriptorSet(6);

                    mainCommandBuffer->SetComputeTexture(pyramid.pyramidTexture->GetDefaultView(), DefaultCullBindings::depthPyramid);
                    mainCommandBuffer->SetComputeSampler(depthPyramidSampler, DefaultCullBindings::depthPyramidSamplerBinding);
					mainCommandBuffer->SetComputeBytes(globalCubo, 0);
					mainCommandBuffer->DispatchCompute(std::ceil(cubo.numObjects / 64.f), 1, 1, 64, 1, 1);
					mainCommandBuffer->EndCompute();
				}
				mainCommandBuffer->EndComputeDebugMarker();
			};


            auto cullTheRenderData = [this, &viewproj, &worldTransformBuffer, &camPos, &pyramid, &lightingFilter, &reallocBuffer, layers, &worldOwning](auto&& renderData) {
				RVE_PROFILE_FN_N("Cull RenderData");
				CullingUBO globalCubo{
					.viewProj = viewproj,
					.camPos = camPos,
					.numCubos = 0,
					.cameraRenderLayers = layers,
					.singleInstanceModeAndShadowMode = (lightingFilter.FilterLightBlockers ? (1 << 1) : 0u),
				};

				uint32_t totalEntities = 0;
				for (auto& [materialInstance, drawcommand] : renderData) {
					bool shouldKeep = filterRenderData(lightingFilter, materialInstance);
					if (!shouldKeep) {
						continue;
					}

					for (const auto& command : drawcommand.commands) {
						if (auto mesh = command.mesh.lock()) {
							globalCubo.numCubos += mesh->GetNumLods();
							totalEntities += command.entities.DenseSize();
						}
					}
				}
				if (globalCubo.numCubos == 0) {
					return;
				}

				reallocBuffer(worldOwning->renderData.cuboBuffer, globalCubo.numCubos, sizeof(CullingUBOinstance), RGL::BufferAccess::Private, { .StorageBuffer = true, }, { .Writable = false, .debugName = "CUBO Private buffer" });

				auto cuboStagingBuffer = worldOwning->renderData.stagingBufferPool.GetBuffer(worldOwning->renderData.cuboBuffer->getBufferSize());

				uint32_t cuboIdx = 0;
				for (auto& [materialInstance, drawcommand] : renderData) {
					bool shouldKeep = filterRenderData(lightingFilter, materialInstance);

					
					// is this the correct material type? if not, skip
					if (!shouldKeep) {
						continue;
					}

					//prepass: get number of LODs and entities
					uint32_t numLODs = 0, numEntities = 0;

					MeshAttributes materialAttributes;
					std::visit(CaseAnalysis{
					[&materialAttributes](const Ref<LitMaterial>& mat) {
						materialAttributes = mat->GetAttributes();
					},
					[&materialAttributes](const Ref<UnlitMaterial>& mat) {
						materialAttributes = mat->GetAttributes();
					} }
					, materialInstance.mat->GetMat()->variant);



					for (const auto& command : drawcommand.commands) {
						if (auto mesh = command.mesh.lock()) {
							auto meshattr = mesh->GetMeshForLOD(0)->GetAttributes();
							Debug::Assert(mesh->GetNumLods() > 0, "Mesh has no LODs!");
							Debug::Assert(materialAttributes.CompatibleWith(meshattr), "Mesh does not have all attributes required for material!");
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
						RVE_PROFILE_FN_N("Update staging buffer");
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
										.indexStart = meshInst->meshAllocation.getIndexRangeStart(),
										.baseVertex = meshInst->meshAllocation.getVertexRangeStart(),
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

					RVE_PROFILE_SECTION(dispatchcull,"Write Cubo Data");


					CullingUBOinstance cubo{
						.indirectBufferOffset = 0,
					};
					static_assert(sizeof(cubo) <= 128, "CUBO is too big!");
					{
						
						for (auto& command : drawcommand.commands) {

							if (auto mesh = command.mesh.lock()) {
								uint32_t lodsForThisMesh = mesh->GetNumLods();

								cubo.numObjects = command.entities.DenseSize();
								cubo.radius = mesh->GetRadius();
								cubo.numLODs = lodsForThisMesh;
								cubo.idOutputBufferBindlessHandle = drawcommand.cullingBuffer->GetReadwriteBindlessGPUHandle();
								cubo.indirectOutputBufferBindlessHandle = drawcommand.indirectBuffer->GetReadwriteBindlessGPUHandle();
								cubo.lodDistanceBufferBindlessHandle = mesh->lodDistances.GetPrivateBuffer()->GetReadonlyBindlessGPUHandle();
								cubo.entityIDInputBufferBindlessHandle = command.entities.GetPrivateBuffer()->GetReadonlyBindlessGPUHandle();


								cuboStagingBuffer->UpdateBufferData(cubo, cuboIdx * sizeof(CullingUBOinstance));
								cuboIdx++;

								cubo.indirectBufferOffset += lodsForThisMesh;
								cubo.cullingBufferOffset += lodsForThisMesh * command.entities.DenseSize();
							}
						}
					}

					RVE_PROFILE_SECTION_END(dispatchcull);
				}
				// copy to cubo buffer
				mainCommandBuffer->CopyBufferToBuffer(
					{
						.buffer = cuboStagingBuffer
					},
					{
						.buffer = worldOwning->renderData.cuboBuffer
					},
					worldOwning->renderData.cuboBuffer->getBufferSize());

				mainCommandBuffer->BeginCompute(defaultCullingComputePipeline);
                for (auto& [materialInstance, drawcommand] : renderData) {
                    for (auto& command : drawcommand.commands) {
                        // make buffers resident
                        if (auto mesh = command.mesh.lock()) {
                            mainCommandBuffer->UseResource(drawcommand.cullingBuffer);
                            mainCommandBuffer->UseResource(drawcommand.indirectBuffer);
                            mainCommandBuffer->UseResource(mesh->lodDistances.GetPrivateBuffer());
                            mainCommandBuffer->UseResource(command.entities.GetPrivateBuffer());
                        }
                    }
                }
                mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.cuboBuffer, DefaultCullBindings::Cubo);
                mainCommandBuffer->BindComputeBuffer(worldTransformBuffer, DefaultCullBindings::modelMatrix);
				mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.renderLayers.GetPrivateBuffer(), DefaultCullBindings::renderLayer);
				mainCommandBuffer->BindComputeBuffer(worldOwning->renderData.perObjectAttributes.GetPrivateBuffer(), DefaultCullBindings::perObject);
				mainCommandBuffer->BindBindlessBufferDescriptorSet(3);
				mainCommandBuffer->BindBindlessBufferDescriptorSet(4);
				mainCommandBuffer->BindBindlessBufferDescriptorSet(5);
				mainCommandBuffer->BindBindlessBufferDescriptorSet(6);
                mainCommandBuffer->SetComputeTexture(pyramid.pyramidTexture->GetDefaultView(), DefaultCullBindings::depthPyramid);
                mainCommandBuffer->SetComputeSampler(depthPyramidSampler, DefaultCullBindings::depthPyramidSamplerBinding);
				mainCommandBuffer->SetComputeBytes(globalCubo, 0);
				mainCommandBuffer->DispatchCompute(std::ceil(totalEntities / 64.f), 1, 1, 64, 1, 1);

				mainCommandBuffer->EndCompute();

			};
			struct BufferSet {
				RGLBufferPtr positionBuffer, normalBuffer, tangentBuffer, bitangentBuffer, uv0Buffer, lightmapBuffer;
			};
            auto renderTheRenderData = [this, &viewproj, &viewonly,&projOnly, &worldTransformBuffer, &pipelineSelectorFunction, &viewportScissor, &worldOwning, particleBillboardMatrices, &lightDataOffset,&layers, &target, &camIdx](auto&& renderData, const BufferSet& vertexBufferSet, LightingType currentLightingType) {
				// do static meshes
				RVE_PROFILE_FN_N("RenderTheRenderData");
				mainCommandBuffer->SetViewport({
					.x = float(viewportScissor.offset[0]),
					.y = float(viewportScissor.offset[1]),
					.width = static_cast<float>(viewportScissor.extent[0]),
					.height = static_cast<float>(viewportScissor.extent[1]),
					});
				mainCommandBuffer->SetScissor(viewportScissor);
				
				for (auto& [materialInstance, drawcommand] : renderData) {

					bool shouldKeep = filterRenderData(currentLightingType, materialInstance);

					// is this the correct material type? if not, skip
					if (!shouldKeep) {
						continue;
					}

					// bind the pipeline
					auto pipeline = pipelineSelectorFunction(materialInstance.mat->GetMat());

					if (pipeline.attributes.position) {
						mainCommandBuffer->SetVertexBuffer(vertexBufferSet.positionBuffer, { .bindingPosition = VTX_POSITION_BINDING });
					}
					if (pipeline.attributes.normal) {
						mainCommandBuffer->SetVertexBuffer(vertexBufferSet.normalBuffer, { .bindingPosition = VTX_NORMAL_BINDING });
					}
					if (pipeline.attributes.tangent) {
						mainCommandBuffer->SetVertexBuffer(vertexBufferSet.tangentBuffer, { .bindingPosition = VTX_TANGENT_BINDING });
					}
					if (pipeline.attributes.bitangent) {
						mainCommandBuffer->SetVertexBuffer(vertexBufferSet.bitangentBuffer, { .bindingPosition = VTX_BITANGENT_BINDING });
					}
					if (pipeline.attributes.uv0) {
						mainCommandBuffer->SetVertexBuffer(vertexBufferSet.uv0Buffer, { .bindingPosition = VTX_UV0_BINDING });
					}
					if (pipeline.attributes.lightmapUV) {
						mainCommandBuffer->SetVertexBuffer(vertexBufferSet.lightmapBuffer, { .bindingPosition = VTX_LIGHTMAP_BINDING });
					}
					mainCommandBuffer->SetIndexBuffer(sharedIndexBuffer);

					mainCommandBuffer->BindRenderPipeline(pipeline.pipeline);

					// this is always needed
					mainCommandBuffer->BindBuffer(transientBuffer, 11, lightDataOffset);
						
					if constexpr (includeLighting) {
						// make textures resident and put them in the right format
						worldOwning->Filter([this](const DirectionalLight& light, const Transform& t) {
                            for(const auto& shadowMap : light.shadowData.shadowMap){
                                mainCommandBuffer->UseResource(shadowMap->GetDefaultView());
                            }
						});
						worldOwning->Filter([this](const SpotLight& light, const Transform& t) {
							mainCommandBuffer->UseResource(light.shadowData.shadowMap->GetDefaultView());
						});

						mainCommandBuffer->BindBuffer(worldOwning->renderData.ambientLightData.GetPrivateBuffer(),12);
						mainCommandBuffer->BindBuffer(worldOwning->renderData.directionalLightData.GetPrivateBuffer(),13);
						mainCommandBuffer->BindBuffer(worldOwning->renderData.pointLightData.GetPrivateBuffer(), 15);
						mainCommandBuffer->BindBuffer(worldOwning->renderData.spotLightData.GetPrivateBuffer(), 17);
                        mainCommandBuffer->BindBuffer(worldOwning->renderData.renderLayers.GetPrivateBuffer(), 28);
						mainCommandBuffer->BindBuffer(worldOwning->renderData.perObjectAttributes.GetPrivateBuffer(), 29);
                        mainCommandBuffer->BindBuffer(worldOwning->renderData.directionalLightPassVarying.GetPrivateBuffer(), 30, camIdx * sizeof(World::DirLightUploadDataPassVarying));
						mainCommandBuffer->SetFragmentTexture(device->GetGlobalBindlessTextureHeap(), 1);	
						mainCommandBuffer->SetFragmentTexture(device->GetGlobalBindlessTextureHeap(), 2);
						mainCommandBuffer->SetFragmentSampler(shadowSampler, 14);
						mainCommandBuffer->SetFragmentSampler(shadowSampler, 31);
						mainCommandBuffer->BindBuffer(lightClusterBuffer, 16);
						
					}
                    if constexpr(transparentMode){
                        assert(target != nullptr); // no target provided!
                        mainCommandBuffer->SetFragmentTexture(target->mlabAccum[0]->GetDefaultView(), 23);
                        mainCommandBuffer->SetFragmentTexture(target->mlabAccum[1]->GetDefaultView(), 24);
                        mainCommandBuffer->SetFragmentTexture(target->mlabAccum[2]->GetDefaultView(), 25);
                        mainCommandBuffer->SetFragmentTexture(target->mlabAccum[3]->GetDefaultView(), 26);
                        mainCommandBuffer->SetFragmentTexture(target->mlabDepth->GetDefaultView(), 27);
                    }
                        
					// set push constant data
					auto pushConstantData = materialInstance.mat->GetPushConstantData();

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
					auto& bufferBindings = materialInstance.mat->GetBufferBindings();
					auto& textureBindings = materialInstance.mat->GetTextureBindings();
					for (int i = 0; i < materialInstance.mat->maxBindingSlots; i++) {
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
					mainCommandBuffer->SetVertexBuffer(drawcommand.cullingBuffer, { .bindingPosition = ENTITY_INPUT_BINDING });
					mainCommandBuffer->BindBuffer(worldTransformBuffer, 10);

					// do the indirect command
					mainCommandBuffer->ExecuteIndirectIndexed({
						.indirectBuffer = drawcommand.indirectBuffer,
						.nDraws = uint32_t(drawcommand.indirectBuffer->getBufferSize() / sizeof(RGL::IndirectIndexedCommand))	// the number of structs in the buffer
					});
				}

				// render particles
				mainCommandBuffer->BeginRenderDebugMarker("Render Particles");
                worldOwning->Filter([this, &viewproj, &particleBillboardMatrices, &currentLightingType, &pipelineSelectorFunction, &lightDataOffset, &worldOwning, &layers, &target, &camIdx, &worldTransformBuffer](const ParticleEmitter& emitter, const Transform& t) {
                    // check if the render layers match
                    auto renderLayers = worldOwning->renderData.renderLayers[emitter.GetOwner().GetID().id];
                    if ((renderLayers & layers) == 0){
                        return;
                    }

					// check if casting shadows
                    auto attributes = worldOwning->renderData.perObjectAttributes[emitter.GetOwner().GetID().id];
					const bool shouldConsider = !currentLightingType.FilterLightBlockers || (currentLightingType.FilterLightBlockers && (attributes & CastsShadowsBit));
					if (!shouldConsider) {
						return;
					}
                    
					if (!emitter.GetVisible()) {
						return;
					}

                    auto sharedParticleImpl = [this, &particleBillboardMatrices, &pipelineSelectorFunction, &worldOwning, &lightDataOffset, &target, &camIdx, &worldTransformBuffer](const ParticleEmitter& emitter, auto&& materialInstance, Ref<ParticleRenderMaterial> material, RGLBufferPtr activeParticleIndexBuffer, bool isLit) {
						auto pipeline = pipelineSelectorFunction(material);


						mainCommandBuffer->BindRenderPipeline(pipeline);
						mainCommandBuffer->BindBuffer(emitter.particleDataBuffer, material->particleDataBufferBinding);
						mainCommandBuffer->BindBuffer(activeParticleIndexBuffer, material->particleAliveIndexBufferBinding);
                        mainCommandBuffer->BindBuffer(emitter.emitterStateBuffer, material->particleEmitterStateBufferBinding);
						mainCommandBuffer->BindBuffer(transientBuffer, material->particleMatrixBufferBinding, particleBillboardMatrices);
						mainCommandBuffer->BindBuffer(worldTransformBuffer, 10);
						mainCommandBuffer->BindBuffer(transientBuffer, 11, lightDataOffset);
						if (isLit) {
							mainCommandBuffer->BindBuffer(worldOwning->renderData.ambientLightData.GetPrivateBuffer(), 12);
							mainCommandBuffer->BindBuffer(worldOwning->renderData.directionalLightData.GetPrivateBuffer(), 13);
							mainCommandBuffer->SetFragmentSampler(shadowSampler, 14);
							mainCommandBuffer->SetFragmentSampler(shadowSampler, 31);
							mainCommandBuffer->BindBuffer(worldOwning->renderData.pointLightData.GetPrivateBuffer(), 15);
							mainCommandBuffer->BindBuffer(worldOwning->renderData.spotLightData.GetPrivateBuffer(), 17);
                            mainCommandBuffer->BindBuffer(worldOwning->renderData.renderLayers.GetPrivateBuffer(), 28);
							mainCommandBuffer->BindBuffer(worldOwning->renderData.perObjectAttributes.GetPrivateBuffer(), 29);
                            mainCommandBuffer->BindBuffer(worldOwning->renderData.directionalLightPassVarying.GetPrivateBuffer(), 30,camIdx * sizeof(World::DirLightUploadDataPassVarying));
							mainCommandBuffer->BindBuffer(lightClusterBuffer, 16);
							mainCommandBuffer->SetFragmentTexture(device->GetGlobalBindlessTextureHeap(), 1);
							mainCommandBuffer->SetFragmentTexture(device->GetGlobalBindlessTextureHeap(), 2);	// redundant on some backends, needed for DX
						}
                        if constexpr(transparentMode){
                            assert(target != nullptr); // no target provided!
                            mainCommandBuffer->SetFragmentTexture(target->mlabAccum[0]->GetDefaultView(), 23);
                            mainCommandBuffer->SetFragmentTexture(target->mlabAccum[1]->GetDefaultView(), 24);
                            mainCommandBuffer->SetFragmentTexture(target->mlabAccum[2]->GetDefaultView(), 25);
                            mainCommandBuffer->SetFragmentTexture(target->mlabAccum[3]->GetDefaultView(), 26);
                            mainCommandBuffer->SetFragmentTexture(target->mlabDepth->GetDefaultView(), 27);
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

								mainCommandBuffer->SetVertexBuffer(sharedPositionBuffer, { .bindingPosition = VTX_POSITION_BINDING });
								mainCommandBuffer->SetVertexBuffer(sharedNormalBuffer, { .bindingPosition = VTX_NORMAL_BINDING });
								mainCommandBuffer->SetVertexBuffer(sharedTangentBuffer, { .bindingPosition = VTX_TANGENT_BINDING });
								mainCommandBuffer->SetVertexBuffer(sharedBitangentBuffer, { .bindingPosition = VTX_BITANGENT_BINDING });
								mainCommandBuffer->SetVertexBuffer(sharedUV0Buffer, { .bindingPosition = VTX_UV0_BINDING });
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
				mainCommandBuffer->EndRenderDebugMarker();
			};

			// do culling operations
			if constexpr (runCulling) {
				mainCommandBuffer->BeginComputeDebugMarker("Cull Static Meshes");
				cullTheRenderData(worldOwning->renderData.staticMeshRenderData);
				mainCommandBuffer->EndComputeDebugMarker();
				if (skeletalPrepareResult.skeletalMeshesExist) {
					cullSkeletalMeshes(viewproj, pyramid);
				}
			}


			// do rendering operations
			mainCommandBuffer->BeginRendering(renderPass);

            // make bindless resources resident

            if constexpr(includeLighting){
                if (wantsSSAO){
                    mainCommandBuffer->UseResource(ssaoTargetTexture->GetDefaultView());
                }
                worldOwning->Filter([this](const AmbientLight& light){
                    if (light.environment.has_value()){
                        mainCommandBuffer->UseResource(light.environment->outputTexture->GetView());
                    }
                });
            }
        
			mainCommandBuffer->BeginRenderDebugMarker("Render Static Meshes");
			BufferSet vertexBuffers{
				.positionBuffer = sharedPositionBuffer,
				.normalBuffer = sharedNormalBuffer,
				.tangentBuffer = sharedTangentBuffer,
				.bitangentBuffer = sharedBitangentBuffer,
				.uv0Buffer = sharedUV0Buffer,
				.lightmapBuffer = sharedLightmapUVBuffer,
			};
			renderTheRenderData(worldOwning->renderData.staticMeshRenderData, vertexBuffers, lightingFilter);
			mainCommandBuffer->EndRenderDebugMarker();

			if (skeletalPrepareResult.skeletalMeshesExist) {
				mainCommandBuffer->BeginRenderDebugMarker("Render Skinned Meshes");
				vertexBuffers = {
					.positionBuffer = sharedSkinnedPositionBuffer,
					.normalBuffer = sharedSkinnedNormalBuffer,
					.tangentBuffer = sharedSkinnedTangentBuffer,
					.bitangentBuffer = sharedSkinnedBitangentBuffer,
					.uv0Buffer = sharedSkinnedUV0Buffer,
					.lightmapBuffer = nullptr,
				};
				renderTheRenderData(worldOwning->renderData.skinnedMeshRenderData, vertexBuffers, lightingFilter);
				mainCommandBuffer->EndRenderDebugMarker();
			}

			mainCommandBuffer->EndRendering();
			};

		struct lightViewProjResult {
			glm::mat4 lightProj, lightView;
			glm::vec3 camPos = glm::vec3{ 0,0,0 };
			DepthPyramid depthPyramid;
			RGLTexturePtr shadowmapTexture;
		};

		// render shadowmaps only once per light


		// the generic shadowmap rendering function
		RVE_PROFILE_SECTION(encode_shadowmaps, "Render Encode Shadowmaps");
        auto renderLightShadowmap = [this, &renderFromPerspective, &worldOwning](auto&& lightStore, uint32_t numShadowmaps, auto&& genLightViewProjAtIndex, auto&& postshadowmapFunction, auto&& shouldRendershadowmap) {
			if (lightStore.DenseSize() <= 0) {
				return;
			}
			mainCommandBuffer->BeginRenderDebugMarker("Render shadowmap");
			for (uint32_t i = 0; i < lightStore.DenseSize(); i++) {
				auto& light = lightStore.GetAtDenseIndex(i);
				if (!light.castsShadows) {
					continue;	// don't do anything if the light doesn't cast
				}
				auto sparseIdx = lightStore.GetSparseIndexForDense(i);
                auto owner = Entity({sparseIdx, worldOwning->VersionForEntity(sparseIdx)}, worldOwning.get());

				using lightadt_t = std::remove_reference_t<decltype(lightStore)>;

				for (uint8_t sm_i = 0; sm_i < numShadowmaps; sm_i++) {
                    if (!shouldRendershadowmap(sm_i, owner)){
                        continue;
                    }
					lightViewProjResult lightMats = genLightViewProjAtIndex(sm_i, i, light, owner);

					auto lightSpaceMatrix = lightMats.lightProj * lightMats.lightView;

					auto shadowTexture = lightMats.shadowmapTexture;

					shadowRenderPass->SetDepthAttachmentTexture(shadowTexture->GetDefaultView());
					auto shadowMapSize = shadowTexture->GetSize().width;
					renderFromPerspective.template operator()<false,false>(lightSpaceMatrix, lightMats.lightView, lightMats.lightProj, lightMats.camPos, {}, shadowRenderPass, [](auto&& mat) {
						return mat->GetShadowRenderPipeline();
                    }, { 0, 0, shadowMapSize,shadowMapSize }, { .Lit = true, .Unlit = true, .FilterLightBlockers = true, .Opaque = true }, lightMats.depthPyramid, light.shadowLayers, nullptr, {});

				}
				postshadowmapFunction(owner);
			}
			mainCommandBuffer->EndRenderDebugMarker();
		};

		RVE_PROFILE_SECTION(encode_spot_shadows,"Render Encode Spot Shadows");
		const auto spotlightShadowMapFunction = [](uint8_t index, uint32_t denseIdx, const RavEngine::World::SpotLightDataUpload& light, Entity owner) {

			auto camPos = light.worldTransform * glm::vec4(0, 0, 0, 1);
			auto& origLight = owner.GetComponent<SpotLight>();
            auto viewMat = origLight.CalcViewMatrix(light.worldTransform);
			auto projMat = origLight.CalcProjectionMatrix();

			return lightViewProjResult{
				.lightProj = projMat,
				.lightView = viewMat,
				.camPos = camPos,
				.depthPyramid = origLight.shadowData.pyramid,
				.shadowmapTexture = origLight.shadowData.shadowMap,
			};
        };
        
		renderLightShadowmap(worldOwning->renderData.spotLightData, 1,
			spotlightShadowMapFunction,
			[](Entity unused) {},
            [](uint32_t i, auto&& entity){ return true;}
		);
		RVE_PROFILE_SECTION_END(encode_spot_shadows);

		RVE_PROFILE_SECTION(encode_point_shadows, "Render Encode Point Shadows");
		constexpr auto pointLightShadowmapFunction = [](uint8_t index, uint32_t denseIdx, const RavEngine::World::PointLightUploadData& light, Entity owner) {
			auto camPos = light.position;

			auto& origLight = owner.GetComponent<PointLight>();

			return lightViewProjResult{
				.lightProj = origLight.CalcProjectionMatrix(),
				.lightView = PointLight::CalcViewMatrix(light.position,index),
				.camPos = camPos,
				.depthPyramid = origLight.shadowData.cubePyramids[index],
				.shadowmapTexture = origLight.shadowData.cubeShadowmaps[index],
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
            },
        [](uint32_t i, auto&& entity){ return true;}
        );
		RVE_PROFILE_SECTION_END(encode_point_shadows);
		RVE_PROFILE_SECTION_END(encode_shadowmaps);

		RVE_PROFILE_SECTION(allViews, "Render Encode All Views");
		for (const auto& view : screenTargets) {
			currentRenderSize = view.pixelDimensions;
			auto nextImgSize = view.pixelDimensions;
			auto& target = view.collection;

            auto renderLitPass_Impl = [this,&target, &renderFromPerspective,&renderLightShadowmap,&worldOwning, &camIdx]<bool transparentMode = false>(auto&& camData, auto&& fullSizeViewport, auto&& fullSizeScissor, auto&& renderArea) {
				// directional light shadowmaps
                

				if constexpr (!transparentMode) {
					RVE_PROFILE_SECTION(dirShadow, "Render Encode Dirlight shadowmap");
					mainCommandBuffer->BeginRenderDebugMarker("Render Directional Lights");
                
                    
                    const auto dirlightShadowmapDataFunction = [&camData, &worldOwning, &camIdx](uint8_t index, uint32_t denseIdx, const RavEngine::World::DirLightUploadData& light, Entity owner) {
                        
                        auto& origLight = owner.GetComponent<DirectionalLight>();
                        
                        const auto& hostdata = worldOwning->renderData.directionalLightPassVaryingHostOnly;
                        
						return lightViewProjResult{
                            .lightProj = hostdata[camIdx + denseIdx].lightProj[index],
                            .lightView = hostdata[camIdx + denseIdx].lightview[index],
							.camPos = camData.camPos,
							.depthPyramid = origLight.shadowData.pyramid[index],
							.shadowmapTexture = origLight.shadowData.shadowMap[index],
						};
                    };

					renderLightShadowmap(worldOwning->renderData.directionalLightData, MAX_CASCADES,
						dirlightShadowmapDataFunction,
						[](Entity unused) {},
                         [](uint32_t index, const Entity& owner){
                            auto& origLight = owner.GetComponent<DirectionalLight>();
                            if (index >= origLight.numCascades ){
                                return false;     // only render the requested number of cascades
                            }
                            return true;
                        }
					);
					mainCommandBuffer->EndRenderDebugMarker();
					RVE_PROFILE_SECTION_END(dirShadow);

					// render depth prepass
					mainCommandBuffer->BeginRenderDebugMarker("Lit Opaque Depth Prepass");
					renderFromPerspective.template operator() < true, transparentMode, true > (camData.viewProj, camData.viewOnly, camData.projOnly, camData.camPos, camData.zNearFar, depthPrepassRenderPassLit, [](auto&& mat) {
						return mat->GetDepthPrepassPipeline();
						}, renderArea, { .Lit = true, .Transparent = transparentMode, .Opaque = !transparentMode, }, target.depthPyramid, camData.layers, & target, {});
					mainCommandBuffer->EndRenderDebugMarker();

					// render SSAO
					if (camData.indirectSettings.SSAOEnabled) {
						mainCommandBuffer->BeginRenderDebugMarker("SSAO");
						ssaoPassClear->SetAttachmentTexture(0, target.ssaoOutputTexture1->GetDefaultView());
						ssaoPassClear->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());
						ssgiPassNoClear->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());

						mainCommandBuffer->BeginRendering(ssaoPassClear);
						mainCommandBuffer->BindRenderPipeline(ssaoPipeline);
						mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
						mainCommandBuffer->SetFragmentTexture(target.depthStencil->GetDefaultView(), 1);
						mainCommandBuffer->SetFragmentTexture(target.viewSpaceNormalsTexture->GetDefaultView(), 2);

						const auto size = target.ssaoOutputTexture1->GetSize();
						{
							SSAOUBO ssgiubo{
								.proj = camData.projOnly,
								.invProj = glm::inverse(camData.projOnly),
								.screenDim = {size.width, size.height},
							};
							mainCommandBuffer->SetFragmentBytes(ssgiubo, 0);
						}

						mainCommandBuffer->SetVertexBuffer(screenTriVerts);
						mainCommandBuffer->Draw(3);
						mainCommandBuffer->EndRendering();

						// blur it horizontally --> tex2
						mainCommandBuffer->BeginCompute(blurKernelX);
						mainCommandBuffer->SetComputeTexture(target.ssaoOutputTexture1->GetDefaultView(), 0);
						mainCommandBuffer->SetComputeTexture(target.ssaoOutputTexture2->GetDefaultView(), 1);
						mainCommandBuffer->DispatchCompute(std::ceil(size.width / 64.0), (size.height), 1, 64, 1, 1);

						mainCommandBuffer->EndCompute();

						// blur it vertically --> tex1
						mainCommandBuffer->BeginCompute(blurKernelY);
						mainCommandBuffer->SetComputeTexture(target.ssaoOutputTexture2->GetDefaultView(), 0);
						mainCommandBuffer->SetComputeTexture(target.ssaoOutputTexture1->GetDefaultView(), 1);
						mainCommandBuffer->DispatchCompute(std::ceil(size.height / 64.0), (size.width), 1, 64, 1, 1);
						mainCommandBuffer->EndCompute();

						mainCommandBuffer->EndRenderDebugMarker();
					}
				}

				// render with shading

				renderFromPerspective.template operator()<true, transparentMode, transparentMode>(camData.viewProj, camData.viewOnly, camData.projOnly, camData.camPos, camData.zNearFar, transparentMode ? litTransparentPass : litRenderPass, [](auto&& mat) {
					return mat->GetMainRenderPipeline();
					}, renderArea, { .Lit = true, .Transparent = transparentMode, .Opaque = !transparentMode, }, target.depthPyramid, camData.layers, & target, {.enableSSAO = camData.indirectSettings.SSAOEnabled});
#if 0
				if (!transparentMode) {

					if (camData.indirectSettings.SSAOEnabled || camData.indirectSettings.SSGIEnabled) {
						constexpr auto divFacForMip = [](uint32_t mip) {
							return std::pow(2, mip);
							};
						const auto size = target.ssgiOutputTexture->GetSize();

						mainCommandBuffer->BeginRenderDebugMarker("SSGI");
						{
							ssgiPassClear->SetAttachmentTexture(0, target.ssgiOutputTexture->GetViewForMip(1));	// not rendering to base mip
							ssgiPassClear->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());
							ssgiPassNoClear->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());

							mainCommandBuffer->BeginRendering(ssgiPassClear);
							mainCommandBuffer->BindRenderPipeline(ssgipipeline);
							mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
							mainCommandBuffer->SetFragmentTexture(target.depthStencil->GetDefaultView(), 1);
							mainCommandBuffer->SetFragmentTexture(target.viewSpaceNormalsTexture->GetDefaultView(), 2);
							mainCommandBuffer->SetFragmentTexture(target.radianceTexture->GetDefaultView(), 3);

							const auto divFac = divFacForMip(1);
							{
								SSGIUBO ssgiubo{
									.projection = camData.projOnly,
									.invProj = glm::inverse(camData.projOnly),
									.outputDim = {size.width / divFac, size.height / divFac},
									.sampleCount = 4,
									.sampleRadius = 4.0,
									.sliceCount = 4,
									.hitThickness = 0.5,
								};
								mainCommandBuffer->SetFragmentBytes(ssgiubo, 0);
							}

							mainCommandBuffer->SetVertexBuffer(screenTriVerts);
							mainCommandBuffer->Draw(3);
							mainCommandBuffer->EndRendering();
						}


						// dealing with the results
						// first, downsample AO and GI one step
						{
							ssgiPassClear->SetAttachmentTexture(0, target.ssgiOutputTexture->GetViewForMip(2));

							mainCommandBuffer->BeginRendering(ssgiPassClear);
							mainCommandBuffer->BindRenderPipeline(ssgiDownsamplePipeline);
							mainCommandBuffer->SetFragmentSampler(textureSampler, 1);
							mainCommandBuffer->SetFragmentTexture(target.ssgiOutputTexture->GetViewForMip(1), 0);

							const auto divFac = divFacForMip(2);

							DownsampleUBO ubo{
								.targetDim = {0,0,size.width / divFac, size.height / divFac},
							};
							mainCommandBuffer->SetFragmentBytes(ubo, 0);

							mainCommandBuffer->SetVertexBuffer(screenTriVerts);
							mainCommandBuffer->Draw(3);

							mainCommandBuffer->EndRendering();
						}

						// next, upsample the AO from mip 2 to mip 0
						if (camData.indirectSettings.SSAOEnabled)
						{
							mainCommandBuffer->BeginRenderDebugMarker("Upsample AO");
							for (int i = 2; i >= 1; i--) {
								ssgiPassNoClear->SetAttachmentTexture(0, target.ssgiOutputTexture->GetViewForMip(i - 1));

								mainCommandBuffer->BeginRendering(ssgiPassNoClear);
								mainCommandBuffer->BindRenderPipeline(aoUpsamplePipeline);
								mainCommandBuffer->SetFragmentSampler(textureSampler, 1);
								mainCommandBuffer->SetFragmentTexture(target.ssgiOutputTexture->GetViewForMip(i), 0);

								const auto divFac = divFacForMip(i - 1);

								UpsampleUBO ubo{
									.targetDim = {0,0,size.width / divFac, size.height / divFac},
									.filterRadius = 0.005
								};
								mainCommandBuffer->SetFragmentBytes(ubo, 0);

								mainCommandBuffer->SetVertexBuffer(screenTriVerts);
								mainCommandBuffer->Draw(3);

								mainCommandBuffer->EndRendering();
							}
							mainCommandBuffer->EndRenderDebugMarker();
						}


						// downscale AO + GI the rest of the way upscale 
						if (camData.indirectSettings.SSGIEnabled) {
							mainCommandBuffer->BeginRenderDebugMarker("Downsample");
							const uint32_t numMips = std::min<uint32_t>(std::log2(std::min(size.width, size.height)), maxssgimips);
							for (int i = 3; i < numMips; i++) {
								ssgiPassClear->SetAttachmentTexture(0, target.ssgiOutputTexture->GetViewForMip(i));

								mainCommandBuffer->BeginRendering(ssgiPassClear);
								mainCommandBuffer->BindRenderPipeline(ssgiDownsamplePipeline);
								mainCommandBuffer->SetFragmentSampler(textureSampler, 1);
								mainCommandBuffer->SetFragmentTexture(target.ssgiOutputTexture->GetViewForMip(i - 1), 0);

								const auto divFac = divFacForMip(i);

								DownsampleUBO ubo{
									.targetDim = {0,0,size.width / divFac, size.height / divFac},
								};
								mainCommandBuffer->SetFragmentBytes(ubo, 0);

								mainCommandBuffer->SetVertexBuffer(screenTriVerts);
								mainCommandBuffer->Draw(3);

								mainCommandBuffer->EndRendering();
							}
							mainCommandBuffer->EndRenderDebugMarker();
							mainCommandBuffer->BeginRenderDebugMarker("Upsample");
							for (int i = numMips - 1; i > 0; i--) {
								ssgiPassNoClear->SetAttachmentTexture(0, target.ssgiOutputTexture->GetViewForMip(i - 1));

								mainCommandBuffer->BeginRendering(ssgiPassNoClear);
								mainCommandBuffer->BindRenderPipeline(i == 1 ? ssgiUpsamplePipleineFinalStep : ssgiUpsamplePipeline);
								mainCommandBuffer->SetFragmentSampler(textureSampler, 1);
								mainCommandBuffer->SetFragmentTexture(target.ssgiOutputTexture->GetViewForMip(i), 0);

								const auto divFac = divFacForMip(i - 1);

								UpsampleUBO ubo{
									.targetDim = {0,0,size.width / divFac, size.height / divFac},
									.filterRadius = 0.005
								};
								mainCommandBuffer->SetFragmentBytes(ubo, 0);

								mainCommandBuffer->SetVertexBuffer(screenTriVerts);
								mainCommandBuffer->Draw(3);

								mainCommandBuffer->EndRendering();
							}
							mainCommandBuffer->EndRenderDebugMarker();
						}

						mainCommandBuffer->EndRenderDebugMarker();
					}
					// ambient and SSGI
					ssgiAmbientApplyPass->SetAttachmentTexture(0, target.lightingTexture->GetDefaultView());
					ssgiAmbientApplyPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());

					mainCommandBuffer->BeginRenderDebugMarker("Ambient + GI");
					mainCommandBuffer->BeginRendering(ssgiAmbientApplyPass);
					mainCommandBuffer->BindRenderPipeline(ambientSSGIApplyPipeline);

					mainCommandBuffer->SetFragmentSampler(textureSampler, 0);
					mainCommandBuffer->SetFragmentTexture(target.lightingScratchTexture->GetDefaultView(), 1);	// albedo color
					mainCommandBuffer->SetFragmentTexture(target.radianceTexture->GetDefaultView(), 2);
					mainCommandBuffer->SetFragmentTexture(target.ssgiOutputTexture->GetDefaultView(), 3);
					mainCommandBuffer->SetFragmentTexture(target.viewSpaceNormalsTexture->GetDefaultView(), 4);
					mainCommandBuffer->SetFragmentTexture(device->GetGlobalBindlessTextureHeap(), 2);

					AmbientSSGIApplyUBO ubo{
						.invView = glm::transpose(camData.viewOnly),
						.ambientLightCount = worldOwning->renderData.ambientLightData.DenseSize(),
						.ssaoStrength = camData.indirectSettings.ssaoStrength,
					};
					if (camData.indirectSettings.SSAOEnabled) {
						ubo.options |= AmbientSSGIApplyUBO::SSAOBIT;
					}
					if (camData.indirectSettings.SSGIEnabled) {
						ubo.options |= AmbientSSGIApplyUBO::SSGIBIT;
					}
					mainCommandBuffer->SetFragmentBytes(ubo, 0);

					mainCommandBuffer->BindBuffer(worldOwning->renderData.ambientLightData.GetPrivateBuffer(), 10);

					mainCommandBuffer->SetVertexBuffer(screenTriVerts);
					mainCommandBuffer->Draw(3);

					mainCommandBuffer->EndRendering();
					mainCommandBuffer->EndRenderDebugMarker();
				}
#endif
			};

            auto renderLitPass = [&renderLitPass_Impl](auto&& camData, auto&& fullSizeViewport, auto&& fullSizeScissor, auto&& renderArea) {
				renderLitPass_Impl(camData, fullSizeViewport, fullSizeScissor, renderArea);
			};

			auto renderUnlitPass = [this, &target, &renderFromPerspective, &renderLightShadowmap, &worldOwning](auto&& camData, auto&& fullSizeViewport, auto&& fullSizeScissor, auto&& renderArea) {
				// render depth prepass
				mainCommandBuffer->BeginRenderDebugMarker("Unlit Opaque Depth Prepass");
				renderFromPerspective.template operator() < false, false, true > (camData.viewProj, camData.viewOnly, camData.projOnly, camData.camPos, camData.zNearFar, depthPrepassRenderPass, [](auto&& mat) {
					return mat->GetDepthPrepassPipeline();
					}, renderArea, { .Unlit = true, .Opaque = true }, target.depthPyramid, camData.layers, & target, {});
				mainCommandBuffer->EndRenderDebugMarker();

				// render color
				renderFromPerspective.template operator() < false, false, false> (camData.viewProj, camData.viewOnly, camData.projOnly, camData.camPos, {}, unlitRenderPass, [](auto&& mat) {
					return mat->GetMainRenderPipeline();
					}, renderArea, { .Unlit = true, .Opaque = true }, target.depthPyramid, camData.layers, & target, {});
			};

			auto renderLitPassTransparent = [&renderLitPass_Impl](auto&& camData, auto&& fullSizeViewport, auto&& fullSizeScissor, auto&& renderArea) {
				renderLitPass_Impl.template operator()<true>(camData, fullSizeViewport, fullSizeScissor, renderArea);
			};

            auto renderFinalPass = [this, &target, &worldOwning, &view, &guiScaleFactor, &nextImgSize, &renderFromPerspective,&renderSkybox](auto&& camData, auto&& fullSizeViewport, auto&& fullSizeScissor, auto&& renderArea) {
               

				// render unlits with transparency
				RVE_PROFILE_SECTION(unlittrans, "Encode Unlit Transparents");
				unlitTransparentPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());
				renderFromPerspective.template operator() < false, true > (camData.viewProj, camData.viewOnly, camData.projOnly, camData.camPos, {}, unlitTransparentPass, [](auto&& mat) {
					return mat->GetMainRenderPipeline();
					}, renderArea, { .Unlit = true, .Transparent = true }, target.depthPyramid, camData.layers, & target, {});
				RVE_PROFILE_SECTION_END(unlittrans);
                
                // then do the skybox, if one is defined.
                if (worldOwning->skybox && worldOwning->skybox->skyMat && worldOwning->skybox->skyMat->GetMat()->renderPipeline) {
					mainCommandBuffer->BeginRendering(unlitRenderPass);
					mainCommandBuffer->BeginRenderDebugMarker("Skybox");
					renderSkybox(worldOwning->skybox->skyMat->GetMat()->renderPipeline, glm::transpose(camData.viewOnly), camData.camPos, deg_to_rad(camData.fov), fullSizeViewport, fullSizeScissor, [] {});
					mainCommandBuffer->EndRenderDebugMarker();
					mainCommandBuffer->EndRendering();
                }


				// apply transparency

				transparencyApplyPass->SetAttachmentTexture(0, target.lightingTexture->GetDefaultView());

				mainCommandBuffer->BeginRenderDebugMarker("Apply All Transparency");
				mainCommandBuffer->BeginRendering(transparencyApplyPass);

				mainCommandBuffer->BindRenderPipeline(transparencyApplyPipeline);
                
                for(const auto& [i, tx] : Enumerate(target.mlabAccum)){
                    mainCommandBuffer->SetFragmentTexture(tx->GetDefaultView(), i);
                }
                
				mainCommandBuffer->SetVertexBuffer(screenTriVerts);
				mainCommandBuffer->Draw(3);

				mainCommandBuffer->EndRendering();
				mainCommandBuffer->EndRenderDebugMarker();


                // afterwards render the post processing effects
				RVE_PROFILE_SECTION(postfx, "Encode Post Processing Effects");
                uint32_t totalPostFXRendered = 0;
                RGL::TextureView currentInput = target.lightingTexture->GetDefaultView();
                RGL::TextureView altInput = target.lightingScratchTexture->GetDefaultView();
				mainCommandBuffer->BeginRenderDebugMarker("Post processing");
                
                for(const auto& effect : camData.postProcessingEffects->effects){
                    if (!effect->enabled){
                        continue;
                    }
					
                    effect->Preamble({ int(fullSizeViewport.width), int(fullSizeViewport.height) });
                    for(const auto pass : effect->passes){
						BasePushConstantUBO baseUbo{
							.dim = {0,0, fullSizeViewport.width, fullSizeViewport.height}
						};
						bool isUsingFinalOutput = pass->outputConfiguration == PostProcessOutput::EngineColor;

						auto activePass = pass->clearOutputBeforeRendering ? postProcessRenderPassClear : postProcessRenderPass;

						if (isUsingFinalOutput) {
							activePass->SetAttachmentTexture(0, altInput);
						}
						else {
							activePass->SetAttachmentTexture(0, pass->outputBinding);
							auto size = pass->GetUserDefinedOutputSize();
							baseUbo.dim = {0,0, size.width, size.height };
						}
                        mainCommandBuffer->BeginRendering(activePass);
                        mainCommandBuffer->BindRenderPipeline(pass->GetEffect()->GetPipeline());
						mainCommandBuffer->SetViewport({
							.x = 0,
							.y = 0,
							.width = float(baseUbo.dim.z),
							.height = float(baseUbo.dim.w),
						});
						mainCommandBuffer->SetScissor({
							.offset = {
								0,0
							},
							.extent = {
								uint32_t(baseUbo.dim.z), 
								uint32_t(baseUbo.dim.w)
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

				mainCommandBuffer->EndRenderDebugMarker();
                
				RVE_PROFILE_SECTION_END(postfx);
                auto blitSource = totalPostFXRendered % 2 == 0 ? target.lightingTexture->GetDefaultView() : target.lightingScratchTexture->GetDefaultView();
                
				// the final on-screen render pass
// contains the results of the previous stages, as well as the UI, skybox and any debugging primitives
				
				glm::ivec4 viewRect {0, 0, nextImgSize.width, nextImgSize.height};

				LightToFBUBO fbubo{
					.viewRect = viewRect
				};

				// does the camera have a tonemapper set?

				const TonemapPassInstance* tonemapPass = nullptr;
				if (tonemapPass = static_cast<const TonemapPassInstance*>(camData.tonemap)) {
					// we got a tonemap, continue
				}
				else {
					// use the dummy tonemap
					tonemapPass = dummyTonemap.get();
				}

				auto tonemapMaterial = tonemapPass->GetEffect();

                finalRenderPassNoDepth->SetAttachmentTexture(0, target.finalFramebuffer->GetDefaultView());
				mainCommandBuffer->BeginRendering(finalRenderPassNoDepth);
				mainCommandBuffer->BeginRenderDebugMarker("Tonemap");
				// start with the results of lighting
				mainCommandBuffer->BindRenderPipeline(tonemapMaterial->GetPipeline());
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
				RVE_PROFILE_FN_N("generatePyramid");
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
				RVE_PROFILE_FN_N("genPyramidForLight");
				for (uint32_t i = 0; i < lightStore.DenseSize(); i++) {
					const auto& light = lightStore.GetAtDenseIndex(i);
					auto sparseIdx = lightStore.GetSparseIndexForDense(i);
                    auto owner = Entity({sparseIdx, worldOwning->VersionForEntity(sparseIdx)}, worldOwning.get());

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
                genPyramidForLight(worldOwning->renderData.directionalLightData, ptr, MAX_CASCADES, [](uint32_t index, auto&& origLight) {
                    struct ReturnData {
                        DepthPyramid pyramid;
                        RGLTexturePtr shadowMap;
                    };
                    return ReturnData{origLight.shadowData.pyramid[index], origLight.shadowData.shadowMap[index]};
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
			depthPrepassRenderPassLit->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());
			depthPrepassRenderPassLit->SetAttachmentTexture(0,target.viewSpaceNormalsTexture->GetDefaultView());
			depthPrepassRenderPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());


			litRenderPass->SetAttachmentTexture(0, target.lightingTexture->GetDefaultView());
			litRenderPass->SetAttachmentTexture(1, target.radianceTexture->GetDefaultView());
			litRenderPass->SetAttachmentTexture(2, target.lightingScratchTexture->GetDefaultView());
			litRenderPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());

			litClearRenderPass->SetAttachmentTexture(0, target.lightingTexture->GetDefaultView());
			litClearRenderPass->SetAttachmentTexture(1, target.radianceTexture->GetDefaultView());
			litClearRenderPass->SetAttachmentTexture(2, target.lightingScratchTexture->GetDefaultView());
			litClearRenderPass->SetAttachmentTexture(3, target.viewSpaceNormalsTexture->GetDefaultView());
			litClearRenderPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());

			mainCommandBuffer->BeginRenderDebugMarker("Lit Pass Opaque");

			mainCommandBuffer->BeginRendering(litClearRenderPass);
			mainCommandBuffer->EndRendering();
			
			RVE_PROFILE_SECTION(lit, "Encode Lit Pass Opaque");
			auto camIdxHere = camIdx;
			for (const auto& camdata : view.camDatas) {
				doPassWithCamData(camdata, renderLitPass);
                camIdx++;
			}

			camIdx = camIdxHere;	// revert because we do another pass
			mainCommandBuffer->EndRenderDebugMarker();
			RVE_PROFILE_SECTION_END(lit);

			//render unlits
			// must be done before transparents because these write depth
			unlitRenderPass->SetAttachmentTexture(0, target.lightingTexture->GetDefaultView());
			unlitRenderPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());
			RVE_PROFILE_SECTION(unlit, "Encode Unlit Opaques");
			for (const auto& camdata : view.camDatas) {
				doPassWithCamData(camdata, renderUnlitPass);
			}
			RVE_PROFILE_SECTION_END(unlit);


            for(const auto& [i, tx] : Enumerate(target.mlabAccum)){
                transparentClearPass->SetAttachmentTexture(i, tx->GetDefaultView());
            }
            transparentClearPass->SetAttachmentTexture(4, target.mlabDepth->GetDefaultView());

			mainCommandBuffer->BeginRenderDebugMarker("Lit Pass Transparent");
			mainCommandBuffer->BeginRendering(transparentClearPass);
			mainCommandBuffer->EndRendering();

            litTransparentPass->SetDepthAttachmentTexture(target.depthStencil->GetDefaultView());
			RVE_PROFILE_SECTION(littrans, "Encode Lit Pass Transparent");
			for (const auto& camdata : view.camDatas) {
				doPassWithCamData(camdata, renderLitPassTransparent);
				camIdx++;
			}
			mainCommandBuffer->EndRenderDebugMarker();
			RVE_PROFILE_SECTION_END(littrans);
			camIdx = camIdxHere;
            
			
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
    
        // sync the transient command buffer
        if (transientOffset > 0){
			transientCommandBuffer->Reset();
            transientCommandBuffer->Begin();
            transientCommandBuffer->CopyBufferToBuffer({
                .buffer = transientStagingBuffer,
                .offset = 0
            }, {
                .buffer = transientBuffer,
                .offset = 0
            }, transientOffset);
            transientCommandBuffer->End();
            transientCommandBuffer->Commit({});
			transientSubmittedLastFrame = true;
        }
		else {
			transientSubmittedLastFrame = false;
		}

		if (transformSyncCommandBufferNeedsCommit) {
			RVE_PROFILE_SECTION(transient_wait,"Wait for Transient Buffer to complete");
			transformSyncCommandBuffer->BlockUntilCompleted();
			RVE_PROFILE_SECTION_END(transient_wait);
		}
   

		frameCount++;

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
