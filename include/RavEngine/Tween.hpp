#pragma once
#include <tweeny.h>
#include "mathtypes.hpp"
#include <functional>
#include "App.hpp"

namespace RavEngine{

namespace TweenCurves{
	//template wrappers around Tweeny
	template<typename base>
	struct TweenCurve : public base{
		//enable this class to be called / converted to std::function
		template<typename T>
		inline T operator()(float pos, T a, T b) const{
			return base::template run<float>(pos, a,b);
		}
	};
	static constexpr struct DefaultCurve : public TweenCurve<tweeny::easing::defaultEasing>{} DefaultCurve;
	static constexpr struct SteppedCurve : public TweenCurve<tweeny::easing::steppedEasing>{} SteppedCurve;
	static constexpr struct LinearCurve : public TweenCurve<tweeny::easing::linearEasing>{} LinearCurve;
	static constexpr struct QuadraticInCurve : public TweenCurve<tweeny::easing::quadraticInEasing>{} QuadraticInCurve;
	static constexpr struct QuadraticOutCurve : public TweenCurve<tweeny::easing::quadraticOutEasing>{} QuadraticOutCurve;
	static constexpr struct QuadraticInOutCurve : public TweenCurve<tweeny::easing::quadraticInOutEasing>{} QuadraticInOutCurve;
	static constexpr struct CubicInCurve : public TweenCurve<tweeny::easing::cubicInEasing>{} CubicInCurve;
	static constexpr struct CubicOutCurve : public TweenCurve<tweeny::easing::cubicOutEasing>{} CubicOutCurve;
	static constexpr struct CubicInOutCurve : public TweenCurve<tweeny::easing::cubicInOutEasing>{} CubicInOutCurve;
	static constexpr struct QuarticInCurve : public TweenCurve<tweeny::easing::quarticInEasing>{} QuarticInEasing;
	static constexpr struct QuarticOutCurve : public TweenCurve<tweeny::easing::quarticOutEasing>{} QuarticOutCurve;
	static constexpr struct QuarticInOutCurve : public TweenCurve<tweeny::easing::quarticInOutEasing>{} QuarticInOutCurve;
	static constexpr struct QuinticInCurve : public TweenCurve<tweeny::easing::quinticInEasing>{} QuinticInCurve;
	static constexpr struct QuinticOutCurve : public TweenCurve<tweeny::easing::quinticOutEasing>{} QuinticOutCurve;
	static constexpr struct QuinticInOutCurve : public TweenCurve<tweeny::easing::quinticInOutEasing>{} QuinticInOutCurve;
	static constexpr struct SinusoidalInCurve : public TweenCurve<tweeny::easing::sinusoidalInEasing>{} SinusoidalInCurve;
	static constexpr struct SinusoidalOutCurve : public TweenCurve<tweeny::easing::sinusoidalOutEasing>{} SinusoidalOutCurve;
	static constexpr struct SinusoidalInOutCurve : public TweenCurve<tweeny::easing::sinusoidalInOutEasing>{} SinusoidalInOutCurve;
	static constexpr struct ExponentialInCurve : public TweenCurve<tweeny::easing::exponentialInEasing>{} ExponentialInCurve;
	static constexpr struct ExponentialOutCurve : public TweenCurve<tweeny::easing::exponentialOutEasing>{} ExponentialOutCurve;
	static constexpr struct ExponentialInOutCurve : public TweenCurve<tweeny::easing::exponentialInOutEasing>{} ExponentialInOutCurve;
	static constexpr struct CircularInCurve : public TweenCurve<tweeny::easing::circularInEasing>{} CircularInCurve;
	static constexpr struct CircularOutCurve : public TweenCurve<tweeny::easing::circularOutEasing>{} CircularOutCurve;
	static constexpr struct CircularInOutCurve : public TweenCurve<tweeny::easing::circularInOutEasing>{} CircularInOutCurve;
	static constexpr struct BounceInCurve : public TweenCurve<tweeny::easing::bounceInEasing>{} BounceInCurve;
	static constexpr struct BounceOutCurve : public TweenCurve<tweeny::easing::bounceOutEasing>{} BounceOutCurve;
	static constexpr struct BounceInOutCurve : public TweenCurve<tweeny::easing::bounceInOutEasing>{} BounceInOutCurve;
	static constexpr struct ElasticInCurve : public TweenCurve<tweeny::easing::elasticInEasing>{} ElasticInCurve;
	static constexpr struct ElasticOutCurve : public TweenCurve<tweeny::easing::elasticOutEasing>{} ElasticOutCurve;
	static constexpr struct ElasticInOutCurve : public TweenCurve<tweeny::easing::elasticOutEasing>{} ElasticInOutCurve;
	static constexpr struct BackInCurve : public TweenCurve<tweeny::easing::backInEasing>{} BackInCurve;
	static constexpr struct BackOutCurve : public TweenCurve<tweeny::easing::backOutEasing>{} BackOutCurve;
	static constexpr struct BackInOutCurve : public TweenCurve<tweeny::easing::backOutEasing>{} BackInOutCurve;
}



	template<typename ...Floats>
	class Tween {
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
		inline Tween& AddKeyframe(decimalType time, T interpolation, Floats ... values){
			anim.to(values...).during(time * App::evalNormal).via(interpolation);
			return *this;
		}
		
		/**
		 Perform one step of this animation. Note that adding more keys while an animation is playing will affect its playback
		 @param scale the timestep. Generally you want to simply pass the value from tick.
		 */
		inline void step(decimalType scale){
			anim.step((float)scale / anim.duration());
		}
		
		inline void seek(float pos){
			anim.seek(pos);
		}
		
		inline float progress() const{
			return anim.progress();
		}
	};
}
