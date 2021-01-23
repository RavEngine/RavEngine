#include "System.hpp"
#include "AudioSource.hpp"

namespace RavEngine{

class AudioEngine;

class AudioSyncSystem : public System{
protected:
	static list_type queries;
	AudioEngine& engptr;
public:
	/**
	 Create an AudioSyncSystem
	 @param p the reference to the AudioEngine
	 */
	AudioSyncSystem(AudioEngine& p) : engptr(p){}
	
	const list_type& QueryTypes() const override{
		return queries;
	}
	ctti_t ID() const override{
		return CTTI<AudioSyncSystem>;
	}
	
	void Tick(float fpsScale, Ref<Entity> e) override;
};

}
