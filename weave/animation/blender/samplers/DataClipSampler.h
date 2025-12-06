#pragma once

#include "DataClip.h"
#include "SamplingTime.h"
#include "weave/system/blender/Blender.h"
#include "weave/system/blender/GraphTime.h"
#include "weave/system/memory/DataType.h"
#include "weave/system/math/Interpolation.h"
#include <algorithm>


namespace weave::blender::data {
	using namespace weave::blender;
	using namespace weave::types;

template<typename ...Types>
class DataClipSampler : public BlenderNode<
	Uniform<GraphTime>,
	In<float, float>,
	Out<Types...>>
{
public:
	enum InputIndex : size_t {
		TimeScaleInput,  // Playback speed multiplier
		TimeOffsetInput  // Phase offset in seconds
	};

	enum OutputIndex : size_t {
		ClipSampleOutput // Sampled clip channel value(s)
	};

private:
	DataClipView clip;
	float clipLoops = 0.0f;
		
	float currentTimeScale = 1.0f;
	float localTimeOffset = 0.0f;

public:
	DataClipSampler(std::shared_ptr<DataClip> clip)
		: clip(clip) {
		this->input.SetDefaultValues(1.0f, 0.0f);
	}

	DataClipSampler(std::shared_ptr<DataClip> clip, float clipLoops) 
		: clip(clip)
		, clipLoops(clipLoops) {
		this->input.SetDefaultValues(1.0f, 0.0f);
	}

	DataClipSampler(DataClipView clipView)
		: clip(clipView) {
		this->input.SetDefaultValues(1.0f, 0.0f);
	}

	DataClipSampler(DataClipView clipView, float clipLoops)
		: clip(clipView)
		, clipLoops(clipLoops) {
		this->input.SetDefaultValues(1.0f, 0.0f);
	}

	void ExecuteNode() override {
		GraphTime const& samplingTime = this->uniform.template Ref<0>();
		float timeScale = this->input.template Ref<TimeScaleInput>();
		float timeOffset = this->input.template Ref<TimeOffsetInput>();
			
		// Adjust the offset to react to a time scaling change
		if (timeScale != currentTimeScale) {
			float localTimeCurrent = GlobalToLocalTime(clip.clipRangeLength, static_cast<float>(samplingTime.totalSeconds), currentTimeScale, localTimeOffset + timeOffset, clipLoops);
			float localTimeNew = GlobalToLocalTime(clip.clipRangeLength, static_cast<float>(samplingTime.totalSeconds), timeScale, localTimeOffset + timeOffset, clipLoops);

			localTimeOffset += localTimeCurrent - localTimeNew;
			currentTimeScale = timeScale;
		}

		// Get the local time from the current global time
		float localTime = GlobalToLocalTime(clip.clipRangeLength, static_cast<float>(samplingTime.totalSeconds), timeScale, localTimeOffset + timeOffset, clipLoops);

		// Find the two frames between which the time value lies
		float frameTime = clip->samplingRate * localTime;
		float frameA = std::floor(frameTime);
		float frameB = std::ceil(frameTime);
		// Find the interpolation factor based on the time fraction
		float u = frameTime - frameA;

		size_t numSampleA = size_t(frameA) % clip->clipSamples;
		size_t numSampleB = clip->nonLoopHoldFrame ? std::clamp(size_t(frameB), size_t(0), clip->clipSamples) : size_t(frameB) % clip->clipSamples;

		SampleClipChannel<0>(clip, numSampleA, numSampleB, u);
	}

	// This samples clip channels individually mapping them to a sample output to its desired type.
	template<size_t N>
	void SampleClipChannel(DataClipView const& clip, size_t numSampleA, size_t numSampleB, float u) {
		if constexpr (N < this->output.GetOutputCount()) {
			if (N < clip->channels.size()) {
				// Take two samples from the clip and interpolate into the output
				auto& sampleOut = this->output.template Ref<N>();
				auto& traits = GetRuntimeTypeTraits(clip->channels[N].dataType);
					
				using SampleType = std::decay_t<decltype(sampleOut)>;
				SampleType sampleA = DynamicTypeConvert<SampleType>(clip.GetSample(N, numSampleA), traits);
				SampleType sampleB = DynamicTypeConvert<SampleType>(clip.GetSample(N, numSampleB), traits);
					
				sampleOut = weave::interpolation::lerp(sampleA, sampleB, u);

				// Next channel
				SampleClipChannel<N + 1>(clip, numSampleA, numSampleB, u);
			}
		}
	}
	
};

}
