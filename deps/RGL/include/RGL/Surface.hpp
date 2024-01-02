#pragma once
#include <RGL/Types.hpp>
#include <memory>
#include <cstdint>

namespace RGL {

	struct ISurface {

	};

	struct CreateSurfaceConfig {
		const void* pointer = nullptr;
		uintptr_t pointer2 = 0;
#if __linux__
		bool isWayland = false;
#endif
	};

	/**
	Create a surface from platform native data
	@param pointer A CAMetalLayer* (Apple), HWND* (Win32),
	@param createSurfaceObject If set to true, RGL will create the OS-specific context object, so pointer should be a NSWindow* or UIWindow*
	*/
	RGLSurfacePtr CreateSurfaceFromPlatformHandle(const CreateSurfaceConfig& pointer, bool createSurfaceObject = false);

}
