#pragma once
#include "Tonemap.hpp"

namespace RavEngine {

	struct DummyTonemap : public TonemapPass {
		DummyTonemap();
	};

	struct DummyTonemapInstance : public TonemapPassInstance {
		DummyTonemapInstance(Ref<DummyTonemap> pass);
	};

}