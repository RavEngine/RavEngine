#if !RVE_SERVER
#include "Tonemap.hpp"
namespace RavEngine {
	TonemapPass::TonemapPass(const std::string_view name, const ScreenEffectConfig& config) : ScreenEffectBase(name, config, {
	.outputFormat = RGL::TextureFormat::BGRA8_Unorm
		}
	) {
	}
}
#endif