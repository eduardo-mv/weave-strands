#pragma once

#include <algorithm>
#include <cstddef>

#include "SamplingTime.h"
#include "weave/system/blender/Blender.h"
#include "weave/system/blender/GraphTime.h"
#include "weave/system/memory/DataType.h"
#include "weave/system/time/Clock.h"
#include "weave/system/math/Easing.h"
#include "weave/system/math/Interpolation.h"

namespace weave::blender::data {
	using namespace weave::blender;
	using namespace weave::types;

class SignalSampler : public BlenderNode<
	Uniform<GraphTime>,
	In<float, float, float, float, float, float, float>,
	Out<float>>
{
public:
	enum InputIndex : size_t {
		TimeScaleInput,   // Playback speed multiplier
		TimeOffsetInput,  // Phase offset applied before sampling (seconds)
		MinValueInput,    // Lower interpolation bound
		MaxValueInput,    // Upper interpolation bound
		SignalLengthInput,// Duration of the easing segment (seconds)
		SignalLoopsInput, // Number of loops before clamping (0 = infinite)
		MirrorInput       // Mirror factor (1 = off, >1 mirrors the curve)
	};

	enum OutputIndex : size_t {
		ResultOutput // Sampled easing value
	};

	explicit SignalSampler(weave::easing::EasingCurve curve = weave::easing::linear)
		: curve(std::move(curve)) {
		this->input.SetDefaultValues(1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
	}

	void ExecuteNode() override {
		GraphTime const& samplingTime = this->uniform.Ref<0>();
		const float timeScale = this->input.template Ref<TimeScaleInput>();
		const float timeOffset = this->input.template Ref<TimeOffsetInput>();
		const float minValue = this->input.template Ref<MinValueInput>();
		const float maxValue = this->input.template Ref<MaxValueInput>();
		const float signalLength = std::max(this->input.template Ref<SignalLengthInput>(), 1e-4f);
		const float signalLoops = this->input.template Ref<SignalLoopsInput>();
		const float mirror = std::max(this->input.template Ref<MirrorInput>(), 1.0f);

		// Adjust the offset to react to a time scaling change
		if (timeScale != currentTimeScale) {
			const float localTimeCurrent = GlobalToLocalTime(signalLength, static_cast<float>(samplingTime.totalSeconds), currentTimeScale, localTimeOffset + timeOffset, signalLoops);
			const float localTimeNew = GlobalToLocalTime(signalLength, static_cast<float>(samplingTime.totalSeconds), timeScale, localTimeOffset + timeOffset, signalLoops);

			localTimeOffset += localTimeCurrent - localTimeNew;
			currentTimeScale = timeScale;
		}

		// Get the local time from the current global time
		const float mirroredLength = signalLength * mirror;
		float localTime = GlobalToLocalTime(mirroredLength, static_cast<float>(samplingTime.totalSeconds), timeScale, localTimeOffset + timeOffset, signalLoops);
		localTime /= signalLength;
		if (mirror > 1.0f && localTime > 1.0f) {
			localTime = mirror - localTime;
		}

		// Sample the easing curve and use the value as interpolation factor between values
		this->output.template Ref<ResultOutput>() = interpolation::lerp(minValue, maxValue, curve(localTime));

	}

private:
	weave::easing::EasingCurve curve;
	float currentTimeScale = 1.0f;
	float localTimeOffset = 0.0f;
};

}
