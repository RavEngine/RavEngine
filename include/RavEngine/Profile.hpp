#pragma once

#define PROF_STR(a) const char* const a

#if (__has_include(<tracy/Tracy.hpp>))
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
#define RVE_PROFILE 1
#endif

namespace RavEngine {
	namespace Profile {

		void BeginFrame(const char* const name);
		void EndFrame(const char* const name);
		void EndTick();

#if RVE_PROFILE
#define RVE_PROFILE_FN ZoneScoped
#define RVE_PROFILE_FN_NC(name, color) ZoneScopedNC(name,color)
#define RVE_PROFILE_FN_N(name) ZoneScopedN(name)
#define RVE_PROFILE_SECTION(varName,zoneName) TracyCZoneN(RVE_PRF_ ## varName,zoneName,true);
#define RVE_PROFILE_SECTION_END(varName) TracyCZoneEnd(RVE_PRF_ ## varName) ;
#else
#define RVE_PROFILE_FN
#define RVE_PROFILE_FN_NC(n,c)
#define RVE_PROFILE_FN_N(n)
#define RVE_PROFILE_SECTION(varName,zoneName)
#define RVE_PROFILE_SECTION_END(varName)
#endif
	}
}