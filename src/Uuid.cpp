#include "Uuid.hpp"

#ifdef _WIN32
#include <rpc.h>
#include <rpcdce.h>
#elif __APPLE__ || __linux__ || __EMSCRIPTEN__
#include <uuid/uuid.h>
#else
#error UUID: Unsupported platform
#endif

namespace RavEngine {

	RavEngine::uuids::uuid uuids::uuid::create()
	{
		uuid id;
#if _WIN32
		static_assert(sizeof(UUID) == 16, "Windows UUID is not the correct size!");

		UUID win_id;
		[[maybe_unused]] auto retval =  UuidCreate(&win_id);
		std::memcpy(id.data.data(), &win_id, id.size());
#elif __APPLE__ || __linux__ || __EMSCRIPTEN__
        static_assert(sizeof(uuid_t) == 16, "POSIX UUID is not the correct size!");
        uuid_t posix_id;
        uuid_generate(posix_id);
        
        std::memcpy(id.data.data(), &posix_id, id.size());
        
#endif
		return id;
	}
	std::string RavEngine::uuids::uuid::to_string() const
	{
		char str[38]{0};

		auto uuid = data.data();

		snprintf(str, std::size(str),
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
			uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]
		);

		return std::string(str, std::size(str));
	}
}
