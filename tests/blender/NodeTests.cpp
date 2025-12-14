#include "tests/TestEntryPoints.h"

#include "weave/system/blender/Blender.h"
#include "weave/system/blender/GraphTime.h"
#include "weave/system/blender/nodes/ArithmeticNodes.h"
#include "weave/animation/blender/samplers/DataClipSampler.h"
#include "weave/animation/blender/samplers/SignalSampler.h"
#include "weave/system/math/Interpolation.h"

#include <chrono>
#include <cmath>
#include <format>
#include <iostream>
#include <memory>
#include <thread>

namespace weave::tests::blender {
namespace {

using namespace weave;
using namespace weave::blender;
using namespace weave::blender::data;

struct MatrixNode : BlenderNode<In<float, float, int>, Out<weave::Matrix4x4>> {
	void ExecuteNode() override {
		output.Ref<weave::Matrix4x4>() = weave::Matrix4x4{} * input.Ref<0>() * input.Ref<1>();
	}
};

struct SomeNode
	: BlenderNode<Out<weave::Matrix4x4>, In<weave::Matrix4x4, weave::Matrix4x4, weave::Matrix3x3, float, bool>> {

	void ExecuteNode() override {
		if (input.Value<4>()) {
			output = input.Value<0>() * input.Value<1>() * input.Value<3>();
			output = input.Value<0>();
		}
	}
};

struct SomeExternalBool : BlenderNode<Out<bool>> {
	bool setMe = false;

	void ExecuteNode() override {
		output = setMe;
	}
};

struct PrintFloats : BlenderNode<In<float, float>> {
	void ExecuteNode() override {
		std::cout << "Input 1: " << input.Value<0>() << std::endl;
		std::cout << "Input 2: " << input.Value<1>() << std::endl;
	}
};

struct MessageNode : BlenderNode<> {
	std::string message;

	explicit MessageNode(std::string msg) : message(std::move(msg)) {}

	void ExecuteNode() override {
		std::cout << message << std::endl;
	}
};

class SineWaveNode : public BlenderNode<Uniform<GraphTime>, Out<float>> {
public:
	SineWaveNode(float frequency, float amplitude)
		: frequency(frequency), amplitude(amplitude) {}

	void ExecuteNode() override {
		float time = static_cast<float>(uniform.Ref<GraphTime>().totalSeconds);
		this->output.Ref<0>() = amplitude * std::sin(2.0f * 3.14159265f * frequency * time);
	}

private:
	float frequency;
	float amplitude;
};

struct AlwaysMessageNode : BlenderNode<> {
	std::string message;

	explicit AlwaysMessageNode(std::string msg) : message(std::move(msg)) {}

	void ExecuteNode() override {
		DisableCache();
	}
};

[[maybe_unused]] DataClip CreateSampleData() {
	DataClip dataClip;
	dataClip.samplingRate = 24.0f;

	// Channel 0: 2 seconds of a float sin wave
	float sinWaveDuration = 2.0f;
	float sinWaveFrequency = 1.0f;
	size_t sinWaveSampleCount = static_cast<size_t>(sinWaveDuration * dataClip.samplingRate);
	DataClip::Channel sinWaveChannel(DataType::Float, sizeof(float));
	sinWaveChannel.sampleCount = sinWaveSampleCount;
	sinWaveChannel.data.resize(sinWaveSampleCount * sizeof(float));

	for (size_t i = 0; i < sinWaveSampleCount; ++i) {
		float time = i / dataClip.samplingRate;
		float sinValue = std::sin(2 * 3.14159f * sinWaveFrequency * time);
		reinterpret_cast<float*>(sinWaveChannel.data.data())[i] = sinValue;
	}
	dataClip.channels.push_back(std::move(sinWaveChannel));

	// Channel 1: 0.5 seconds of a float square wave
	float squareWaveDuration = 0.5f;
	float squareWaveFrequency = 2.0f;
	size_t squareWaveSampleCount = static_cast<size_t>(squareWaveDuration * dataClip.samplingRate);
	DataClip::Channel squareWaveChannel(DataType::Float, sizeof(float));
	squareWaveChannel.sampleCount = squareWaveSampleCount;
	squareWaveChannel.data.resize(squareWaveSampleCount * sizeof(float));

	for (size_t i = 0; i < squareWaveSampleCount; ++i) {
		float time = i / dataClip.samplingRate;
		float squareValue = (std::sin(2 * 3.14159f * squareWaveFrequency * time) >= 0) ? 1.0f : -1.0f;
		reinterpret_cast<float*>(squareWaveChannel.data.data())[i] = squareValue;
	}
	dataClip.channels.push_back(std::move(squareWaveChannel));

	// Channel 2: 2 seconds of an integer sequence starting from 0 going up
	float intSeqDuration = 2.0f;
	size_t intSeqSampleCount = static_cast<size_t>(intSeqDuration * dataClip.samplingRate);
	DataClip::Channel intSeqChannel(DataType::Int32, sizeof(int));
	intSeqChannel.sampleCount = intSeqSampleCount;
	intSeqChannel.data.resize(intSeqSampleCount * sizeof(int));

	for (size_t i = 0; i < intSeqSampleCount; ++i) {
		reinterpret_cast<int*>(intSeqChannel.data.data())[i] = static_cast<int>(i);
	}
	dataClip.channels.push_back(std::move(intSeqChannel));

	dataClip.UpdateClipLength();

	return dataClip;
}

std::shared_ptr<DataClip> CreateSingleChannelFloatDataClip() {
	constexpr float kPi = 3.14159265358979323846f;

	size_t sampleCount = static_cast<size_t>(10.0f * 30.0f);

	auto dataClip = std::make_shared<DataClip>();
	dataClip->samplingRate = 30.0f;
	dataClip->channels.emplace_back(weave::types::DataType::Float, sizeof(float));
	dataClip->channels.back().sampleCount = sampleCount;

	dataClip->channels.back().data.reserve(sampleCount * sizeof(float));

	for (size_t i = 0; i < sampleCount; ++i) {
		float t = static_cast<float>(i) / dataClip->samplingRate;
		float value = static_cast<float>(std::sin(2.0f * kPi * t) * std::cos(4.0f * kPi * t));
		auto* valuePtr = reinterpret_cast<std::byte*>(&value);
		dataClip->channels.back().data.insert(dataClip->channels.back().data.end(), valuePtr, valuePtr + sizeof(float));
	}

	dataClip->UpdateClipLength();

	return dataClip;
}

[[maybe_unused]] std::shared_ptr<DataClip> CreateDampedSineWaveClip(float duration, float dampingFactor) {
	size_t sampleCount = static_cast<size_t>(std::ceil(duration * 30.0f));

	auto clip = std::make_shared<DataClip>();
	clip->channels.emplace_back(weave::types::DataType::Float, sizeof(float));
	clip->channels.back().sampleCount = sampleCount;
	clip->channels.back().data.resize(sampleCount * sizeof(float));
	clip->samplingRate = 30.0f;
	clip->UpdateClipLength();

	float timeStep = duration / sampleCount;
	for (size_t i = 0; i < sampleCount; ++i) {
		float t = i * timeStep;
		float amplitude = std::exp(-dampingFactor * t);
		float value = static_cast<float>(amplitude * std::sin(2.0f * 3.14159265f * t));
		reinterpret_cast<float*>(clip->channels[0].data.data())[i] = value;
	}

	return clip;
}

} // namespace

TestReport TestNodes() {
	TestReport report;

	{
		Blender blender;

		auto constantFloat = blender.CreateNode<ConstNode<float>>(5.0f);
		auto someMult = blender.CreateNode<MultiplyNode<float, float>>(2.0f, 2.0f);
		auto someNode = blender.CreateNode<SomeNode>();
		auto constantFloatInt = blender.CreateNode<ConstNode<float, int>>(10.0f, 20);
		auto matrixNode = blender.CreateNode<MatrixNode>();
		auto printFloats = blender.CreateNode<PrintFloats>();
		auto someBool = blender.CreateNode<SomeExternalBool>();
		someBool->setMe = true;

		auto message = blender.CreateNode<MessageNode>("I'm a SomeMult Trigger...");
		auto messageTrig2 = blender.CreateNode<MessageNode>("I'm a SomeMult & PrintFloats Trigger...");

		auto root = blender.CreateNode<MessageNode>("Start of the tree!");
		root->ConnectOutflowLink(someMult);
		root->ConnectOutflowLink(blender.CreateNode<MessageNode>("Second node"));

		someMult->ConnectInputTo(0, constantFloat, 0);
		someMult->ConnectOutflowLink(message);
		someMult->ConnectOutflowLink(messageTrig2);
		someMult->ConnectOutflowLink(someNode);
		someMult->ConnectOutflowLink(printFloats);
		printFloats->ConnectOutflowLink(messageTrig2);

		someNode->ConnectInputTo(0, constantFloat, 0);
		someNode->ConnectInputTo(0, constantFloat, 1);
		someNode->ConnectInputTo(0, constantFloatInt, 3);
		someNode->ConnectInputTo(0, someBool, 4);
		someNode->ConnectInputTo(0, matrixNode, 1);

		matrixNode->ConnectInputTo(0, someNode, 0);
		printFloats->ConnectInputTo(0, someMult, 0);
		printFloats->ConnectInputTo(0, constantFloat, 1);

		blender.RegisterNodeName(constantFloat, "Constant Float");
		blender.RegisterOutputName(constantFloat, 0, "Float");

		blender.RegisterNodeName(printFloats, "Print Floats");
		blender.RegisterInputName(printFloats, 0, "Float 0");
		blender.RegisterInputName(printFloats, 1, "Float 1");

		Connection con;
		con.fromOutputNode = "Constant Float";
		con.outputIndex = 0ull;
		con.toInputNode = printFloats;
		con.inputIndex = 0ull;

		blender.Connect(con);

		auto outptr = blender.ExposeOutput<0>(someMult, "X");

		blender.AddRootFlowLink(root);
		blender.Execute();

		std::cout << std::format("\nOutPtr says: {}", *outptr);

		auto inTypes = printFloats->GetInputTypes();
		for (auto t : inTypes) {
			std::cout << std::format("\n{}", t.name());
		}
	}

	{
		Blender blender;

		NodeIoNames constNodeNames;
		constNodeNames.outputNames = { { "value", 0 } };

		NodeIoNames addNodeNames;
		addNodeNames.inputNames = { { "a", 0 }, { "b", 1 } };
		addNodeNames.outputNames = { { "sum", 0 } };

		NodeIoNames multiplyNodeNames;
		multiplyNodeNames.inputNames = { { "a", 0 }, { "b", 1 } };
		multiplyNodeNames.outputNames = { { "product", 0 } };

		NodeIoNames subtractNodeNames;
		subtractNodeNames.inputNames = { { "a", 0 }, { "b", 1 } };
		subtractNodeNames.outputNames = { { "difference", 0 } };

		[[maybe_unused]] auto const1 = blender.CreateNamedNode<ConstNode<float>>({ "const1", constNodeNames }, 2.0f);
		[[maybe_unused]] auto const2 = blender.CreateNamedNode<ConstNode<float>>({ "const2", constNodeNames }, 3.0f);
		[[maybe_unused]] auto const3 = blender.CreateNamedNode<ConstNode<float>>({ "const3", constNodeNames }, 4.0f);
		[[maybe_unused]] auto addNode = blender.CreateNamedNode<AddNode<float, float>>({ "addNode", addNodeNames });
		[[maybe_unused]] auto multiplyNode = blender.CreateNamedNode<MultiplyNode<float, float>>({ "multiplyNode", multiplyNodeNames });
		[[maybe_unused]] auto subtractNode = blender.CreateNamedNode<SubtractNode<float, float>>({ "subtractNode", subtractNodeNames });

		blender.Connect({ "const1", "value", "addNode", "a" });
		blender.Connect({ "const2", "value", "addNode", "b" });
		blender.Connect({ "addNode", "sum", "multiplyNode", "a" });
		blender.Connect({ "const3", "value", "multiplyNode", "b" });
		blender.Connect({ "multiplyNode", "product", "subtractNode", "a" });
		blender.Connect({ "const1", "value", "subtractNode", "b" });

		blender.AddRootFlowLink(subtractNode);
		blender.Execute();

		const float result = subtractNode->output.Ref<0>();
		report.Expect(std::abs(result - 18.0f) < 1e-4f, "Arithmetic node chain produced unexpected result");

		Uniform<int, float> u{ blender.GetUniforms<int, float>() };
		u.Set(int(100));
	}

	{
		Blender blender;

		auto time = blender.GetUniform<GraphTime>();

		auto data = CreateSingleChannelFloatDataClip();
		auto* sampler = blender.CreateNode<DataClipSampler<float, float, int32_t>>(data, 0.0f);
		auto* signal = blender.CreateNode<SignalSampler>(weave::easing::expInOut);

		auto add = blender.CreateNode<AddNode<float, float>>(1.0f, 1.0f);
		add->ConnectInputTo<0, 0>(signal);
		add->ConnectInputTo<1, 0>(sampler);

		time->totalSeconds = 0.0;
		time->deltaSeconds = 1.0f / 24.0f;

		blender.AddRootFlowLink(sampler);

		auto scaleControl = std::make_shared<float>(2.0f);
		sampler->ConnectInputTo<0>(scaleControl);
		sampler->ConnectOutflowLink(add);

		auto sineWave1 = blender.CreateNode<SineWaveNode>(1.0f, 0.5f);
		auto sineWave2 = blender.CreateNode<SineWaveNode>(0.25f, 1.0f);

		auto smoothstep = blender.CreateNode<SmoothstepNode<float>>();
		smoothstep->ConnectInputTo<0, 0>(sineWave1);
		smoothstep->ConnectInputTo<1, 0>(sineWave2);
		smoothstep->ConnectInputTo<2, 0>(add);
		sampler->ConnectOutflowLink(smoothstep);

		auto always = blender.CreateNode<AlwaysMessageNode>("Hello!\n");
		blender.AddRootFlowLink(always);
		sampler->ConnectOutflowLink(always);

		[[maybe_unused]] auto out0 = blender.ExposeOutput<0>(sampler, "SamplerOutput0");
		[[maybe_unused]] auto out1 = blender.ExposeOutput<0>(sampler, "SamplerOutput1");
		[[maybe_unused]] auto out2 = blender.ExposeOutput<0>(sampler, "SamplerOutput2");

		auto smoothOut = blender.ExposeOutput<0>(smoothstep, "SmoothOutput");

		float firstSmooth = 0.0f;
		float lastSmooth = 0.0f;

		for (int i = 0, t = 10; i < t; ++i) {
			blender.Execute();
			auto a = sampler->output.Ref<0>();
			auto b = sampler->output.Ref<1>();
			auto c = sampler->output.Ref<2>();
			auto d = signal->output.Ref<0>();
			auto e = add->output.Ref<0>();
			auto f = *smoothOut;

			auto drawValue = [&](auto v) {
				int xmov = static_cast<int>(10.0f * v) + 10;
				for (int j = 0; j < xmov; ++j) {
					std::cout << " ";
				}
				std::cout << "*\n";
			};

			drawValue(f);

			(void)a;
			(void)b;
			(void)c;
			(void)d;
			(void)e;
			(void)f;
			if (i == 0) {
				firstSmooth = f;
			}
			lastSmooth = f;

			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			time->totalSeconds += time->deltaSeconds;
			*scaleControl -= 0.01f;
		}

		report.Expect(std::abs(lastSmooth - firstSmooth) > 1e-3f, "Smooth output did not change over time");
		report.Expect(time->totalSeconds > 0.0, "Graph time did not advance");
		report.Expect(*scaleControl < 2.0f, "Scale control was not updated");
	}

	return report;
}

} // namespace weave::tests::blender
