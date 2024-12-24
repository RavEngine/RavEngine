#if !RVE_SERVER
#include "PostProcess.hpp"
#include "RenderEngine.hpp"


namespace RavEngine {
	PostProcessPass::PostProcessPass(const std::string_view name, const ScreenEffectConfig& config) : ScreenEffectBase(name, config, {
	.outputFormat = RenderEngine::colorTexFormat
		}) {
	}
}
#endif