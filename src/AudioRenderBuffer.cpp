#include "AudioRenderBuffer.hpp"
#include "Debug.hpp"

namespace RavEngine {
	void SingleAudioRenderBuffer_t_readwritetrack::AcquireRead() const
	{
#if RVE_DEBUG_AUDIO_ACCESS
		std::lock_guard lock(mtx);
		// reads can be taken as long as a write is not active
		if (writersActive) {
			Debug::Fatal("Audio reader taken when writers are active!");
		}
		readerCount++;
#endif

	}
	void SingleAudioRenderBuffer_t_readwritetrack::ReleaseRead() const
	{
#if RVE_DEBUG_AUDIO_ACCESS
		std::lock_guard lock(mtx);
		readerCount--;
#endif
	}
	void SingleAudioRenderBuffer_t_readwritetrack::AcquireWrite() const
	{
#if RVE_DEBUG_AUDIO_ACCESS
		std::lock_guard lock(mtx);
		// writes can be taken if:
			// 1. readers are not active
			// 2. another write is not active
		if (readerCount > 0 || writersActive) {
			Debug::Fatal("Audio writer taken when writers are active, or readers are active!");
		}
		writersActive = true;
#endif
	}
	void SingleAudioRenderBuffer_t_readwritetrack::ReleaseWrite() const
	{
#if RVE_DEBUG_AUDIO_ACCESS
		std::lock_guard lock(mtx);
		writersActive = false;
#endif
	}
}