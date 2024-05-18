#pragma once
#include <RGL/Types.hpp>
#include "mathtypes.hpp"
#include <string>
#include <variant>

namespace RavEngine {

	struct BilboardParticleEngineData {
		vector3 pos{ 0 };
		vector2 scale{ 0 };
		uint32_t animationFrame = 0;
	};


	struct ParticleUpdateUBO {
		float fpsScale;
	};

	struct ParticleBillboardUBO {
		matrix4 viewProj;
		vector2 spritesheetDim;
		vector2 spritesheetFrameDim;
	};

	struct ParticleQuadVert {
		vector2 pos;
	};

	// Subclass this to make custom particle materials
	struct ParticleMaterial {
		friend class RenderEngine;

		ParticleMaterial(const std::string& initShaderName, const std::string& updateShaderName, const std::string& particleVS, const std::string& particleFS);

		/**
		* @return The number of bytes required for your material's per-particle user data. This data is placed immediately after the engine's required particle data for each particle
		*/
		virtual uint16_t ParticleUserDataSize() const {
			return 0;
		}

		const auto GetInitShader() const {
			return userInitPipeline;
		}

		const auto GetUpdateShader() const {
			return userUpdatePipeline;
		}

	private:
		RGLComputePipelinePtr userInitPipeline, userUpdatePipeline;
		RGLRenderPipelinePtr userRenderPipeline;
	};


}