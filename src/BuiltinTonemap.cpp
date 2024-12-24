#include "BuiltinTonemap.hpp"

namespace RavEngine {
	DummyTonemap::DummyTonemap() : TonemapPass("dummytonemap",
		{
            .bindings = {
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
           
        },
		}
		
	)
	{

	}
    DummyTonemapInstance::DummyTonemapInstance(Ref<DummyTonemap> pass) : TonemapPassInstance(pass, {})
    {
    }
}