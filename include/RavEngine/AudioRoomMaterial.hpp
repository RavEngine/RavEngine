#pragma once
#include <common/room_properties.h>
#include "DataStructures.hpp"

namespace RavEngine{

/**
 Reverbation data for a room. Subclass to create new material presets
 */
class RoomMaterial : public vraudio::ReverbProperties{
public:
	/**
	 RT60 values
	 @param rt60 the RT60 values representing this material
	 @param gain optional gain multiplier
	 */
	RoomMaterial(const Array<std::remove_reference<decltype( *rt60_values )>::type,9>& rt60, float gain = 0){
		std::memcpy(rt60_values, rt60.data(), sizeof(rt60_values));
		this->gain = gain;
	}
	
	/**
	 Change the gain
	 */
	constexpr inline void SetGain(float g){ gain = g; }
};

// put presets here

}

