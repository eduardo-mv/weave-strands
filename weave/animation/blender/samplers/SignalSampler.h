#pragma once

#include "SamplingTime.h"
#include "weave/system/blender/Blender.h"
#include "weave/system/memory/DataType.h"
#include "weave/system/time/Clock.h"
#include "weave/system/math/Easing.h"
#include "weave/system/math/Interpolation.h"

namespace weave::blender::data {
	using namespace weave::blender;
	using namespace weave::types;

class SignalSampler : public BlenderNode<
	Uniform<SamplingTime>,
	In<float, float, float, float>,
	Out<float>>
{

private:
	weave::easing::EasingCurve curve;
	float signalLength = 1.0f;
	float signalLoops = 0.0f;
	float mirror = 1.0f; //A value of 2.0f will be used to mirror the curve's length
		
	float currentTimeScale = 1.0f;
	float localTimeOffset = 0.0f;

public:
	SignalSampler(weave::easing::EasingCurve curve, float minValue = 0.0f, float maxValue = 1.0f, float signalLength = 1.0f, float signalLoops = 0.0f, bool doMirror = false)
		: curve(std::move(curve))
		, signalLength(signalLength)
		, signalLoops(signalLoops)
		, mirror(doMirror ? 2.0f : 1.0f) {

		this->input.SetDefaultValues(1.0f, 0.0f, minValue, maxValue);
	}

	void ExecuteNode() override {
		SamplingTime const& samplingTime = this->uniform.Ref<0>();
		float timeScale = this->input.Ref<0>();
		float timeOffset = this->input.Ref<1>();
		float minValue = this->input.Ref<2>();
		float maxValue = this->input.Ref<3>();
			
		// Adjust the offset to react to a time scaling change
		if (timeScale != currentTimeScale) {
			float localTimeCurrent = GlobalToLocalTime(signalLength, samplingTime.globalTimeStart, samplingTime.globalTimeNow, currentTimeScale, localTimeOffset + timeOffset, signalLoops);
			float localTimeNew = GlobalToLocalTime(signalLength, samplingTime.globalTimeStart, samplingTime.globalTimeNow, timeScale, localTimeOffset + timeOffset, signalLoops);

			localTimeOffset += localTimeCurrent - localTimeNew;
			currentTimeScale = timeScale;
		}

		// Get the local time from the current global time
		float localTime = GlobalToLocalTime(signalLength * mirror, samplingTime.globalTimeStart, samplingTime.globalTimeNow, timeScale, localTimeOffset + timeOffset, signalLoops);
		localTime /= signalLength;
		if (localTime > 1.0f) {
			localTime = mirror - localTime;
		}

		// Sample the easing curve and use the value as interpolation factor between values
		this->output.Ref<0>() = interpolation::lerp(minValue, maxValue, curve(localTime));

	}
};

}
