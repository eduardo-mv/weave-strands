#include "DataClip.h"

namespace weave::blender::data {

DataClip::Channel::Channel(DataType dataType, size_t dataUnitByteSize)
	: dataType(dataType), dataUnitByteSize(dataUnitByteSize) {}

const std::byte* DataClip::Channel::GetSample(size_t sample) const {
	if (data.empty()) {
		return nullptr;
	}
	size_t sampleByte = std::clamp(sample, size_t(0), sampleCount - 1) * dataUnitByteSize;
	return &data[sampleByte];
}

const std::byte* DataClip::GetSample(size_t channelNum, size_t sample) const {
	if (channels.empty()) {
		return nullptr;
	}
	auto& channel = channels[channelNum % channels.size()];
	return channel.GetSample(sample);
}

void DataClip::UpdateClipLength() {
	clipSamples = GetMaxSampleCount();
	clipLength = clipSamples / samplingRate;
}

float DataClip::GetClipRangeLength(int64_t firstFrame, int64_t lastFrame) const {
	float clipRangeLength = static_cast<float>(std::clamp(lastFrame - firstFrame + 1, int64_t(0), static_cast<int64_t>(clipSamples))) / samplingRate;
	return clipRangeLength;
}

size_t DataClip::GetMaxSampleCount() const {
	size_t maxSampleCount = 0;
	for (const auto& channel : channels) {
		maxSampleCount = std::max(maxSampleCount, channel.sampleCount);
	}
	return maxSampleCount;
}

DataClipView::DataClipView(std::shared_ptr<DataClip> clip)
	: clip(clip)
	, startFrame(0)
	, endFrame(clip->clipSamples)
	, clipRangeLength(clip->GetClipRangeLength(startFrame, endFrame)) {}

DataClipView::DataClipView(std::shared_ptr<DataClip> clip, size_t startFrame, size_t endFrame)
	: clip(clip)
	, startFrame(startFrame)
	, endFrame(endFrame)
	, clipRangeLength(clip->GetClipRangeLength(startFrame, endFrame)) {}

const std::byte* DataClipView::GetSample(size_t channelNum, size_t sample) const {
	return clip->GetSample(channelNum, std::clamp(startFrame + sample, startFrame, endFrame));
}

} // namespace weave::blender::data
