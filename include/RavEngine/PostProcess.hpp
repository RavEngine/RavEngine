#pragma once
#include "ScreenEffect.hpp"

namespace RavEngine {

	struct PostProcessPass : public ScreenEffectBase {
		PostProcessPass(const std::string_view name, const ScreenEffectConfig& config = {});
	};

	using PostProcessPassInstance = ScreenEffectPassInstance<PostProcessPass>;
	using PostProcessEffect = ScreenEffect<PostProcessPassInstance>;

	struct PostProcessEffectStack {
		Vector<std::unique_ptr<PostProcessEffect>> effects;
	};
}