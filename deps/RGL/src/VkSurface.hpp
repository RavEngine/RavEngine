#pragma once
#include <RGL/Types.hpp>
#include <RGL/Surface.hpp>
#include <volk.h>

namespace RGL {

	struct SurfaceVk : public ISurface {
		VkSurfaceKHR surface;
		SurfaceVk(decltype(surface) surface) : surface(surface) {}
		virtual ~SurfaceVk();
	};

	RGLSurfacePtr CreateVKSurfaceFromPlatformData(const CreateSurfaceConfig& config);
}