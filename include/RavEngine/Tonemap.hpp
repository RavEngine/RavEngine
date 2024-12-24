#pragma once
#include "ScreenEffect.hpp"

namespace RavEngine {
	struct TonemapPass : public ScreenEffectBase {
		TonemapPass(const std::string_view name, const ScreenEffectConfig& config = {});
	};

	using TonemapPassInstance = ScreenEffectPassInstance<TonemapPass>;

}