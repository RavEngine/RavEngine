#pragma once
#include <tweeny.h>
#include "mathtypes.hpp"
#include <vector>
#include <functional>
#include "RavEngine_App.hpp"

namespace RavEngine{
	class Tween : public SharedObject{
	protected:
		tweeny::tween<decimalType> anim;
		typedef std::function<bool(decimalType)> stepfunc;
		
	public:
		Tween(){};
		
		/**
		 Construct a Tween object
		 @param step the function to invoke on each step.
		 @param intialValue the starting value, set to 0 if not passed.
		 */
		Tween(const stepfunc& step, decimalType initialValue = 0){
			anim = tweeny::from(initialValue).onStep(step);
		}
		
		Tween(const Tween& other){
			anim = other.anim;
		}
		
		//copy assignment
		Tween& operator=(const Tween& other) {
			if (&other == this) {
				return *this;
			}
			anim = other.anim;
			return *this;
		}
		
		/**
		 Add a keyframe to the animation
		 @param value the value for the keyframe
		 @param time the time (in seconds) for this keyframe.
		 @param interpolation the interpolation function to use
		 */
		template<typename T>
		Tween& AddKeyframe(decimalType value, decimalType time, T interpolation){
			anim.to(value).during(time * App::evalNormal).via(interpolation);
			return *this;
		}
		
		/**
		 Perform one step of this animation. Note that adding more keys while an animation is playing will affect its playback
		 @param scale the timestep. Generally you want to simply pass the value from tick.
		 */
		void step(decimalType scale){
			anim.step((float)scale / anim.duration());
		}
	};
}
