#include "BuiltinTonemap.hpp"

static std::vector<RGL::PipelineLayoutDescriptor::LayoutBindingDesc> basicBindings{
             {
                .binding = 0,
                .type = RGL::BindingType::Sampler,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },
            {
                .binding = 1,
                .type = RGL::BindingType::SampledImage,
                .stageFlags = RGL::BindingVisibility::Fragment,
            },

};

namespace RavEngine {
	DummyTonemap::DummyTonemap() : TonemapPass("dummytonemap",
		{
            .bindings = basicBindings
		}
	)
	{

	}
    DummyTonemapInstance::DummyTonemapInstance(Ref<DummyTonemap> pass) : TonemapPassInstance(pass, {})
    {
    }

    ACESTonemap::ACESTonemap() : TonemapPass("tonemap_aces", {
            .bindings = basicBindings
        })
    {
    }
    ACESTonemapInstance::ACESTonemapInstance(Ref<ACESTonemap> pass) : TonemapPassInstance(pass, {})
    {
    }
    ReinhardJodieTonemap::ReinhardJodieTonemap() : TonemapPass("tonemap_reinhard_jodie", {
            .bindings = basicBindings
        })
    {
    }
    ReinhardJodieTonemapInstance::ReinhardJodieTonemapInstance(Ref<ReinhardJodieTonemap> pass) : TonemapPassInstance(pass, {})
    {
    }

    AGXTonemap::AGXTonemap() : TonemapPass("tonemap_agx", {
            .bindings = basicBindings
        })
    {
    }
    AGXTonemapInstance::AGXTonemapInstance(Ref<AGXTonemap> pass) : TonemapPassInstance(pass, {})
    {
    }
}