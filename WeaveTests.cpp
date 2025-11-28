// WeaveInput.cpp : Defines the entry point for the application.
//

#include "WeaveTests.h"
#include "tests/TestEntryPoints.h"
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
#include <exception>
#include <string_view>

/*
Port Anim to Blenders
*/

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

} // namespace

bool RunAllTests() {
	constexpr auto logFailures = [](std::string_view scope, const weave::tests::TestReport& report) {
		for (auto const& failure : report.failures) {
			std::cerr << "[" << scope << "] " << failure << '\n';
		}
	};

	bool allPassed = true;

	try {
		auto blenderReport = weave::tests::blender::TestNodes();
		if (!blenderReport) {
			logFailures("blender", blenderReport);
			allPassed = false;
		}
	} catch (const std::exception& ex) {
		std::cerr << "[blender] Unhandled exception: " << ex.what() << '\n';
		allPassed = false;
	} catch (...) {
		std::cerr << "[blender] Unhandled unknown exception.\n";
		allPassed = false;
	}

	auto particleBufferReport = weave::tests::particles::TestParticleBuffer();
	if (!particleBufferReport) {
		logFailures("particles.buffer", particleBufferReport);
		allPassed = false;
	}

	auto particleMachineReport = weave::tests::particles::RunParticleMachineSelfTest();
	if (!particleMachineReport) {
		logFailures("particles.machine", particleMachineReport);
		allPassed = false;
	}

	auto sceneGraphReport = weave::tests::scenegraph::RunSceneGraphSelfTest();
	if (!sceneGraphReport) {
		logFailures("scenegraph", sceneGraphReport);
		allPassed = false;
	}

	return allPassed;
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
	if (!RunAllTests()) {
		return -1;
	}
#ifdef _WIN32
    return RunWin32Harness();
#else
    return RunWaylandHarness();
#endif
}
