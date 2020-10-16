#pragma once
#include <tweeny.h>
#include "mathtypes.hpp"
#include <functional>
#include "App.hpp"

namespace RavEngine{

	//template wrappers around Tweeny
	template<typename base>
	struct TweenCurve : public base{
		//enable this class to be called / converted to std::function
		template<typename T>
		T operator()(float pos, T a, T b) const{
			return base::template run<float>(pos, a,b);
		}
	};

	static constexpr struct DefaultCurveCL : public TweenCurve<tweeny::easing::defaultEasing>{} DefaultCurve;
	static constexpr struct SteppedCurveCL : public TweenCurve<tweeny::easing::steppedEasing>{} SteppedCurve;
	static constexpr struct LinearCurveCL : public TweenCurve<tweeny::easing::linearEasing>{} LinearCurve;
	static constexpr struct QuadraticInCurveCL : public TweenCurve<tweeny::easing::quadraticInEasing>{} QuadraticInCurve;
	static constexpr struct QuadraticOutCurveCL : public TweenCurve<tweeny::easing::quadraticOutEasing>{} QuadraticOutCurve;
	static constexpr struct QuadraticInOutCurveCL : public TweenCurve<tweeny::easing::quadraticInOutEasing>{} QuadraticInOutCurve;
	static constexpr struct CubicInCurveCL : public TweenCurve<tweeny::easing::cubicInEasing>{} CubicInCurve;
	static constexpr struct CubicOutCurveCL : public TweenCurve<tweeny::easing::cubicOutEasing>{} CubicOutCurve;
	static constexpr struct CubicInOutCurveCL : public TweenCurve<tweeny::easing::cubicInOutEasing>{} CubicInOutCurve;
	static constexpr struct QuarticInCurveCL : public TweenCurve<tweeny::easing::quarticInEasing>{} QuarticInEasing;
	static constexpr struct QuarticOutCurveCL : public TweenCurve<tweeny::easing::quarticOutEasing>{} QuarticOutCurve;
	static constexpr struct QuarticInOutCurveCL : public TweenCurve<tweeny::easing::quarticInOutEasing>{} QuarticInOutCurve;
	static constexpr struct QuinticInCurveCL : public TweenCurve<tweeny::easing::quinticInEasing>{} QuinticInCurve;
	static constexpr struct QuinticOutCurveCL : public TweenCurve<tweeny::easing::quinticOutEasing>{} QuinticOutCurve;
	static constexpr struct QuinticInOutCurveCL : public TweenCurve<tweeny::easing::quinticInOutEasing>{} QuinticInOutCurve;
	static constexpr struct SinusoidalInCurveCL : public TweenCurve<tweeny::easing::sinusoidalInEasing>{} SinusoidalInCurve;
	static constexpr struct SinusoidalOutCurveCL : public TweenCurve<tweeny::easing::sinusoidalOutEasing>{} SinusoidalOutCurve;
	static constexpr struct SinusoidalInOutCurveCL : public TweenCurve<tweeny::easing::sinusoidalInOutEasing>{} SinusoidalInOutCurve;


	template<typename ...Floats>
	class Tween : public SharedObject{
	protected:
		tweeny::tween<Floats...> anim;
		typedef std::function<void(Floats...)> stepfunc;
		
	public:
		Tween(){};
		
		/**
		 Construct a Tween object
		 @param step the function to invoke on each step.
		 @param intialValue the starting value, set to 0 if not passed.
		 */
		Tween(const stepfunc& step, Floats ... initialValue){
			anim = tweeny::from(initialValue...).onStep([=](Floats... values) -> bool{
				step(values...);
				return false;
			});
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
		Tween& AddKeyframe(decimalType time, T interpolation, Floats ... values){
			anim.to(values...).during(time * App::evalNormal).via(interpolation);
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
