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
#include <cstdint>
#include <atomic>
#include <filesystem>
#include <system_error>
#include <array>
#include <algorithm>

#include "weave/system/blender/Blender.h"
#include "weave/system/blender/ArithmeticNodes.h"
#include "weave/animation/blender/samplers/DataClipSampler.h"
#include "weave/animation/blender/samplers/SignalSampler.h"

#include "weave/particles/ParticleBuffer.h"


#include "weave/system/math/Interpolation.h"
/*
Port Anim to Blenders
Port particles to Blenders
*/

using namespace weave;
using namespace weave::blender;

namespace {

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
	std::atomic<float> hue{0.58f};
	std::atomic<float> pulse{0.0f};
	std::atomic<float> sparkle{0.0f};
	std::atomic<bool> rainbow{false};
};

InputEffectSharedState& GetInputEffectSharedState() {
	static InputEffectSharedState state;
	return state;
}

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

	// Create a ParticleBuffer with an initial particle size of 2 bytes
	ParticleBuffer buffer(0, 2);

	// Add 10 particles to the emission buffer
	buffer.AddEmissionParticles(10);

	// Publish the emitted particles to the editable buffer
	buffer.PublishEmittedParticles();

	// Check if the buffer size is correct
	assert(buffer.GetBufferByteSize() == 20);

	// Access the editable buffer as a particle_span
	auto editableBuffer = buffer.EditableBuffer();

	// Iterate through the particles in the editable buffer and set their values
	for (size_t i = 0; i < editableBuffer.size(); ++i) {
		std::byte& byte1 = editableBuffer[i];
		byte1 = static_cast<std::byte>(i);

		std::byte& byte2 = editableBuffer.CastOffset<std::byte>(&byte1, 1);
		byte2 = static_cast<std::byte>(i + 1);
	}

	// Verify the data in the editable buffer
	for (size_t i = 0; i < editableBuffer.size(); ++i) {
		assert(static_cast<int>(editableBuffer[i]) == i);
		assert(static_cast<int>(editableBuffer.CastOffset<std::byte>(&editableBuffer[i], 1)) == i + 1);
	}

	// Kill 3 particles starting from the 2nd particle
	buffer.KillParticles(1, 3);

	// Verify the data in the editable buffer after killing particles
	auto newEditableBuffer = buffer.EditableBuffer();
	assert(newEditableBuffer.size() == 7);

	
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

		else if (inputData.HasDeviceState()) {
			std::cout << " -> " << weave::input::DeviceStateName(inputData.GetDeviceState());
		}

		NextProcessor(inputData, inputState);
	}
};
class MyInputEffect : public weave::input::InputProcessor {

private:
	InputEffectSharedState& sharedState;
	std::array<weave::input::VirtualKey, 10> konami{
		weave::input::VirtualKey::Up,
		weave::input::VirtualKey::Up,
		weave::input::VirtualKey::Down,
		weave::input::VirtualKey::Down,
		weave::input::VirtualKey::Left,
		weave::input::VirtualKey::Right,
		weave::input::VirtualKey::Left,
		weave::input::VirtualKey::Right,
		weave::input::VirtualKey::B,
		weave::input::VirtualKey::A
	};
	std::size_t konamiIndex = 0;

	void HandleKeyboard(weave::input::VirtualKey key) {
		AddPulse(0.12f);
		if (key >= weave::input::VirtualKey::A && key <= weave::input::VirtualKey::Z) {
			const float delta = 0.004f * (1.0f + static_cast<float>(static_cast<int>(key) - static_cast<int>(weave::input::VirtualKey::A)));
			NudgeHue(delta);
		}

		if (key == weave::input::VirtualKey::Space) {
			bool current = sharedState.rainbow.load(std::memory_order_relaxed);
			sharedState.rainbow.store(!current, std::memory_order_relaxed);
		} else if (key == weave::input::VirtualKey::Esc) {
			sharedState.rainbow.store(false, std::memory_order_relaxed);
		}

		if (key == weave::input::VirtualKey::Vary5 || key == weave::input::VirtualKey::Plus) {
			BoostSparkle(0.5f);
		}

		UpdateKonami(key);
	}

	void AddPulse(float amount) {
		float current = sharedState.pulse.load(std::memory_order_relaxed);
		while (true) {
			float next = std::min(1.5f, current + amount);
			if (sharedState.pulse.compare_exchange_weak(current, next, std::memory_order_relaxed, std::memory_order_relaxed)) {
				break;
			}
		}
	}

	void BoostSparkle(float amount) {
		float current = sharedState.sparkle.load(std::memory_order_relaxed);
		while (true) {
			float next = std::clamp(current + amount, 0.0f, 1.5f);
			if (sharedState.sparkle.compare_exchange_weak(current, next, std::memory_order_relaxed, std::memory_order_relaxed)) {
				break;
			}
		}
	}

	void NudgeHue(float amount) {
		float current = sharedState.hue.load(std::memory_order_relaxed);
		while (true) {
			float next = std::fmod(current + amount, 1.0f);
			if (next < 0.0f) {
				next += 1.0f;
			}
			if (sharedState.hue.compare_exchange_weak(current, next, std::memory_order_relaxed, std::memory_order_relaxed)) {
				break;
			}
		}
	}

	void UpdateKonami(weave::input::VirtualKey key) {
		if (konamiIndex >= konami.size()) {
			konamiIndex = 0;
		}

		if (key == konami[konamiIndex]) {
			++konamiIndex;
			if (konamiIndex == konami.size()) {
				TriggerKonami();
				konamiIndex = 0;
			}
		} else {
			konamiIndex = (key == konami.front()) ? 1u : 0u;
		}
	}

	void TriggerKonami() {
		sharedState.rainbow.store(true, std::memory_order_relaxed);
		sharedState.pulse.store(1.5f, std::memory_order_relaxed);
		sharedState.sparkle.store(1.5f, std::memory_order_relaxed);
		std::cout << "\nKonami code unlocked rainbow mode! (Press ESC to settle down)";
	}

public:
	explicit MyInputEffect(InputEffectSharedState& state)
		: sharedState(state) {}

	void InputEvent(weave::input::InputEventData inputData, weave::input::InputStateMap& inputState) override {
		if (inputData.HasVirtualKeyState()) {
			if (inputData.device == weave::input::VirtualDevice::Keyboard &&
				inputData.GetVirtualKeyState() == weave::input::VirtualKeyState::Down) {
				HandleKeyboard(inputData.key);
			} else if (inputData.device == weave::input::VirtualDevice::Mouse &&
				inputData.GetVirtualKeyState() == weave::input::VirtualKeyState::Down) {
				BoostSparkle(0.35f);
			}
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

    struct Uniforms {
        float color[4] = { 0.1f, 0.5f, 0.2f, 1.0f };
        float color2[4] = { 0.1f, 0.5f, 0.2f, 1.0f };
        float color3[4] = { 0.5f, 0.1f, 0.1f, 1.0f };
        float crap[4] = { 0.2f, 0.3f, 0.4f, 0.5f };
    } uniforms;

    struct MyBuffer {
        float color[4] = { 0.1f, -0.5f, 0.0f, 1.0f };
        float crap[4] = { 0.2f, 0.3f, 0.4f, 0.5f };
        float x = 1.0f;
    };

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
	auto dampValue = [](std::atomic<float>& value, float dt, float decayRate) {
		float current = value.load(std::memory_order_relaxed);
		if (current <= 0.0f) {
			return 0.0f;
		}
		float next = std::max(0.0f, current - decayRate * dt);
		if (next != current) {
			value.store(next, std::memory_order_relaxed);
		}
		return next;
	};
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

		float baseHue = inputEffectState.hue.load(std::memory_order_relaxed);
		if (inputEffectState.rainbow.load(std::memory_order_relaxed)) {
			baseHue += deltaSeconds * 0.1f;
			if (baseHue > 1.0f) {
				baseHue -= 1.0f;
			}
			inputEffectState.hue.store(baseHue, std::memory_order_relaxed);
		}

		const float pulse = dampValue(inputEffectState.pulse, deltaSeconds, 0.7f);
		const float sparkle = dampValue(inputEffectState.sparkle, deltaSeconds, 1.2f);
		auto rgb = HslToRgb(baseHue,
			std::clamp(0.5f + pulse * 0.4f, 0.0f, 1.0f),
			std::clamp(0.35f + pulse * 0.5f, 0.0f, 1.0f));
		rgb[0] = std::clamp(rgb[0] + sparkle * 0.1f, 0.0f, 1.0f);
		rgb[1] = std::clamp(rgb[1] + sparkle * 0.05f, 0.0f, 1.0f);

        defaultFb.Bind();
        gl::ClearColor(rgb[0], rgb[1], rgb[2], 1.0f);
        gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

        float ar = fb.Width() / (fb.Height() * 1.0f);
        gl::BlitNamedFramebuffer(fb.GLId(), 0, 0, fb.Height(), fb.Width(), 0, 0, 0, static_cast<uint32_t>(windowHeight * ar), windowHeight, gl::COLOR_BUFFER_BIT, gl::LINEAR);

        pipeline.Use();
        programV.UseStages(pipeline.GLId());
        programF.UseStages(pipeline.GLId());

        unifBuffer.BindToUniforms(5);
        shdBuffer.BindToShader(3);
        shdBuffer.data.color[0] += 0.00001f + pulse * 0.0002f;
        shdBuffer.data.color[1] = 0.2f + sparkle * 0.5f;
        shdBuffer.UpdateBuffer();

        tex.Bind(0);

        mesh.RenderInstances(1);
        sphere.RenderInstances(1);

        swapBuffers();
    }
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
                auto cursor = keyData.cursor.value_or(weave::input::CursorData{});
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
	midiFeed.Test();

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

    auto enumeratedPads = gamepadFeed.EnumerateGamepads(&inputPipeline);
    if (enumeratedPads == 0) {
        std::cerr << "Wayland gamepad feed active but no controllers detected (check permissions?).\n";
    }

    auto info = gamepadFeed.GetEnumeratedGamepadInfo();
    if (!info.empty()) {
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
                    auto cursor = keyData.cursor.value_or(weave::input::CursorData{});
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
        	gamepadFeed.PollInput(inputPipeline, std::chrono::milliseconds(50));
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
#ifdef _WIN32
    return RunWin32Harness();
#else
    return RunWaylandHarness();
#endif
}
