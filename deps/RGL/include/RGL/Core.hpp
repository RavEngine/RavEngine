#pragma once
#include <memory>
#include <functional>
#include <string>

namespace RGL {

	enum class API : uint8_t {
		Uninitialized,			// You didn't call RGL::Init yet
		PlatformDefault,		// RGL will pick an API
		Noop,					// this API does nothing
		Metal,					// Apple Metal
		Direct3D12,				// Microsoft DirectX 12
		Vulkan,					// Vulkan
		WebGPU					// WebGPU (NOT WebGL!)
	};

	constexpr static API APIsAvailable[] = {
#if RGL_MTL_AVAILABLE
		API::Metal,
#endif
#if RGL_DX12_AVAILABLE
		API::Direct3D12,
#endif
#if RGL_VK_AVAILABLE
		API::Vulkan,
#endif
#if RGL_WEBGPU_AVAILABLE
		API::WebGPU,
#endif
		API::Noop,
	};

	bool CanInitAPI(API api);
	const char* APIToString(API);

	enum class MessageSeverity : uint8_t {
		Info = 0,
		Warning,
		Error,
		Fatal
	};

	using callback_t = std::function<void(MessageSeverity, const std::string&)>;

	struct InitOptions {
		API api = API::PlatformDefault;	// what graphics API to use
		callback_t callback;			// what function to invoke with debug messages

		std::string appName;			// Name of your app. Used only on Vulkan.
		std::string engineName;			// Name of your game engine. Used only on Vulkan.
		struct Version {
			uint8_t variant = 0;
			uint8_t major = 0;
			uint8_t minor = 0;
			uint8_t patch = 0;
		} appVersion,				// The version number of your app. Used only on Vulkan.
			engineVersion;			// The version number of your engine. Used only on Vulkan.
	};

	API CurrentAPI();

	void Init(const InitOptions&);
	void Shutdown();

}