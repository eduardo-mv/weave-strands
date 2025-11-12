#pragma once

#include "weave/system/blender/Blender.h"
#include "weave/system/memory/DataType.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

namespace weave::blender::data {

using namespace weave::blender;
using namespace weave::types;

struct DataClip {

	struct Channel {
		DataType dataType;
		size_t dataUnitByteSize;
		size_t sampleCount = 0;
		std::vector<std::byte> data;

		Channel(DataType dataType, size_t dataUnitByteSize);

		std::byte const* GetSample(size_t sample) const;
	};

	size_t clipSamples = 0;
	bool nonLoopHoldFrame = false;
	float clipLength = 0.0f;
	float samplingRate = 30.0f;
	std::vector<Channel> channels;

	const std::byte* GetSample(size_t channelNum, size_t sample) const;

	void UpdateClipLength();

	float GetClipRangeLength(int64_t firstFrame, int64_t lastFrame) const;

private:
	size_t GetMaxSampleCount() const;
};

struct DataClipView {
	std::shared_ptr<DataClip> clip;
	size_t startFrame;
	size_t endFrame;
	float clipRangeLength;

public:
	DataClipView(std::shared_ptr<DataClip> clip);
	DataClipView(std::shared_ptr<DataClip> clip, size_t startFrame, size_t endFrame);

	std::byte const* GetSample(size_t channelNum, size_t sample) const;

	auto operator->() const { return clip.operator->(); }
	auto operator->() { return clip.operator->(); }
};

} // namespace weave::blender::data