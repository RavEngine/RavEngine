#include "System.hpp"
#include "AudioSource.hpp"

namespace RavEngine{

class AudioSyncSystem : public System{
protected:
	static list_type queries;
public:
	const list_type& QueryTypes() const override{
		return queries;
	}
	ctti_t ID() const override{
		return CTTI<AudioSyncSystem>;
	}
	
	void Tick(float fpsScale, Ref<Entity> e) override;
};

}
