// WeaveInput.cpp : Defines the entry point for the application.
//

#include "WeaveTests.h"
#ifdef _WIN32
#include "weave/platform/win32/Win32Exceptions.h"
#include "weave/platform/win32/Win32Window.h"
#include "weave/platform/win32/Win32OpenGL.h"
#else
#include "weave/platform/wayland/WaylandExceptions.h"
#include "weave/platform/wayland/WaylandWindow.h"
#include "weave/platform/wayland/WaylandOpenGL.h"
#endif

#include "weave/platform/graphics/ImageData.h"
#include "weave/platform/graphics/ImageLoader.h"
#include "weave/platform/graphics/MeshData.h"
#include "weave/platform/graphics/MeshLoader.h"
#include "weave/platform/graphics/Primitives3D.h"

#include "weave/platform/graphics/gl/ApiHelpers.h"
#include "weave/platform/graphics/gl/Framebuffer.h"
#include "weave/platform/graphics/gl/Texture.h"
#include "weave/platform/graphics/gl/TextureLoader.h"
#include "weave/platform/graphics/gl/Shader.h"
#include "weave/platform/graphics/gl/ShaderLoader.h"
#include "weave/platform/graphics/gl/Mesh.h"
#include "weave/platform/graphics/gl/MeshUploader.h"
#include "weave/platform/graphics/gl/Buffer.h"



#include "weave/input/system/InputPipeline.h"
#include "weave/input/processors/InputStateWriter.h"
#include "weave/input/processors/InputRouter.h"
#include "weave/input/processors/SingleDevice.h"
#include "weave/input/processors/InputEventContext.h"

#ifdef _WIN32
#include "weave/input/win32/Win32KeyboardFeed.h"
#include "weave/input/win32/Win32MouseFeed.h"
#include "weave/input/rawinput/RawInputGamepad.h"
#include "weave/input/rawinput/RawInputKeyboardFeed.h"
#include "weave/input/rawinput/RawInputMouseFeed.h"
#else
#include "weave/input/wayland/WaylandSeat.h"
#include "weave/input/wayland/WaylandGamepadFeed.h"
#include "weave/input/wayland/WaylandMidiFeed.h"
#endif
#include "weave/input/system/GamepadLayouts.h"

#include "weave/system/math/VectorMath.h"
#include "weave/system/time/Clock.h"

#include <thread>
#include <functional>
#include <format>
#include <type_traits>
#include <utility>
#include <cmath>
#include <chrono>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <system_error>
#include <array>
#include <algorithm>

#include "weave/system/blender/Blender.h"
#include "weave/particles/nodes/ArithmeticNodes.h"
#include "weave/animation/blender/samplers/DataClipSampler.h"
#include "weave/animation/blender/samplers/SignalSampler.h"

#include "weave/particles/ParticleBuffer.h"
#include "weave/particles/ParticleMachine.h"
#include "weave/particles/nodes/ParticleNode.h"
#include "weave/particles/nodes/ParticleEmitter.h"
#include "weave/particles/nodes/CommitEmissionNode.h"

#include "weave/system/math/Interpolation.h"
#include "weave/scenegraph/SceneGraph.h"
/*
Port Anim to Blenders
*/

using namespace weave;
using namespace weave::blender;

namespace {

using weave::algebra::equivalent;

std::array<float, 3> HslToRgb(float h, float s, float l) {
	auto clamp01 = [](float v) { return std::clamp(v, 0.0f, 1.0f); };
	h = std::fmod(h, 1.0f);
	if (h < 0.0f) {
		h += 1.0f;
	}
	s = clamp01(s);
	l = clamp01(l);

	if (s == 0.0f) {
		return { l, l, l };
	}

	auto hue2rgb = [](float p, float q, float t) {
		if (t < 0.0f) t += 1.0f;
		if (t > 1.0f) t -= 1.0f;
		if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
		if (t < 0.5f) return q;
		if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
		return p;
	};

	float q = (l < 0.5f) ? l * (1.0f + s) : (l + s - l * s);
	float p = 2.0f * l - q;
	return {
		hue2rgb(p, q, h + 1.0f / 3.0f),
		hue2rgb(p, q, h),
		hue2rgb(p, q, h - 1.0f / 3.0f)
	};
}

struct InputEffectSharedState {
	static constexpr size_t kBarCount = 108;
	struct MidiBar {
		float targetHeight{ 0.0f };
		uint8_t note{ 0 };
		bool active{ false };
		float highlight{ 0.0f };
	};

	std::array<MidiBar, kBarCount> bars{};
	std::array<bool, 128> noteActive{};
	mutable std::mutex barsMutex;
};

InputEffectSharedState& GetInputEffectSharedState() {
	static InputEffectSharedState state;
	return state;
}

struct MyBuffer {
	float color[4] = { 0.1f, -0.5f, 0.0f, 1.0f };
	float crap[4] = { 0.2f, 0.3f, 0.4f, 0.5f };
	float x = 1.0f;
};

struct BarVertex {
	float position[2];
	float color[3];
};

struct ParticleTestWriter : particles::ParticleNode<> {
	ParticleTestWriter(float lifeValue, float maxLifeValue)
		: lifeValue(lifeValue), maxLifeValue(maxLifeValue) {}

	void ExecuteNode() override {
		wroteCount = 0;
		auto& context = GetContext();
		for (auto* buffer : context.buffers) {
			if (!buffer) {
				continue;
			}
			for (auto&& [life, maxLife] : buffer->EmissionSpan<particles::layout::LifeTime, particles::layout::MaxLifeTime>()) {
				life.lifeTime = lifeValue;
				maxLife.maxLifeTime = maxLifeValue;
				++wroteCount;
			}
		}
	}

	float lifeValue{};
	float maxLifeValue{};
	size_t wroteCount{};
};

struct ParticleTestInspector : particles::ParticleNode<> {
	ParticleTestInspector(float lifeValue, float maxLifeValue)
		: lifeValue(lifeValue), maxLifeValue(maxLifeValue) {}

	void ExecuteNode() override {
		verifiedCount = 0;
		mismatchCount = 0;
		auto& context = GetContext();
		for (auto* buffer : context.buffers) {
			if (!buffer) {
				continue;
			}
			for (auto&& [life, maxLife] : buffer->EditableSpan<particles::layout::LifeTime, particles::layout::MaxLifeTime>()) {
				const bool matches = std::abs(life.lifeTime - lifeValue) < 1e-5f
					&& std::abs(maxLife.maxLifeTime - maxLifeValue) < 1e-5f;
				if (matches) {
					++verifiedCount;
				} else {
					++mismatchCount;
				}
			}
		}
	}

	float lifeValue{};
	float maxLifeValue{};
	size_t verifiedCount{};
	size_t mismatchCount{};
};

} // namespace

struct MatrixNode : BlenderNode<In<float, float, int>, Out<weave::Matrix4x4>> {
	void ExecuteNode() override {
		output.Ref<weave::Matrix4x4>() = weave::Matrix4x4{};
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

struct SomeExternalBool
	: BlenderNode<Out<bool>> {

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

struct MessageNode
	: BlenderNode<> {
	std::string message;

	MessageNode(std::string msg) : message(msg) {}

	void ExecuteNode() override {
		std::cout << message << std::endl;
	}
};

class SineWaveNode : public BlenderNode<Uniform<SamplingTime>, Out<float>> {
public:
	SineWaveNode(float frequency, float amplitude)
		: frequency(frequency), amplitude(amplitude) {}

	void ExecuteNode() override {
		float time = uniform.Ref<SamplingTime>().globalTimeNow;
		this->output.Ref<0>() = amplitude * std::sin(2.0f * 3.14159265f * frequency * time);
	}

private:
	float frequency;
	float amplitude;
};

struct AlwaysMessageNode
	: BlenderNode<> {
	std::string message;

	AlwaysMessageNode(std::string msg) : message(msg) {}

	void ExecuteNode() override {
		//std::cout << message << " " << executionStamp << std::endl;
		DisableCache();
	}
};

using namespace weave::blender::data;

DataClip CreateSampleData() {
	DataClip dataClip;
	dataClip.samplingRate = 24.0f;

	// Channel 0: 2 seconds of a float sin wave
	float sinWaveDuration = 2.0f;
	float sinWaveFrequency = 1.0f; // Choose any frequency
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
	float squareWaveFrequency = 2.0f; // Choose any frequency
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

std::shared_ptr<weave::blender::data::DataClip> CreateSingleChannelFloatDataClip() {
#define M_PI 3.14159265358979323846
	using namespace weave::blender::data;

	// Calculate the number of samples required for 10 seconds at 30 fps
	size_t sampleCount = static_cast<size_t>(10.0f * 30.0f);

	// Create a DataClip with a single float channel
	auto dataClip = std::make_shared<DataClip>();
	dataClip->samplingRate = 30.0f;
	dataClip->channels.emplace_back(weave::types::DataType::Float, sizeof(float));
	dataClip->channels.back().sampleCount = sampleCount;

	// Reserve memory for the samples
	dataClip->channels.back().data.reserve(sampleCount * sizeof(float));

	// Generate animated data for the channel
	for (size_t i = 0; i < sampleCount; ++i) {
		float t = static_cast<float>(i) / dataClip->samplingRate;
		float value = (float) (std::sin(2.0f * M_PI * t) * std::cos(4.0f * M_PI * t));
		dataClip->channels.back().data.insert(dataClip->channels.back().data.end(), reinterpret_cast<std::byte*>(&value), reinterpret_cast<std::byte*>(&value) + sizeof(float));
	}

	// Update the clip length
	dataClip->UpdateClipLength();

	return dataClip;
}

std::shared_ptr<weave::blender::data::DataClip> CreateDampedSineWaveClip(float duration, float dampingFactor) {
	using namespace weave::blender::data;

	// Calculate the number of samples based on the duration and 30 fps
	size_t sampleCount = static_cast<size_t>(std::ceil(duration * 30.0f));

	// Create a new DataClip with one float channel
	auto clip = std::make_shared<DataClip>();
	clip->channels.emplace_back(weave::types::DataType::Float, sizeof(float));
	clip->channels.back().sampleCount = sampleCount;
	clip->channels.back().data.resize(sampleCount * sizeof(float));
	clip->samplingRate = 30.0f;
	clip->UpdateClipLength();

	// Generate the damped sine wave data
	float timeStep = duration / sampleCount;
	for (size_t i = 0; i < sampleCount; ++i) {
		float t = i * timeStep;
		float amplitude = std::exp(-dampingFactor * t);
		float value = (float)(amplitude * std::sin(2.0f * M_PI * t));
		reinterpret_cast<float*>(clip->channels[0].data.data())[i] = value;
	}

	return clip;
}


void TestNodes() {
	{
		// Create nodes
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

		// Connect nodes
		auto root = blender.CreateNode<MessageNode>("Start of the tree!");
		root->ConnectTrigger(someMult);
		root->ConnectTrigger(blender.CreateNode<MessageNode>("Second node"));

		someMult->ConnectInputTo(0, constantFloat, 0);
		someMult->ConnectTrigger(message);
		someMult->ConnectTrigger(someNode);
		someMult->ConnectTrigger(printFloats);

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

		// Set root node and execute
		blender.AddRootTrigger(root);
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
		addNodeNames.inputNames = { {"a", 0}, {"b", 1} };
		addNodeNames.outputNames = { { "sum", 0 } };

		NodeIoNames multiplyNodeNames;
		multiplyNodeNames.inputNames = { {"a", 0}, {"b", 1} };
		multiplyNodeNames.outputNames = { { "product", 0 } };

		NodeIoNames subtractNodeNames;
		subtractNodeNames.inputNames = { {"a", 0}, {"b", 1} };
		subtractNodeNames.outputNames = { { "difference", 0 } };

		// Creating nodes
		[[maybe_unused]] ConstNode<float>* const1 = blender.CreateNamedNode<ConstNode<float>>({ "const1", constNodeNames}, 2.0f);
		[[maybe_unused]] ConstNode<float>* const2 = blender.CreateNamedNode<ConstNode<float>>({ "const2", constNodeNames}, 3.0f);
		[[maybe_unused]] ConstNode<float>* const3 = blender.CreateNamedNode<ConstNode<float>>({ "const3", constNodeNames}, 4.0f);
		[[maybe_unused]] AddNode<float, float>* addNode = blender.CreateNamedNode<AddNode<float, float>>({ "addNode", addNodeNames });
		[[maybe_unused]] MultiplyNode<float, float>* multiplyNode = blender.CreateNamedNode<MultiplyNode<float, float>>({ "multiplyNode", multiplyNodeNames });
		[[maybe_unused]] SubtractNode<float, float>* subtractNode = blender.CreateNamedNode<SubtractNode<float, float>>({ "subtractNode", subtractNodeNames });
		// Connecting nodes
		blender.Connect({ "const1", "value", "addNode", "a" });
		blender.Connect({ "const2", "value", "addNode", "b" });
		blender.Connect({ "addNode", "sum", "multiplyNode", "a" });
		blender.Connect({ "const3", "value", "multiplyNode", "b" });
		blender.Connect({ "multiplyNode", "product", "subtractNode", "a" });
		blender.Connect({ "const1", "value", "subtractNode", "b" });

		// Setting the root node
		blender.AddRootTrigger(subtractNode);

		// Executing the graph
		blender.Execute();

		// Printing the result
		std::cout << "Result: " << subtractNode->output.Ref<0>() << std::endl;

		Uniform<int, float> u{ blender.GetUniforms<int, float>() };
		u.Set(int(100));

	}

	{
		Blender blender;

		auto time = blender.GetUniform<weave::blender::SamplingTime>();

		//std::shared_ptr<DataClip> data = std::make_shared<DataClip>();
		//*data = CreateSampleData();

		auto data = CreateSingleChannelFloatDataClip();
		//auto data = CreateDampedSineWaveClip(10.0f, 1.1f);
		auto* sampler = blender.CreateNode<DataClipSampler<float, float, int32_t>>(data, 0.0f);
		//auto* sampler2 = blender.CreateNode<DataClipSampler<float>>(CreateDampedSineWaveClip(10.0f, 0.1f), 0.0f);
		auto* signal = blender.CreateNode<SignalSampler>(weave::easing::expInOut, 0.0f, 1.0f, 1.0f, 0.0f, true);

		auto* add = blender.CreateNode<AddNode<float, float>>(1.0f, 1.0f);
		add->ConnectInputTo<0,0>(signal);
		add->ConnectInputTo<1,0>(sampler);

		time->globalTimeStart = 0.0f;
		time->globalTimeNow = 0.0f;
		time->delta = 0.1f;

		blender.AddRootTrigger(sampler);

		std::shared_ptr<float> scaleControl = std::make_shared<float>(2.0f);
		sampler->ConnectInputTo<0>(scaleControl);
		//sampler->ConnectInputTo(0, sampler2, 0);
		sampler->ConnectTrigger(add);


		// Sine wave nodes with different frequencies and amplitudes
		auto sineWave1 = blender.CreateNode<SineWaveNode>(1.0f, 0.5f);
		auto sineWave2 = blender.CreateNode<SineWaveNode>(0.25f, 1.0f);

		// Smoothstep node to blend between the sine waves
		auto smoothstep = blender.CreateNode<SmoothstepNode<float>>();
		smoothstep->ConnectInputTo<0, 0>(sineWave1);
		smoothstep->ConnectInputTo<1, 0>(sineWave2);
		smoothstep->ConnectInputTo<2, 0>(add);

		//smoothstep->input.SetDefaultValue<2>(0.3f); // Blending factor

		sampler->ConnectTrigger(smoothstep);

		auto always = blender.CreateNode<AlwaysMessageNode>("Hello!\n");
		blender.AddRootTrigger(always);
		sampler->ConnectTrigger(always);

		//constexpr auto v = weave::is_specialization_of_v<BlenderNode, AlwaysMessageNode>;

		[[maybe_unused]]auto out0 = blender.ExposeOutput<0>(sampler, "SamplerOutput0");
		[[maybe_unused]]auto out1 = blender.ExposeOutput<0>(sampler, "SamplerOutput1");
		[[maybe_unused]]auto out2 = blender.ExposeOutput<0>(sampler, "SamplerOutput2");

		auto smoothOut = blender.ExposeOutput<0>(smoothstep, "SmoothOutput");

		for (int i = 0, t = 10; i < t; ++i) {
			blender.Execute();
			auto a = sampler->output.Ref<0>();
			auto b = sampler->output.Ref<1>();
			auto c = sampler->output.Ref<2>();

			auto d = signal->output.Ref<0>();

			auto e = add->output.Ref<0>();

			auto f = *smoothOut;//smoothstep->output.Ref<0>();

			auto drawValue = [&](auto v) {
				int xmov = int(10.0f * v) + 10;

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
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			time->globalTimeNow += 1.0f / 24.f;

			*scaleControl -= .01f;
		}


	}
}


int TestParticleBuffer() {
	using namespace weave::particles;

	struct TestFieldA { std::byte value; };
	struct TestFieldB { std::byte value; };
	static_assert(sizeof(TestFieldA) == 1);
	static_assert(sizeof(TestFieldB) == 1);

	ParticleLayout layout;
	layout.particleByteSize = sizeof(TestFieldA) + sizeof(TestFieldB);
	layout.SetOffset<TestFieldA>(0);
	layout.SetOffset<TestFieldB>(sizeof(TestFieldA));

	ParticleBuffer buffer(0, layout);

	// Add 10 particles to the emission buffer
	buffer.AddEmissionParticles(10);

	// Publish the emitted particles to the editable buffer
	buffer.CommitEmittedParticles();

	// Check if the buffer size is correct
	assert(buffer.GetBufferByteSize() == 10 * layout.particleByteSize);

	// Access the editable buffer as a particle_span
	auto editableBuffer = buffer.EditableSpan<TestFieldA, TestFieldB>();

	// Iterate through the particles in the editable buffer and set their values
	size_t index = 0;
	for (auto&& [fieldA, fieldB] : editableBuffer) {
		fieldA.value = static_cast<std::byte>(index);
		fieldB.value = static_cast<std::byte>(index + 1);
		++index;
	}

	// Verify the data in the editable buffer
	index = 0;
	for (auto&& [fieldA, fieldB] : editableBuffer) {
		assert(std::to_integer<int>(fieldA.value) == static_cast<int>(index));
		assert(std::to_integer<int>(fieldB.value) == static_cast<int>(index + 1));
		++index;
	}

	// Kill 3 particles starting from the 2nd particle
	buffer.KillParticles(1, 3);

	// Verify the data in the editable buffer after killing particles
	auto newEditableBuffer = buffer.EditableSpan<TestFieldA, TestFieldB>();
	assert(newEditableBuffer.size() == 7);

	// Verify offsetted span access using explicit offsets
	auto offsets = buffer.GetLayout().GetOffsets<TestFieldA, TestFieldB>();
	auto skippedEditable = buffer.EditableSpan<TestFieldA, TestFieldB>(1, offsets);
	assert(skippedEditable.size() == newEditableBuffer.size() - 1);
	auto firstSkipped = skippedEditable[0];
	auto referenceTuple = newEditableBuffer[1];
	assert(std::to_integer<int>(std::get<0>(firstSkipped).value) ==
		std::to_integer<int>(std::get<0>(referenceTuple).value));
	assert(std::to_integer<int>(std::get<1>(firstSkipped).value) ==
		std::to_integer<int>(std::get<1>(referenceTuple).value));

	// Emit two more particles using the variadic AddEmissionParticles helper
	auto emissionView = buffer.AddEmissionParticles<TestFieldA, TestFieldB>(2);
	assert(emissionView.size() == 2);
	int emissionIndex = 0;
	for (auto&& [fieldA, fieldB] : emissionView) {
		fieldA.value = static_cast<std::byte>(10 + emissionIndex);
		fieldB.value = static_cast<std::byte>(20 + emissionIndex);
		++emissionIndex;
	}

	// Ensure EmissionSpan reports the pending particles
	auto pendingEmission = buffer.EmissionSpan<TestFieldA, TestFieldB>();
	assert(pendingEmission.size() == 2);

	// Commit and verify editable particles now include the emitted ones
	buffer.CommitEmittedParticles();
	auto committedEditable = buffer.EditableSpan<TestFieldA, TestFieldB>();
	assert(committedEditable.size() == 9);
	assert(std::to_integer<int>(std::get<0>(committedEditable[8]).value) == 10);
	assert(std::to_integer<int>(std::get<1>(committedEditable[8]).value) == 20);

	// Active span exposes a const view over the editable bytes
	auto activeSpan = buffer.ActiveSpan();
	assert(activeSpan.size() == committedEditable.size());
	auto [firstByte] = activeSpan[0];
	assert(firstByte == std::get<0>(committedEditable[0]).value);

	
	std::cout << "All tests passed!" << std::endl;
	return 0;
}


class MyInputLogger : public weave::input::InputProcessor {
private:

public:
	void InputEvent(weave::input::InputEventData inputData, weave::input::InputStateMap& inputState) override {

		auto data = inputState.QueryKeyData(inputData.device, inputData.key);
		if (data.state.has_value() && data.state.value().current == data.state.value().previous) {
			return NextProcessor(inputData, inputState);
		}

		std::cout << std::format("\n{}: ", weave::input::VirtualDeviceName(inputData.device));

		if (inputData.HasVirtualKeyState()) {
			std::cout << std::format("{} - ({})", 
				weave::input::VirtualKeyName(inputData.key), 
				weave::input::VirtualKeyStateName(inputData.GetVirtualKeyState()),
				weave::input::VirtualKeyStateName(data.state.value().previous));
		}

		else if (inputData.HasCursorPositionAndDelta()) {
			auto [position, delta] = inputData.GetCursorPositionAndDelta();
			std::cout << std::format("({}, {}) ^ ({}, {})", position.x, position.y, position.z, delta.x, delta.y, delta.z);
		}

		else if (inputData.HasCursorPosition()) {
			auto position = inputData.GetCursorPosition();
			std::cout << std::format("{}, {}, {}", position.x, position.y, position.z);
		}
		else if (inputData.HasCursorDelta()) {
			auto delta = inputData.GetCursorDelta();
			std::cout << std::format("{}, {}, {}", delta.x, delta.y, delta.z);

		}
		else if (inputData.HasMidiNote()) {
			auto event = inputData.GetMidiNote();
			std::cout << std::format("Note: {}, {}, {}, {}", event.note, event.channel, event.velocity, event.pressed);
		}
		else if (inputData.HasMidiControl()) {
			auto event = inputData.GetMidiControl();
			std::cout << std::format("Ctrl: {}, {}, {}", event.control, event.channel, event.value);
		}
		else if (inputData.HasMidiPitchBend()) {
			auto event = inputData.GetMidiPitchBend();
			std::cout << std::format("PitchBend: {}, {}", event.channel, event.value);
		}
		else if (inputData.HasMidiProgram()) {
			auto event = inputData.GetMidiProgram();
			std::cout << std::format("Program: {}, {}", event.channel, event.program);
		}

		else if (inputData.HasDeviceState()) {
			std::cout << " -> " << weave::input::DeviceStateName(inputData.GetDeviceState());
		}

		NextProcessor(inputData, inputState);
	}
};
class MyInputEffect : public weave::input::InputProcessor {
	InputEffectSharedState& sharedState;

	void UpdateBar(uint8_t note, uint8_t velocity, bool pressed) {
		auto index = static_cast<size_t>(note) % InputEffectSharedState::kBarCount;
		float normalizedVelocity = static_cast<float>(velocity) / 127.0f;
		std::scoped_lock lock(sharedState.barsMutex);
		auto& bar = sharedState.bars[index];
		bar.note = note;
		bar.active = pressed;
		bar.targetHeight = pressed ? normalizedVelocity : 0.0f;
		if (note < sharedState.noteActive.size()) {
			sharedState.noteActive[note] = pressed;
		}
		DetectChordsLocked();
	}

	void HighlightNoteLocked(uint8_t note) {
		if (InputEffectSharedState::kBarCount == 0) {
			return;
		}
		size_t index = static_cast<size_t>(note) % InputEffectSharedState::kBarCount;
		sharedState.bars[index].highlight = 1.0f;
	}

	void DetectChordsLocked() {
		auto const limit = sharedState.noteActive.size();
		for (size_t i = 0; i < limit; ++i) {
			if (!sharedState.noteActive[i]) {
				continue;
			}
			if (i + 4 < limit && i + 7 < limit) {
				if (sharedState.noteActive[i + 4] && sharedState.noteActive[i + 7]) {
					HighlightNoteLocked(static_cast<uint8_t>(i));
					HighlightNoteLocked(static_cast<uint8_t>(i + 4));
					HighlightNoteLocked(static_cast<uint8_t>(i + 7));
				}
			}
			if (i + 3 < limit && i + 7 < limit) {
				if (sharedState.noteActive[i + 3] && sharedState.noteActive[i + 7]) {
					HighlightNoteLocked(static_cast<uint8_t>(i));
					HighlightNoteLocked(static_cast<uint8_t>(i + 3));
					HighlightNoteLocked(static_cast<uint8_t>(i + 7));
				}
			}
		}
	}

public:
	explicit MyInputEffect(InputEffectSharedState& state)
		: sharedState(state) {}

	void InputEvent(weave::input::InputEventData inputData, weave::input::InputStateMap& inputState) override {
		if (inputData.HasMidiNote()) {
			auto event = inputData.GetMidiNote();
			UpdateBar(event.note, event.velocity, event.pressed);
		}

		NextProcessor(inputData, inputState);
	}
};


static void RunRenderLoop(const std::function<void()>& swapBuffers,
                          std::atomic<bool>& exitRequested,
                          uint32_t windowWidth,
                          uint32_t windowHeight)
{
    weave::opengl::Context context;

    weave::opengl::SetVSync(true);

    weave::opengl::BindDebugCallback([](std::string msg) {
        std::cout << msg;
        std::cout << weave::opengl::GetErrorString(gl::GetError());
    });
    weave::opengl::EnableDebugMessage(true);

    weave::opengl::Framebuffer defaultFb;
    defaultFb.SetSize(windowWidth, windowHeight);

    std::filesystem::path loadedTexturePath;
    auto loadSceneTexture = [&loadedTexturePath]() -> weave::opengl::Texture {
        const std::vector<std::filesystem::path> candidates = {
#ifdef _WIN32
            std::filesystem::path(R"(X:/Docs/ASMR/Kittyklaw/Blue_bodysuit/A.jpg)"),
#endif
            std::filesystem::path("assets/textures/cat.png")
        };

        for (const auto& candidate : candidates) {
            if (candidate.empty()) {
                continue;
            }

            std::filesystem::path path = candidate;
            if (!path.is_absolute()) {
                path = std::filesystem::current_path() / path;
            }

            std::error_code ec;
            if (!std::filesystem::exists(path, ec) || ec) {
                continue;
            }

            auto image = weave::graphics::ImageLoader::Load(path);
            if (!image.IsValid()) {
                continue;
            }

            auto texture = weave::opengl::TextureLoader::LoadImage2D(image);
            if (texture.GLId() != 0) {
                loadedTexturePath = path;
                return texture;
            }
        }

        return {};
    };

    weave::opengl::Texture tex = loadSceneTexture();
    if (tex.GLId() == 0) {
        GLuint texId = 0;
        gl::CreateTextures(gl::TEXTURE_2D, 1, &texId);
        gl::TextureStorage2D(texId, 1, gl::RGBA8, 1, 1);
        uint8_t pixel[4] = { 255, 255, 255, 255 };
        gl::TextureSubImage2D(texId, 0, 0, 0, 1, 1, gl::RGBA, gl::UNSIGNED_BYTE, pixel);
        tex.WrapGLTexture(gl::TEXTURE_2D, texId);
        tex.SetSamplerDefaults();
        std::cout << "Falling back to generated 1x1 texture.\n";
    } else {
        std::cout << std::format("Loaded texture: {}\n", loadedTexturePath.string());
    }

    weave::opengl::Framebuffer fb;
    fb.AttachColorTarget(tex.GLTargetAndId());
    fb.AttachDepthBuffer(true);
    fb.CheckStatus(true);

    weave::opengl::Program program;
    {
        weave::opengl::Shader vertexShader =
            weave::opengl::ShaderLoader::BuildSource(gl::VERTEX_SHADER, std::filesystem::path("vertex.glsl"));
        weave::opengl::Shader fragShader =
            weave::opengl::ShaderLoader::BuildSource(gl::FRAGMENT_SHADER, std::filesystem::path("fragment.glsl"));

        {
            auto [ok, log] = vertexShader.CompilerLog();
            std::cout << '\n' << log;
        }
        {
            auto [ok, log] = fragShader.CompilerLog();
            std::cout << '\n' << log;
        }

        program.LinkProgram({ vertexShader.GLId(), fragShader.GLId() });
        {
            auto [ok, log] = program.CompilerLog();
            std::cout << '\n' << log;
        }
    }

    weave::opengl::Program programSpv;
    {
        weave::opengl::Shader vertexSpv =
            weave::opengl::ShaderLoader::BuildSpirv(gl::VERTEX_SHADER, std::filesystem::path("vertex.spv"));
        weave::opengl::Shader fragSpv =
            weave::opengl::ShaderLoader::BuildSpirv(gl::FRAGMENT_SHADER, std::filesystem::path("fragment.spv"));

        {
            auto [ok, log] = vertexSpv.CompilerLog();
            std::cout << '\n' << log;
        }
        {
            auto [ok, log] = fragSpv.CompilerLog();
            std::cout << '\n' << log;
        }

        programSpv.LinkProgram({ vertexSpv.GLId(), fragSpv.GLId() });
        {
            auto [ok, log] = programSpv.CompilerLog();
            std::cout << '\n' << log;
        }
    }

    weave::opengl::Program programV, programF;
    weave::opengl::ProgramPipeline pipeline;
    {
        weave::opengl::Shader vertexShader =
            weave::opengl::ShaderLoader::BuildSource(gl::VERTEX_SHADER, std::filesystem::path("vertex_prog.glsl"));
        weave::opengl::Shader fragShader =
            weave::opengl::ShaderLoader::BuildSource(gl::FRAGMENT_SHADER, std::filesystem::path("fragment_prog.glsl"));

        {
            auto [ok, log] = vertexShader.CompilerLog();
            std::cout << '\n' << log;
        }
        {
            auto [ok, log] = fragShader.CompilerLog();
            std::cout << '\n' << log;
        }

        programV.LinkShaderProgram(vertexShader.GLId());
        {
            auto [ok, log] = programV.CompilerLog();
            std::cout << '\n' << log;
        }

        programF.LinkShaderProgram(fragShader.GLId());
        {
            auto [ok, log] = programF.CompilerLog();
            std::cout << '\n' << log;
        }
    }

    pipeline.Create();

    weave::opengl::Program midiBarProgram;
    {
        static constexpr char const* barVertexSrc = R"(
#version 450 core
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
out vec3 vColor;
void main() {
    vColor = inColor;
    gl_Position = vec4(inPosition, 0.0, 1.0);
}
)";
        static constexpr char const* barFragmentSrc = R"(
#version 450 core
in vec3 vColor;
layout(location = 0) out vec4 outColor;
void main() {
    outColor = vec4(vColor, 1.0);
}
)";
        auto barVertexShader = weave::opengl::ShaderLoader::BuildSource(gl::VERTEX_SHADER, barVertexSrc);
        auto barFragmentShader = weave::opengl::ShaderLoader::BuildSource(gl::FRAGMENT_SHADER, barFragmentSrc);
        midiBarProgram.LinkProgram({ barVertexShader.GLId(), barFragmentShader.GLId() });
    }

    GLuint barVao = 0;
    GLuint barVbo = 0;
    gl::CreateVertexArrays(1, &barVao);
    gl::CreateBuffers(1, &barVbo);
    constexpr size_t kMaxBarVertices = InputEffectSharedState::kBarCount * 6;
    gl::NamedBufferData(barVbo, kMaxBarVertices * sizeof(BarVertex), nullptr, gl::DYNAMIC_DRAW);
    gl::VertexArrayVertexBuffer(barVao, 0, barVbo, 0, sizeof(BarVertex));
    gl::EnableVertexArrayAttrib(barVao, 0);
    gl::VertexArrayAttribFormat(barVao, 0, 2, gl::FLOAT, false, offsetof(BarVertex, position));
    gl::VertexArrayAttribBinding(barVao, 0, 0);
    gl::EnableVertexArrayAttrib(barVao, 1);
    gl::VertexArrayAttribFormat(barVao, 1, 3, gl::FLOAT, false, offsetof(BarVertex, color));
    gl::VertexArrayAttribBinding(barVao, 1, 0);

    struct Uniforms {
        float color[4] = { 0.1f, 0.5f, 0.2f, 1.0f };
        float color2[4] = { 0.1f, 0.5f, 0.2f, 1.0f };
        float color3[4] = { 0.5f, 0.1f, 0.1f, 1.0f };
        float crap[4] = { 0.2f, 0.3f, 0.4f, 0.5f };
    } uniforms;

    weave::opengl::Buffer unifBuffer;
    weave::opengl::BufferData<MyBuffer> shdBuffer;
    unifBuffer.Create(uniforms);
    shdBuffer.Create();

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    weave::graphics::MeshData meshData =
        weave::graphics::MeshBuilder()
        .NoIndex()
        .AddAttribute(weave::graphics::MeshAttribute::Label::Position, weave::types::DataType::Float_3, sizeof(vertices), vertices)
        .Build();

    weave::opengl::Mesh mesh =
        weave::opengl::MeshUploader::BuildMesh(meshData);

    auto sphereData = weave::graphics::Sphere(0.5f);
    auto sphere = weave::opengl::MeshUploader::BuildMesh(sphereData);

    weave::time::FrameTracker frameTracker;
    weave::time::IntervalTracker interval;
    weave::time::Clock clock1, clock2;
    clock2.SetTickRateHz(10.0);

    TestParticleBuffer();
    TestNodes();

	auto& inputEffectState = GetInputEffectSharedState();
	std::array<float, InputEffectSharedState::kBarCount> barVisualLevels{};
    while (!exitRequested.load()) {
        frameTracker.FrameStart();
        auto delta = interval.Snapshot();
        clock1.Increment(delta);
        clock2.Increment(delta);
		const float deltaSeconds = std::chrono::duration<float>(delta).count();

        if (clock1.tickDelta) {
            std::cout << std::format("\nFPS: {} - Jitter: {}", 1.0 / frameTracker.avgFrameTime.count(), frameTracker.avgJitter.count());
            std::cout << '\n' << clock1.CurrentTime<int, std::chrono::milliseconds>() << '\n';
        }

        defaultFb.Bind();
        gl::ClearColor(0.02f, 0.02f, 0.03f, 1.0f);
        gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

        float ar = fb.Width() / (fb.Height() * 1.0f);
        gl::BlitNamedFramebuffer(fb.GLId(), 0, 0, fb.Height(), fb.Width(), 0, 0, 0, static_cast<uint32_t>(windowHeight * ar), windowHeight, gl::COLOR_BUFFER_BIT, gl::LINEAR);

        pipeline.Use();
        programV.UseStages(pipeline.GLId());
        programF.UseStages(pipeline.GLId());

        unifBuffer.BindToUniforms(5);
        shdBuffer.BindToShader(3);
        shdBuffer.data.color[0] = 0.1f;
        shdBuffer.data.color[1] = 0.2f;
        shdBuffer.UpdateBuffer();

        tex.Bind(0);

        mesh.RenderInstances(1);
        sphere.RenderInstances(1);

		std::array<InputEffectSharedState::MidiBar, InputEffectSharedState::kBarCount> barsSnapshot;
		{
			std::scoped_lock lock(inputEffectState.barsMutex);
			barsSnapshot = inputEffectState.bars;
			for (auto& bar : inputEffectState.bars) {
				bar.highlight = std::max(0.0f, bar.highlight - deltaSeconds * 3.0f);
			}
		}

		std::vector<BarVertex> barVertices;
		barVertices.reserve(InputEffectSharedState::kBarCount * 6);
		const float spacing = 2.0f / static_cast<float>(InputEffectSharedState::kBarCount);
		const float width = spacing * 0.6f;
		const float baseY = -0.85f;
		const float heightScale = 1.7f;
		const float riseSpeed = 8.0f;
		const float fallSpeed = 4.0f;

		for (size_t i = 0; i < barsSnapshot.size(); ++i) {
			float target = std::clamp(barsSnapshot[i].targetHeight, 0.0f, 1.0f);
			float current = barVisualLevels[i];
			if (current < target) {
				current = std::min(target, current + riseSpeed * deltaSeconds);
			} else {
				current = std::max(0.0f, current - fallSpeed * deltaSeconds);
			}
			barVisualLevels[i] = current;
			if (current <= 0.001f) {
				continue;
			}

			float hue = std::clamp(barsSnapshot[i].note / 127.0f, 0.0f, 1.0f) * 0.83f;
			auto barColor = HslToRgb(hue, 0.8f, 0.55f);
			float highlightLevel = std::clamp(barsSnapshot[i].highlight, 0.0f, 1.0f);
			if (highlightLevel > 0.0f) {
				barColor[0] = std::lerp(barColor[0], 1.0f, highlightLevel);
				barColor[1] = std::lerp(barColor[1], 1.0f, highlightLevel);
				barColor[2] = std::lerp(barColor[2], 1.0f, highlightLevel);
			}

			const float left = -1.0f + static_cast<float>(i) * spacing + (spacing - width) * 0.5f;
			const float right = left + width;
			const float top = std::min(0.95f, baseY + current * heightScale);

			auto pushVertex = [&](float x, float y) {
				barVertices.push_back(BarVertex{ { x, y }, { barColor[0], barColor[1], barColor[2] } });
			};

			pushVertex(left, baseY);
			pushVertex(right, baseY);
			pushVertex(right, top);
			pushVertex(left, baseY);
			pushVertex(right, top);
			pushVertex(left, top);
		}

		if (!barVertices.empty()) {
			gl::NamedBufferSubData(barVbo, 0, barVertices.size() * sizeof(BarVertex), barVertices.data());
			midiBarProgram.Use();
			gl::BindVertexArray(barVao);
			gl::Disable(gl::DEPTH_TEST);
			gl::Enable(gl::BLEND);
			gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
			gl::DrawArrays(gl::TRIANGLES, 0, static_cast<GLsizei>(barVertices.size()));
			gl::Disable(gl::BLEND);
			gl::Enable(gl::DEPTH_TEST);
			gl::BindVertexArray(0);
		}

        swapBuffers();
    }
}

bool RunSceneGraphSelfTest() {
	using weave::scenegraph::SceneGraph;

	SceneGraph graph;

	auto rootChildIndex = graph.CreateNode("root_child");
	[[maybe_unused]] auto branchIndex = graph.CreateNode("branch", rootChildIndex);
	auto leafIndex = graph.CreateNode("leaf", "branch");

	if (graph.FindNode("leaf") != leafIndex) {
		return false;
	}

	const bool touchedRootChild = graph.TouchNode(rootChildIndex, [](Transform &transform) {
		transform.Translate(1.0f, 0.0f, 0.0f);
	});
	const bool touchedBranch = graph.TouchNode("branch", [](Transform &transform) {
		transform.Translate(0.0f, 2.0f, 0.0f);
	});
	const bool touchedLeaf = graph.TouchNode(leafIndex, [](Transform &transform) {
		transform.Translate(0.0f, 0.0f, 3.0f);
	});

	if (!(touchedRootChild && touchedBranch && touchedLeaf)) {
		return false;
	}

	graph.UpdateWorldTransforms();

	Vector3 rootChildWorld{};
	Vector3 branchWorld{};
	Vector3 leafWorld{};
	bool rootChildSeen = false;
	bool branchSeen = false;
	bool leafSeen = false;

	graph.Traverse([&](SceneGraph::NodeIndex, SceneGraph::Node const &node) {
		if (node.name.empty()) {
			return;
		}

		Vector3 position(node.worldTransform.W.x, node.worldTransform.W.y, node.worldTransform.W.z);
		if (node.name == "root_child") {
			rootChildWorld = position;
			rootChildSeen = true;
		} else if (node.name == "branch") {
			branchWorld = position;
			branchSeen = true;
		} else if (node.name == "leaf") {
			leafWorld = position;
			leafSeen = true;
		}
	});

	const bool transformsOk = rootChildSeen && branchSeen && leafSeen
		&& equivalent(rootChildWorld, Vector3(1.0f, 0.0f, 0.0f))
		&& equivalent(branchWorld, Vector3(1.0f, 2.0f, 0.0f))
		&& equivalent(leafWorld, Vector3(1.0f, 2.0f, 3.0f));

	const bool removeLeafByIndex = graph.RemoveNode(leafIndex);
	const bool lookupAfterLeafRemoval = graph.FindNode("leaf") == SceneGraph::Node::kInvalidIndex;

	auto reusedLeafIndex = graph.CreateNode("reused_leaf", "branch");
	const bool reusedSlot = reusedLeafIndex == leafIndex;

	const bool removeReusedByName = graph.RemoveNode("reused_leaf");
	const bool removeBranchByName = graph.RemoveNode("branch");
	const bool branchMissing = graph.FindNode("branch") == SceneGraph::Node::kInvalidIndex;
	const bool missingNodeLookup = graph.FindNode("missing") == SceneGraph::Node::kInvalidIndex;

	const bool removeRootFails = !graph.RemoveNode(0);
	const bool touchInvalidIndex = !graph.TouchNode(SceneGraph::Node::kInvalidIndex, [](Transform &transform) {
		transform.Translate(42.0f, 0.0f, 0.0f);
	});
	const bool touchMissingName = !graph.TouchNode("missing_name", [](Transform &transform) {
		transform.Translate(0.0f, 42.0f, 0.0f);
	});

	graph.Reset();
	const bool resetClearsNames = graph.FindNode("root_child") == SceneGraph::Node::kInvalidIndex
		&& graph.FindNode("branch") == SceneGraph::Node::kInvalidIndex;
	const auto postResetIndex = graph.CreateNode("post_reset");
	const bool resetAllowsCreate = postResetIndex != SceneGraph::Node::kInvalidIndex;

	return transformsOk
		&& removeLeafByIndex
		&& lookupAfterLeafRemoval
		&& reusedSlot
		&& removeReusedByName
		&& removeBranchByName
		&& branchMissing
		&& missingNodeLookup
		&& removeRootFails
		&& touchInvalidIndex
		&& touchMissingName
		&& resetClearsNames
		&& resetAllowsCreate;
}

bool RunParticleMachineSelfTest() {
	using namespace weave::particles;

	constexpr uint64_t kParticlesToEmit = 4;
	constexpr float kLifeValue = 0.5f;
	constexpr float kMaxLifeValue = 5.0f;

	ParticleMachine machine;
	ParticleBuffer buffer(0, ParticleLayout::BuildStdParticleLayout());
	machine.AddBuffer(buffer);
	machine.SetSamplingData(1.0f, 1, true);

	auto emitter = machine.Graph().CreateNode<ParticleEmitter>();
	emitter->input.SetDefaultValue<ParticleEmitter::MinEmit>(static_cast<float>(kParticlesToEmit));
	emitter->input.SetDefaultValue<ParticleEmitter::MaxEmit>(static_cast<float>(kParticlesToEmit));
	emitter->input.SetDefaultValue<ParticleEmitter::Rate>(1.0f);
	emitter->input.SetDefaultValue<ParticleEmitter::MinFrequency>(0.0f);
	emitter->input.SetDefaultValue<ParticleEmitter::MaxFrequency>(0.0f);
	emitter->input.SetDefaultValue<ParticleEmitter::MaxRuntime>(-1.0);
	emitter->input.SetDefaultValue<ParticleEmitter::MaxParticles>(kParticlesToEmit);

	auto writer = machine.Graph().CreateNode<ParticleTestWriter>(kLifeValue, kMaxLifeValue);
	auto commit = machine.Graph().CreateNode<particles::CommitEmissionNode>();
	auto inspector = machine.Graph().CreateNode<ParticleTestInspector>(kLifeValue, kMaxLifeValue);

	machine.Graph().AddRootTrigger(emitter);
	emitter->ConnectTrigger(writer);
	writer->ConnectTrigger(commit);
	commit->ConnectTrigger(inspector);

	machine.Execute();

	auto editableSpan = buffer.EditableSpan<particles::layout::LifeTime, particles::layout::MaxLifeTime>();
	const size_t editableCount = editableSpan.size();

	return writer->wroteCount == kParticlesToEmit
		&& inspector->verifiedCount == kParticlesToEmit
		&& inspector->mismatchCount == 0
		&& editableCount == kParticlesToEmit;
}

#ifdef _WIN32
int RunWin32Harness()
{
    constexpr uint32_t windowWidth = 1920;
    constexpr uint32_t windowHeight = 1080;
    std::atomic<bool> exitRequested{false};

    weave::win32::InstallExceptionHandler(0);

    weave::win32::Win32Window window;
    window.Create(L"Weave Strands Test");
    window.Windowed(windowWidth, windowHeight, false, false, true);

    weave::win32::Win32OpenGL platformGL;
    platformGL.InitGL(window.GetHWND());
    auto [glmajor, glminor] = platformGL.GetGLVersion();
    std::cout << std::format("GL Version: {}.{}\n\n", glmajor, glminor);

    SetWindowTextW(window.GetHWND(), std::format(L"Weave Strands Test - GL{}.{}", glmajor, glminor).c_str());

    if (std::filesystem::exists("D:/Dev/Weave")) {
        std::filesystem::current_path("D:/Dev/Weave");
    }
    std::cout << std::format("Working directory: {}\n\n", std::filesystem::current_path().string());

    weave::input::InputPipeline inputPipeline;
    weave::input::Win32KeyboardFeed w32KbFeed;
    weave::input::Win32MouseFeed w32MouseFeed;

    weave::input::RawInputKeyboardFeed rawKbFeed;
    weave::input::RawInputMouseFeed rawMouseFeed;
    weave::input::RawInputGamepad rawGamepadFeed;

    rawGamepadFeed.EnumerateGamepads(window.GetHWND());
    {
        auto list = rawGamepadFeed.GetEnumeratedGamepadInfo();
        std::cout << "Gamepads: " << list.size();
        for (auto const& info : list) {
            std::cout << "\n\t" << info.hardwareId.uniqueId;
            std::cout << "\n\t" << info.deviceName;
            std::cout << "\n\t" << info.osDeviceName;
            std::cout << "\n\t" << info.osProductName;
            std::cout << "\n\t" << weave::input::VirtualDeviceName(info.virtualDevice);
            std::cout << '\n';
        }
    }

    rawMouseFeed.EnumerateMice(window.GetHWND());
    {
        auto list = rawMouseFeed.GetEnumeratedMiceInfo();
        std::cout << "\nMice: " << list.size();
        for (auto const& info : list) {
            std::cout << "\n\t" << info.hardwareId.uniqueId;
            std::cout << "\n\t" << info.deviceName;
            std::cout << "\n\t" << info.osDeviceName;
            std::cout << "\n\t" << info.osProductName;
            std::cout << "\n\t" << weave::input::VirtualDeviceName(info.virtualDevice);
            std::cout << '\n';
        }
    }

    rawKbFeed.EnumerateKeyboards(window.GetHWND());
    {
        auto list = rawKbFeed.GetEnumeratedKeyboardInfo();
        std::cout << "\nKeyboards: " << list.size();
        for (auto const& info : list) {
            std::cout << "\n\t" << info.hardwareId.uniqueId;
            std::cout << "\n\t" << info.deviceName;
            std::cout << "\n\t" << info.osDeviceName;
            std::cout << "\n\t" << info.osProductName;
            std::cout << "\n\t" << weave::input::VirtualDeviceName(info.virtualDevice);
            std::cout << '\n';
        }
    }

    {
        auto processor = std::make_shared<MyInputLogger>();
        processor->SetPriority(0);
        inputPipeline.AddProcessor(processor);
    }

    {
        auto processor = std::make_shared<MyInputEffect>(GetInputEffectSharedState());
        processor->SetPriority(1);
        inputPipeline.AddProcessor(processor);
    }

    {
        auto processor = std::make_shared<weave::input::InputStateWriter>();
        processor->SetPriority(2);
        inputPipeline.AddProcessor(processor);
    }

    {
        auto processor = std::make_shared<weave::input::InputRouter>();
        processor->SetPriority(4);
        processor->AddRoute({ weave::input::VirtualDevice::Mouse1, {} }, { weave::input::VirtualDevice::Mouse3, {} });
        processor->AddRoute({ weave::input::VirtualDevice::Mouse1, {} }, { weave::input::VirtualDevice::Mouse4, {} });
        processor->AddRoute({ weave::input::VirtualDevice::Mouse1, weave::input::VirtualKey::Cursor }, { weave::input::VirtualDevice::Mouse11, {} });
        inputPipeline.AddProcessor(processor);
    }

    {
        auto processor = std::make_shared<weave::input::SingleDevice>();
        processor->SetPriority(10);
        inputPipeline.AddProcessor(processor);
    }

    {
        auto processor = std::make_shared<weave::input::InputEventContext>();
        processor->SetPriority(0);

        weave::input::InputEventContextRule rule;

        weave::input::InputEventContextCondition condition;
        condition.device = weave::input::VirtualDevice::Mouse;
        condition.key = weave::input::VirtualKey::Click_Left;
        condition.states.down = true;
        condition.states.hold = true;
        condition.prevStates.any = true;
        condition.isTrigger = true;
        rule.eventId = 0;
        rule.conditions.emplace_back(condition);

        condition.device = weave::input::VirtualDevice::Keyboard;
        condition.key = weave::input::VirtualKey::Control;
        condition.isTrigger = false;
        rule.eventId = 1;
        rule.conditions.emplace_back(condition);

        processor->AddRule(rule);

        condition.device = weave::input::VirtualDevice::Keyboard;
        condition.key = weave::input::VirtualKey::Alt;
        condition.isTrigger = false;
        rule.eventId = 2;
        rule.feedbackRequests.emplace_back(std::pair{ weave::input::VirtualDevice::Mouse, weave::input::VirtualKey::Cursor });
        rule.conditions.emplace_back(condition);
        rule.forwardMode = weave::input::InputEventForwardMode::Sink;

        processor->AddRule(rule);

        inputPipeline.AddProcessor(processor);
    }

    std::function<void()> swapBuffers = [&]() { SwapBuffers(platformGL.GetHDC()); };
    std::function<void()> messagePump = [&]() { window.MessagePump(); };

    window.SetWinProc([&](auto hwnd, auto msg, auto wparam, auto lparam) {
        (void)hwnd;

        rawMouseFeed.FeedSyncInput(msg, wparam, lparam, inputPipeline);
        rawKbFeed.FeedSyncInput(msg, wparam, lparam, inputPipeline);
        rawGamepadFeed.FeedSyncInput(msg, wparam, lparam, inputPipeline);

        auto& inputState = inputPipeline.GetInputState();
        auto messages = inputState.FlushMessages();
        for (auto const& msgEvent : messages) {
            std::cout << std::format("\nMessage: {} [{} packages]", msgEvent.messageId, msgEvent.report.size());
            for (auto const& keyData : msgEvent.report) {
                auto cursor = keyData.cursor.value_or(weave::input::CursorPayload{});
                std::cout << std::format(" [{} : {} : ({},{},{})]", weave::input::VirtualDeviceName(keyData.device), weave::input::VirtualKeyName(keyData.key), cursor.position.x, cursor.position.y, cursor.position.z);
            }
        }

        switch (msg)
        {
        case WM_QUIT:
        case WM_CLOSE:
        case WM_DESTROY:
            exitRequested.store(true);
            return FALSE;
        default:
            return FALSE;
        }
    });

    weave::opengl::ContextPool::ReleaseContext();

    std::jthread renderThread([&] {
        RunRenderLoop(swapBuffers, exitRequested, windowWidth, windowHeight);
    });

    messagePump();
    exitRequested.store(true);
    return 0;
}
#else
int RunWaylandHarness()
{
    constexpr uint32_t windowWidth = 1920;
    constexpr uint32_t windowHeight = 1080;
    std::atomic<bool> exitRequested{false};

    weave::wayland::WaylandWindow window;
    try {
        window.Create();
        window.Windowed(windowWidth, windowHeight, false, false, true);
    } catch (const std::exception& e) {
        std::cerr << "Wayland init failed: " << e.what() << '\n';
        return -1;
    }

    weave::wayland::WaylandOpenGL platformGL;
    platformGL.InitGL(window);
    auto [glmajor, glminor] = platformGL.GetGLVersion();
    window.SetTitle(std::format("Weave Strands Test - GL{}.{}", glmajor, glminor));
    std::cout << std::format("GL Version: {}.{}\n\n", glmajor, glminor);
    std::cout << std::format("Working directory: {}\n\n", std::filesystem::current_path().string());

    weave::input::InputPipeline inputPipeline;
    weave::input::wayland::WaylandGamepadFeed gamepadFeed;
	weave::input::wayland::WaylandMidiFeed midiFeed;
	
    {
        auto processor = std::make_shared<MyInputLogger>();
        processor->SetPriority(0);
        inputPipeline.AddProcessor(processor);
    }

    {
        auto processor = std::make_shared<MyInputEffect>(GetInputEffectSharedState());
        processor->SetPriority(1);
        inputPipeline.AddProcessor(processor);
    }

    {
        auto processor = std::make_shared<weave::input::InputStateWriter>();
        processor->SetPriority(2);
        inputPipeline.AddProcessor(processor);
    }

    if (gamepadFeed.EnumerateGamepads(inputPipeline) == 0) {
        std::cout << "No Gamepad devices found.\n";
    }
    
    if (auto info = gamepadFeed.GetEnumeratedGamepadInfo(); !info.empty()) {
        std::cout << "\nGamepads: " << info.size();
        for (auto const& pad : info) {
            std::cout << std::format(
                "\n\tNode: {}\n\tName: {}\n\tVendor: 0x{:04X}\n\tProduct: 0x{:04X}\n\tVirtual: {}\n\tLayout: {}",
                pad.id.deviceNode,
                pad.name.empty() ? "[Unnamed]" : pad.name,
                pad.vendorId,
                pad.productId,
                weave::input::VirtualDeviceName(pad.virtualDevice),
                weave::input::gamepad::LayoutName(pad.currentLayout));
        }
        std::cout << '\n';
    }


	if(midiFeed.EnumerateMidiSources(true, inputPipeline) == 0) {
		std::cout << "No MIDI devices found\n";
	}

    if (auto midiInfo = midiFeed.GetAllMidiSources(); !midiInfo.empty()) {
        std::cout << "\nMidi: " << midiInfo.size();
        for (auto const& midi : midiInfo) {
            std::cout << std::format(
                "\n\tName: {}\n\tHardware: {}\n\tClient: {}\n\tPort: {}",
                midi.name.empty() ? "[Unnamed]" : midi.name,
                midi.isHardware,
                midi.client,
				midi.port);
        }
        std::cout << '\n';
    }

    weave::input::wayland::WaylandSeat seat;
    seat.InitializeSeat(window.GetDisplay(), window.GetSurface(), inputPipeline);

    std::function<void()> swapBuffers = [&]() { platformGL.SwapBuffers(); };
    std::function<void()> messagePump = [&]() {
        window.MessagePeekPump([&] {
            if (window.ShouldClose()) {
                exitRequested.store(true);
                window.StopMessagePump();
            }

            auto& state = inputPipeline.GetInputState();
            auto messages = state.FlushMessages();
            for (auto const& msg : messages) {
                std::cout << std::format("\nMessage: {} [{} packages]", msg.messageId, msg.report.size());
                for (auto const& keyData : msg.report) {
                    auto cursor = keyData.cursor.value_or(weave::input::CursorPayload{});
                    std::cout << std::format(" [{} : {} : ({},{},{})]",
                                             weave::input::VirtualDeviceName(keyData.device),
                                             weave::input::VirtualKeyName(keyData.key),
                                             cursor.position.x,
                                             cursor.position.y,
                                             cursor.position.z);
                }
            }
        });
    };

    weave::opengl::ContextPool::ReleaseContext();

	std::jthread inputThread([&] {
		while(!exitRequested.load()) {
        	gamepadFeed.PollInput(std::chrono::milliseconds(50));
		}
    });

	std::jthread midiThread([&] {
		while(!exitRequested.load()) {
        	midiFeed.PollInput(std::chrono::milliseconds(50));
		}
    });

    std::jthread renderThread([&] {
        RunRenderLoop(swapBuffers, exitRequested, windowWidth, windowHeight);
    });

    messagePump();
    exitRequested.store(true);
    return 0;
}
#endif

int main()
{
	if (!RunSceneGraphSelfTest()) {
		std::cerr << "Scene graph self-test failed.\n";
		return -1;
	}

	if (!RunParticleMachineSelfTest()) {
		std::cerr << "Particle machine self-test failed.\n";
		return -1;
	}
#ifdef _WIN32
    return RunWin32Harness();
#else
    return RunWaylandHarness();
#endif
}
