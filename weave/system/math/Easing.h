/*
Title: "Easing library"
File: Easing.h
Author(s): Eduardo Martínez Vidal

Abstract:
	Definition of EasingCurve function type and multiple easing functions with varying properties.
	All implementations are inlined.

Update Log:
	

*/

#pragma once

#include "BasicMath.h"
#include <functional>
#include <string>


namespace weave {

namespace easing {

	//Definition for the function body type required. Receives a normalized time and returns single float representing the curved time value.
	typedef std::function<float(float)> EasingCurve;

	//Returns an easing function by name
	EasingCurve GetEasingFunction(std::string const &name);

	//Follows a standard implementation of common easing curves.
	//A visual representation of curvers can be found at: http://www.timotheegroleau.com/Flash/experiments/easing_function_generator.htm

	//Returns a flat 1.0
	inline float flat(float) { return 1.0f; }
	
	//Returns the input t
	inline float linear(float t) { return t; }

	//Step function which outputs round(t) (0 or 1)
	inline float step(float t) { return std::roundf(t); }

	//Sine function where t 0..1 is mapped to 0..2PI and returns a squashed value between 0 .. 1 instead of -1 .. 1
	inline float sin(float t) { 
		return (std::sin(t * algebra::F_2PI) + 1.0f) / 2.0f; 
	}
	//Cosine function where t 0..1 is mapped to 0..2PI
	inline float cos(float t) { 
		return (std::cos(t * algebra::F_2PI) + 1.0f) / 2.0f; 
	}
	
	//Quadratic
	inline float quadraticIn(float t) { return t*t; }
	inline float quadraticOut(float t) { return -t*(t-2.0f); }
	inline float quadraticInOut(float t) { 
		t *= 2.0f;
		if(t < 1.0f) 
			return 0.5f*t*t;
		--t;
		return -0.5f*(t*(t-2) - 1);
	}

	//Cubic
	inline float cubicIn(float t) { return t*t*t; }
	inline float cubicOut(float t) { 
		--t;
		return t*t*t + 1; 
	}
	inline float cubicInOut(float t) { 
		t *= 2.0f;
		if(t < 1.0f)
			return 0.5f*t*t*t;
		t -= 2;
		return 0.5f*(t*t*t + 2); 
	}
	inline float cubicElasticIn(float t) {
		return (4*t*t*t - 3*t*t);
	}
	inline float cubicElasticOut(float t) {
		return (4*t*t*t - 9*t*t + 6*t);
	}


	//Quartic
	inline float quarticIn(float t) { return t*t*t*t; }
	inline float quarticOut(float t) { 
		--t;
		return -(t*t*t*t - 1); 
	}
	inline float quarticInOut(float t) { 
		t *= 2.0f;
		if(t < 1.0f)
			return 0.5f*t*t*t*t;
		t -= 2;
		return -0.5f*(t*t*t*t - 2); 
	}

	inline float quarticElasticIn(float t) {
		return (2*t*t*t*t + 2*t*t*t - 3*t*t);
	}
	inline float quarticElasticOut(float t) {
		return (-2*t*t*t*t + 10*t*t*t - 15*t*t + 8*t);
	}

	//Quintic
	inline float quinticIn(float t) { return t*t*t*t*t; }
	inline float quinticOut(float t) { 
		--t;
		return t*t*t*t*t + 1; 
	}
	inline float quinticInOut(float t) { 
		t *= 2.0f;
		if(t < 1.0f)
			return 0.5f*t*t*t*t*t;
		t -= 2;
		return 0.5f*(t*t*t*t*t + 2); 
	}
	
	//Sinusoidal
	inline float sinusIn(float t) { return 1.0f - cosf(t * weave::algebra::F_PIH); }
	inline float sinusOut(float t) { return sinf(t * weave::algebra::F_PIH); }
	inline float sinusInOut(float t) { return 0.5f - 0.5f * cosf(t*weave::algebra::F_PI); }

	//Exponential
	inline float expIn(float t) { return powf(2.0f, 10.0f*(t-1)); }
	inline float expOut(float t) { return 1.0f - powf(2.0f, -10.0f*t); }
	inline float expInOut(float t) { 
		t *= 2.0f;
		if(t < 1.0f)
			return 0.5f*powf(2.0f, 10.0f*(t-1));
		--t;
		return 0.5f * (2.0f - powf(2.0f, -10.0f*t));
	}

	//Circular
	inline float circIn(float t) { return 1 - sqrtf(1 - t*t); }
	inline float circOut(float t) { 
		--t;
		return sqrtf(1 - t*t); 
	}
	inline float circInOut(float t) { 
		t *= 2.0f;
		if(t < 1.0f)
			return 0.5f*(1 - sqrtf(1 - t*t));
		t -= 2.0f;
		return 0.5f * (1 + sqrtf(1 - t*t));
	}

	//Elastic
	inline float elasticSoftOut(float t) {
		float ts = t*t;
		float tc = t*ts;
		return (33*tc*ts - 106*ts*ts + 126*tc - 67*ts + 15*t);
	}

	inline float elasticSoftIn(float t) {
		float ts = t*t;
		float tc = t*ts;
		return (33*tc*ts - 59*ts*ts + 32*tc - 5*ts);
	}

	inline float elasticHardOut(float t) {
		float ts = t*t;
		float tc = t*ts;
		return (56*tc*ts - 175*ts*ts + 200*tc - 100*ts + 20*t);
	}

	inline float elasticHardIn(float t) {
		float ts = t*t;
		float tc = t*ts;
		return (56*tc*ts - 105*ts*ts + 60*tc - 10*ts);;
	}

	//Bounce
	//Adapted from https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.5/d3.js
	inline float bounceOut(float t) {
		if(t < 1.0f / 2.75f) {
			return 7.5625f * t * t;
		}
		else if(t < 2.0f / 2.75f){
			t -= (1.5f / 2.75f);
			return 7.5625f * t * t + .75f;
		}
		else if(t < 2.5f / 2.75f) {
			t -= (2.25f / 2.75f);
			return 7.5625f * t * t + .9375f;
		}
		else {
			t -= (2.625f / 2.75f);
			return 7.5625f * t * t + .984375f;
		}
		//This version includes an unsequenced modification.
		//return t < 1.0f / 2.75f ? 7.5625f * t * t : t < 2.0f / 2.75f ? 7.5625f * (t -= (1.5f / 2.75f)) * t + .75f : t < 2.5f / 2.75f ? 7.5625f * (t -= (2.25f / 2.75f)) * t + .9375f : 7.5625f * (t -= (2.625f / 2.75f)) * t + .984375f;
	}

	inline float bounceIn(float t) {
		return 1.0f - bounceOut(1.0f - t);
	}

	inline float bounceInOut(float t) {
		return .5f * (t < .5f ? bounceIn(2.0f * t) : 2.0f - bounceIn(2.0f - 2.0f * t));
	}


	//Inverted versions of the standard easing functions
	inline float ilinear(float t) { return 1.0f - t; }

	//Step function which outputs round(t) (0 or 1)
	inline float istep(float t) { return std::roundf(1.0f - t); }

	//Sine function where t 0..1 is mapped to 0..2PI
	inline float isin(float t) { return easing::sin(1.0f - t); }
	//Cosine function where t 0..1 is mapped to 0..2PI
	inline float icos(float t) { return easing::cos(1.0f - t); }

	//Quadratic
	inline float iquadraticIn(float t) { return 1.0f - t*t; }
	inline float iquadraticOut(float t) { return 1.0f + t*(t - 2.0f); }
	inline float iquadraticInOut(float t) {
		t *= 2.0f;
		if(t < 1.0f)
			return 1.0f - 0.5f*t*t;
		--t;
		return 1.0f + 0.5f*(t*(t - 2) - 1);
	}

	//Cubic
	inline float icubicIn(float t) { return 1.0f - t*t*t; }
	inline float icubicOut(float t) {
		--t;
		return 1.0f - (t*t*t + 1);
	}
	inline float icubicInOut(float t) {
		t *= 2.0f;
		if(t < 1.0f)
			return 1.0f - 0.5f*t*t*t;
		t -= 2;
		return 1.0f - 0.5f*(t*t*t + 2);
	}
	inline float icubicElasticIn(float t) {
		return 1.0f - (4 * t*t*t - 3 * t*t);
	}
	inline float icubicElasticOut(float t) {
		return 1.0f - (4 * t*t*t - 9 * t*t + 6 * t);
	}


	//Quartic
	inline float iquarticIn(float t) { return 1.0f - t*t*t*t; }
	inline float iquarticOut(float t) {
		--t;
		return 1.0f + (t*t*t*t - 1);
	}
	inline float iquarticInOut(float t) {
		t *= 2.0f;
		if(t < 1.0f)
			return 1.0f - 0.5f*t*t*t*t;
		t -= 2;
		return 1.0f + 0.5f*(t*t*t*t - 2);
	}

	inline float iquarticElasticIn(float t) {
		return 1.0f - (2 * t*t*t*t + 2 * t*t*t - 3 * t*t);
	}
	inline float iquarticElasticOut(float t) {
		return 1.0f - (-2 * t*t*t*t + 10 * t*t*t - 15 * t*t + 8 * t);
	}

	//Quintic
	inline float iquinticIn(float t) { return 1.0f - t*t*t*t*t; }
	inline float iquinticOut(float t) {
		--t;
		return 1.0f - (t*t*t*t*t + 1);
	}
	inline float iquinticInOut(float t) {
		t *= 2.0f;
		if(t < 1.0f)
			return 1.0f - 0.5f*t*t*t*t*t;
		t -= 2;
		return 1.0f - 0.5f*(t*t*t*t*t + 2);
	}

	//Sinusoidal
	inline float isinusIn(float t) { return cosf(t * weave::algebra::F_PIH); }
	inline float isinusOut(float t) { return 1.0f - sinf(t * weave::algebra::F_PIH); }
	inline float isinusInOut(float t) { return 0.5f + 0.5f * cosf(t*weave::algebra::F_PI); }

	//Exponential
	inline float iexpIn(float t) { return 1.0f - powf(2.0f, 10.0f*(t - 1)); }
	inline float iexpOut(float t) { return powf(2.0f, -10.0f*t); }
	inline float iexpInOut(float t) {
		t *= 2.0f;
		if(t < 1.0f)
			return 1.0f - 0.5f*powf(2.0f, 10.0f*(t - 1));
		--t;
		return 1.0f - 0.5f * (2.0f - powf(2.0f, -10.0f*t));
	}

	//Circular
	inline float icircIn(float t) { return sqrtf(1 - t*t); }
	inline float icircOut(float t) {
		--t;
		return 1.0f - sqrtf(1 - t*t);
	}
	inline float icircInOut(float t) {
		t *= 2.0f;
		if(t < 1.0f)
			return 1.0f - 0.5f*(1 - sqrtf(1 - t*t));
		t -= 2.0f;
		return 1.0f - 0.5f * (1 + sqrtf(1 - t*t));
	}

	//Elastic
	inline float ielasticSoftOut(float t) {
		float ts = t*t;
		float tc = t*ts;
		return 1.0f - (33 * tc*ts - 106 * ts*ts + 126 * tc - 67 * ts + 15 * t);
	}

	inline float ielasticSoftIn(float t) {
		float ts = t*t;
		float tc = t*ts;
		return 1.0f - (33 * tc*ts - 59 * ts*ts + 32 * tc - 5 * ts);
	}

	inline float ielasticHardOut(float t) {
		float ts = t*t;
		float tc = t*ts;
		return 1.0f - (56 * tc*ts - 175 * ts*ts + 200 * tc - 100 * ts + 20 * t);
	}

	inline float ielasticHardIn(float t) {
		float ts = t*t;
		float tc = t*ts;
		return 1.0f - (56 * tc*ts - 105 * ts*ts + 60 * tc - 10 * ts);;
	}

	//Bounce
	//Adapted from https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.5/d3.js
	inline float ibounceOut(float t) {
		return 1.0f - bounceOut(t);
	}

	inline float ibounceIn(float t) {
		return bounceOut(1.0f - t);
	}

	inline float ibounceInOut(float t) {
		return 1.0f - (.5f * (t < .5f ? bounceIn(2.0f * t) : 2.0f - bounceIn(2.0f - 2.0f * t)));
	}


	//Reflected versions of the easing functions where they start at 0 and end at 0, peaking at 0.5
	inline float rlinear(float t) { 
		if(t < 0.5f)
			return t * 2.0f;
		else
			return 1.0f - (t - 0.5f) * 2.0f; 
	}

	//Step function which outputs round(t) (0 or 1)
	inline float rstep(float t) { 
		if(t < 0.5f)
			return std::roundf(t * 2.0f);
		else
			return std::roundf(1.0f - (t - 0.5f) * 2.0f);
	}

	//Sine function where t 0..1 is mapped to 0..2PI
	inline float rsin(float t) { 
		if(t < 0.5f)
			return easing::sin(t * 2.0f);
		else
			return easing::sin(1.0f - (t - 0.5f) * 2.0f);
	}
	//Cosine function where t 0..1 is mapped to 0..2PI
	inline float rcos(float t) {
		if(t < 0.5f)
			return easing::cos(t * 2.0f);
		else
			return easing::cos(1.0f - (t - 0.5f) * 2.0f);
	}

	//Quadratic
	inline float rquadraticIn(float t) {
		if(t < 0.5f)
			return quadraticIn(t * 2.0f);
		else
			return quadraticIn(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rquadraticOut(float t) {
		if(t < 0.5f)
			return quadraticOut(t * 2.0f);
		else
			return quadraticOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rquadraticInOut(float t) {
		if(t < 0.5f)
			return quadraticInOut(t * 2.0f);
		else
			return quadraticInOut(1.0f - (t - 0.5f) * 2.0f);
	}

	//Cubic
	inline float rcubicIn(float t) {
		if(t < 0.5f)
			return cubicIn(t * 2.0f);
		else
			return cubicIn(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rcubicOut(float t) {
		if(t < 0.5f)
			return cubicOut(t * 2.0f);
		else
			return cubicOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rcubicInOut(float t) {
		if(t < 0.5f)
			return cubicInOut(t * 2.0f);
		else
			return cubicInOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rcubicElasticIn(float t) {
		if(t < 0.5f)
			return cubicElasticIn(t * 2.0f);
		else
			return cubicElasticIn(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rcubicElasticOut(float t) {
		if(t < 0.5f)
			return cubicElasticOut(t * 2.0f);
		else
			return cubicElasticOut(1.0f - (t - 0.5f) * 2.0f);
	}


	//Quartic
	inline float rquarticIn(float t) {
		if(t < 0.5f)
			return quarticIn(t * 2.0f);
		else
			return quarticIn(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rquarticOut(float t) {
		if(t < 0.5f)
			return quarticOut(t * 2.0f);
		else
			return quarticOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rquarticInOut(float t) {
		if(t < 0.5f)
			return quarticInOut(t * 2.0f);
		else
			return quarticInOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rquarticElasticIn(float t) {
		if(t < 0.5f)
			return quarticElasticIn(t * 2.0f);
		else
			return quarticElasticIn(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rquarticElasticOut(float t) {
		if(t < 0.5f)
			return quarticElasticOut(t * 2.0f);
		else
			return quarticElasticOut(1.0f - (t - 0.5f) * 2.0f);
	}

	//Quintic
	inline float rquinticIn(float t) {
		if(t < 0.5f)
			return quinticIn(t * 2.0f);
		else
			return quinticIn(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rquinticOut(float t) {
		if(t < 0.5f)
			return quinticOut(t * 2.0f);
		else
			return quinticOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rquinticInOut(float t) {
		if(t < 0.5f)
			return quinticInOut(t * 2.0f);
		else
			return quinticInOut(1.0f - (t - 0.5f) * 2.0f);
	}

	//Sinusoidal
	inline float rsinusIn(float t) {
		if(t < 0.5f)
			return sinusIn(t * 2.0f);
		else
			return sinusIn(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rsinusOut(float t) {
		if(t < 0.5f)
			return sinusOut(t * 2.0f);
		else
			return sinusOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rsinusInOut(float t) {
		if(t < 0.5f)
			return sinusInOut(t * 2.0f);
		else
			return sinusInOut(1.0f - (t - 0.5f) * 2.0f);
	}

	//Exponential
	inline float rexpIn(float t) {
		if(t < 0.5f)
			return expIn(t * 2.0f);
		else
			return expIn(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rexpOut(float t) {
		if(t < 0.5f)
			return expOut(t * 2.0f);
		else
			return expOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rexpInOut(float t) {
		if(t < 0.5f)
			return expInOut(t * 2.0f);
		else
			return expInOut(1.0f - (t - 0.5f) * 2.0f);
	}

	//Circular
	inline float rcircIn(float t) {
		if(t < 0.5f)
			return circIn(t * 2.0f);
		else
			return circIn(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rcircOut(float t) {
		if(t < 0.5f)
			return circOut(t * 2.0f);
		else
			return circOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rcircInOut(float t) {
		if(t < 0.5f)
			return circInOut(t * 2.0f);
		else
			return circInOut(1.0f - (t - 0.5f) * 2.0f);
	}

	//Elastic
	inline float relasticSoftOut(float t) {
		if(t < 0.5f)
			return elasticSoftOut(t * 2.0f);
		else
			return elasticSoftOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float relasticSoftIn(float t) {
		if(t < 0.5f)
			return elasticSoftIn(t * 2.0f);
		else
			return elasticSoftIn(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float relasticHardOut(float t) {
		if(t < 0.5f)
			return elasticHardOut(t * 2.0f);
		else
			return elasticHardOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float relasticHardIn(float t) {
		if(t < 0.5f)
			return elasticHardIn(t * 2.0f);
		else
			return elasticHardIn(1.0f - (t - 0.5f) * 2.0f);
	}

	//Bounce
	//Adapted from https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.5/d3.js
	inline float rbounceOut(float t) {
		if(t < 0.5f)
			return bounceOut(t * 2.0f);
		else
			return bounceOut(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rbounceIn(float t) {
		if(t < 0.5f)
			return bounceIn(t * 2.0f);
		else
			return bounceIn(1.0f - (t - 0.5f) * 2.0f);
	}

	inline float rbounceInOut(float t) {
		if(t < 0.5f)
			return bounceInOut(t * 2.0f);
		else
			return bounceInOut(1.0f - (t - 0.5f) * 2.0f);
	}

	//Reflected versions of the easing functions where they start at 0 and end at 0, peaking at 0.5 and the out slope is inverted
	inline float rilinear(float t) {
		return rlinear(t);
	}

	//Step function which outputs round(t) (0 or 1)
	inline float ristep(float t) {
		if(t < 0.5f)
			return step(t * 2.0f);
		else
			return istep((t - 0.5f) * 2.0f);
	}

	//Sine function where t 0..1 is mapped to 0..2PI
	inline float risin(float t) {
		if(t < 0.5f)
			return easing::sin(t * 2.0f);
		else
			return easing::isin((t - 0.5f) * 2.0f);
	}
	//Cosine function where t 0..1 is mapped to 0..2PI
	inline float ricos(float t) {
		if(t < 0.5f)
			return easing::cos(t * 2.0f);
		else
			return easing::icos((t - 0.5f) * 2.0f);
	}

	//Quadratic
	inline float riquadraticIn(float t) {
		if(t < 0.5f)
			return quadraticIn(t * 2.0f);
		else
			return iquadraticIn((t - 0.5f) * 2.0f);
	}

	inline float riquadraticOut(float t) {
		if(t < 0.5f)
			return quadraticOut(t * 2.0f);
		else
			return iquadraticOut((t - 0.5f) * 2.0f);
	}

	inline float riquadraticInOut(float t) {
		if(t < 0.5f)
			return quadraticInOut(t * 2.0f);
		else
			return iquadraticInOut((t - 0.5f) * 2.0f);
	}

	//Cubic
	inline float ricubicIn(float t) {
		if(t < 0.5f)
			return cubicIn(t * 2.0f);
		else
			return icubicIn((t - 0.5f) * 2.0f);
	}

	inline float ricubicOut(float t) {
		if(t < 0.5f)
			return cubicOut(t * 2.0f);
		else
			return icubicOut((t - 0.5f) * 2.0f);
	}

	inline float ricubicInOut(float t) {
		if(t < 0.5f)
			return cubicInOut(t * 2.0f);
		else
			return icubicInOut((t - 0.5f) * 2.0f);
	}

	inline float ricubicElasticIn(float t) {
		if(t < 0.5f)
			return cubicElasticIn(t * 2.0f);
		else
			return icubicElasticIn((t - 0.5f) * 2.0f);
	}

	inline float ricubicElasticOut(float t) {
		if(t < 0.5f)
			return cubicElasticOut(t * 2.0f);
		else
			return icubicElasticOut((t - 0.5f) * 2.0f);
	}


	//Quartic
	inline float riquarticIn(float t) {
		if(t < 0.5f)
			return quarticIn(t * 2.0f);
		else
			return iquarticIn((t - 0.5f) * 2.0f);
	}

	inline float riquarticOut(float t) {
		if(t < 0.5f)
			return quarticOut(t * 2.0f);
		else
			return iquarticOut((t - 0.5f) * 2.0f);
	}

	inline float riquarticInOut(float t) {
		if(t < 0.5f)
			return quarticInOut(t * 2.0f);
		else
			return iquarticInOut((t - 0.5f) * 2.0f);
	}

	inline float riquarticElasticIn(float t) {
		if(t < 0.5f)
			return quarticElasticIn(t * 2.0f);
		else
			return iquarticElasticIn((t - 0.5f) * 2.0f);
	}

	inline float riquarticElasticOut(float t) {
		if(t < 0.5f)
			return quarticElasticOut(t * 2.0f);
		else
			return iquarticElasticOut((t - 0.5f) * 2.0f);
	}

	//Quintic
	inline float riquinticIn(float t) {
		if(t < 0.5f)
			return quinticIn(t * 2.0f);
		else
			return iquinticIn((t - 0.5f) * 2.0f);
	}

	inline float riquinticOut(float t) {
		if(t < 0.5f)
			return quinticOut(t * 2.0f);
		else
			return iquinticOut((t - 0.5f) * 2.0f);
	}

	inline float riquinticInOut(float t) {
		if(t < 0.5f)
			return quinticInOut(t * 2.0f);
		else
			return iquinticInOut((t - 0.5f) * 2.0f);
	}

	//Sinusoidal
	inline float risinusIn(float t) {
		if(t < 0.5f)
			return sinusIn(t * 2.0f);
		else
			return isinusIn((t - 0.5f) * 2.0f);
	}

	inline float risinusOut(float t) {
		if(t < 0.5f)
			return sinusOut(t * 2.0f);
		else
			return isinusOut((t - 0.5f) * 2.0f);
	}

	inline float risinusInOut(float t) {
		if(t < 0.5f)
			return sinusInOut(t * 2.0f);
		else
			return isinusInOut((t - 0.5f) * 2.0f);
	}

	//Exponential
	inline float riexpIn(float t) {
		if(t < 0.5f)
			return expIn(t * 2.0f);
		else
			return iexpIn((t - 0.5f) * 2.0f);
	}

	inline float riexpOut(float t) {
		if(t < 0.5f)
			return expOut(t * 2.0f);
		else
			return iexpOut((t - 0.5f) * 2.0f);
	}

	inline float riexpInOut(float t) {
		if(t < 0.5f)
			return expInOut(t * 2.0f);
		else
			return iexpInOut((t - 0.5f) * 2.0f);
	}

	//Circular
	inline float ricircIn(float t) {
		if(t < 0.5f)
			return circIn(t * 2.0f);
		else
			return icircIn((t - 0.5f) * 2.0f);
	}

	inline float ricircOut(float t) {
		if(t < 0.5f)
			return circOut(t * 2.0f);
		else
			return icircOut((t - 0.5f) * 2.0f);
	}

	inline float ricircInOut(float t) {
		if(t < 0.5f)
			return circInOut(t * 2.0f);
		else
			return icircInOut((t - 0.5f) * 2.0f);
	}

	//Elastic
	inline float rielasticSoftOut(float t) {
		if(t < 0.5f)
			return elasticSoftOut(t * 2.0f);
		else
			return ielasticSoftOut((t - 0.5f) * 2.0f);
	}

	inline float rielasticSoftIn(float t) {
		if(t < 0.5f)
			return elasticSoftIn(t * 2.0f);
		else
			return ielasticSoftIn((t - 0.5f) * 2.0f);
	}

	inline float rielasticHardOut(float t) {
		if(t < 0.5f)
			return elasticHardOut(t * 2.0f);
		else
			return ielasticHardOut((t - 0.5f) * 2.0f);
	}

	inline float rielasticHardIn(float t) {
		if(t < 0.5f)
			return elasticHardIn(t * 2.0f);
		else
			return ielasticHardIn((t - 0.5f) * 2.0f);
	}

	//Bounce
	//Adapted from https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.5/d3.js
	inline float ribounceOut(float t) {
		if(t < 0.5f)
			return bounceOut(t * 2.0f);
		else
			return ibounceOut((t - 0.5f) * 2.0f);
	}

	inline float ribounceIn(float t) {
		if(t < 0.5f)
			return bounceIn(t * 2.0f);
		else
			return ibounceIn((t - 0.5f) * 2.0f);
	}

	inline float ribounceInOut(float t) {
		if(t < 0.5f)
			return bounceInOut(t * 2.0f);
		else
			return ibounceInOut((t - 0.5f) * 2.0f);
	}

	
}

} //namespace weave
