#pragma once
#include "Tonemap.hpp"

namespace RavEngine {

	// please don't use this one. It's just a clamp function.
	struct DummyTonemap : public TonemapPass {
		DummyTonemap();
	};

	struct DummyTonemapInstance : public TonemapPassInstance {
		DummyTonemapInstance(Ref<DummyTonemap> pass);
	};

	struct ACESTonemap : public TonemapPass {
		ACESTonemap();
	};

	struct ACESTonemapInstance : public TonemapPassInstance {
		ACESTonemapInstance(Ref<ACESTonemap> pass);
	};

	struct ReinhardJodieTonemap : public TonemapPass {
		ReinhardJodieTonemap();
	};

	struct ReinhardJodieTonemapInstance : public TonemapPassInstance {
		ReinhardJodieTonemapInstance(Ref<ReinhardJodieTonemap> pass);
	};

	struct AGXTonemap : public TonemapPass {
		AGXTonemap();
	};

	struct AGXTonemapInstance : public TonemapPassInstance {
		AGXTonemapInstance(Ref<AGXTonemap> pass);
	};
}