#include "Profile.hpp"
#include <tracy/Tracy.hpp>

namespace RavEngine {
	namespace Profile {
		PROF_STR(RenderBuildCommandlist) = "Render Build Commandlist";
		PROF_STR(RenderExecuteCommandlist) = "Render Execute Commandlist";
		PROF_STR(RenderEncodeDeferredPass) = "Render Encode Deferred Pass";
		PROF_STR(RenderEncodeLightingPass) = "Render Encode Lighting Pass";
		PROF_STR(RenderEncodeForwardPass) = "Render Encode Forward Pass";
		PROF_STR(RenderEncodeShadowmaps) = "Render Encode Shadowmaps";
		PROF_STR(RenderEncodePointShadows) = "Render Encode Point Lights";
		PROF_STR(RenderEncodeSpotShadows) = "Render Encode Spot Lights";
		PROF_STR(RenderEncodeAllViews) = "Render Encode All Views";
		PROF_STR(TickWorld) = "Tick World";

		void BeginFrame(const char* const name)
		{
			FrameMarkStart(name);
		}
		void EndFrame(const char* const name)
		{
			FrameMarkEnd(name);
		}
	}
}