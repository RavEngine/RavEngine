#pragma once

#define PROF_STR(a) const char* const a

namespace RavEngine {
	namespace Profile {
		extern PROF_STR(RenderBuildCommandlist);
		extern PROF_STR(RenderEncodeDeferredPass);
		extern PROF_STR(RenderEncodeLightingPass);
		extern PROF_STR(RenderEncodeForwardPass);
		extern PROF_STR(RenderEncodeShadowmaps);
		extern PROF_STR(RenderEncodePointShadows);
		extern PROF_STR(RenderEncodeSpotShadows);
		extern PROF_STR(RenderEncodeAllViews);

		extern PROF_STR(RenderExecuteCommandlist);


		extern PROF_STR(TickWorld);

		void BeginFrame(const char* const name);
		void EndFrame(const char* const name);
	}
}