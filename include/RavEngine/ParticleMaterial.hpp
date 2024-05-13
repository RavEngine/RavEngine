#pragma once
#include <RGL/Types.hpp>
#include "mathtypes.hpp"
#include <string>

namespace RavEngine {

	struct ParticleEngineData {
		quaternion rotation{ 0,0,0,0 };
		vector3 pos{ 0 }, scale{ 0 };
	};


	struct ParticleUpdateUBO {
		float fpsScale;
	};

	// Subclass this to make custom particle materials
	struct ParticleMaterial {
		friend class RenderEngine;
		ParticleMaterial(const std::string& initShaderName, const std::string& updateShaderName);

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
	};

}