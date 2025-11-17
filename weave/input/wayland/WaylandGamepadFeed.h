#pragma once

#include <atomic>
#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <linux/input.h>
#include <poll.h>

#include "weave/input/system/InputPipeline.h"
#include "weave/input/system/VirtualDevices.h"
#include "weave/input/system/VirtualKeys.h"
#include "weave/input/system/GamepadLayouts.h"

struct libevdev;
struct udev;
struct udev_monitor;

namespace weave::input::wayland {
using GamepadLayout = weave::input::gamepad::GamepadLayout;

class WaylandGamepadFeed {
public:
	struct GamepadId {
		std::string deviceNode;
		bool operator==(GamepadId const& other) const { return deviceNode == other.deviceNode; }
		bool operator!=(GamepadId const& other) const { return !(*this == other); }
		bool Empty() const { return deviceNode.empty(); }
	};

	struct GamepadInfo {
		GamepadId id;
		std::string name;
		uint16_t vendorId = 0;
		uint16_t productId = 0;
		VirtualDevice virtualDevice = VirtualDevice::None;
		GamepadLayout detectedLayout = GamepadLayout::Generic;
		GamepadLayout currentLayout = GamepadLayout::Generic;
	};

	~WaylandGamepadFeed();

	size_t EnumerateGamepads(InputPipeline& pipeline);

	size_t GetEnumeratedGamepadCount() const;
	std::vector<GamepadInfo> GetEnumeratedGamepadInfo() const;
	std::vector<GamepadId> GetLinkedGamepadIds(VirtualDevice vDev) const;
	GamepadInfo GetGamepadInfo(GamepadId const& gamepadId) const;
	void LinkGamepad(GamepadId const& id, VirtualDevice device);
	void UnlinkGamepad(GamepadId const& id);
	void UnlinkVirtualDevice(VirtualDevice vDev);
	void SetDeadzones(GamepadId const& id, float leftDeadzone, float rightDeadzone);
	void SetLayout(GamepadId const& id, GamepadLayout layout);

	void PollInput(std::chrono::milliseconds blockTimeout);

private:
	struct GamepadDevice {
		GamepadInfo info;
		int fd = -1;
		libevdev* evdev = nullptr;
		float leftX = 0.0f;
		float leftY = 0.0f;
		float rightX = 0.0f;
		float rightY = 0.0f;
		float triggerL = 0.0f;
		float triggerR = 0.0f;
		int hatX = 0;
		int hatY = 0;
		bool dpadUp = false;
		bool dpadDown = false;
		bool dpadLeft = false;
		bool dpadRight = false;
		float leftDeadzone = 0.08f;
		float rightDeadzone = 0.08f;
	};

	std::vector<GamepadDevice> devices;
	mutable std::mutex deviceMutex;
	
	udev* udevCtx = nullptr;
	udev_monitor* monitor = nullptr;
	int monitorFd = -1;
	std::vector<pollfd> pollFds; //Cache for the polling
	InputPipeline *inputPipeline{};
	
	bool InitializeUdev();
	void HandleUdevEvents();
	bool AddDevice(const char* devnode);
	void RemoveDevice(const std::string& devnode);
	GamepadDevice* FindDeviceById(GamepadId const& id);

	void ProcessDevice(GamepadDevice& device);
	void ProcessEvent(GamepadDevice& device, const input_event& ev);
	void ProcessAbsEvent(GamepadDevice& device, const input_event& ev);
	void ProcessKeyEvent(GamepadDevice& device, const input_event& ev);

	void UpdateHat(GamepadDevice& device, bool horizontal, int value);

	static float NormalizeStick(const input_absinfo& info, int value);
	static float NormalizeTrigger(const input_absinfo& info, int value);
	static float TriggerValueDeadzone(const GamepadDevice& device, const input_absinfo& info, const input_event& ev);
	static int ButtonIndexFromCode(unsigned code);
	static VirtualKey MapEvdevButton(const GamepadDevice& device, unsigned code);
	static VirtualKey MapLayoutButton(const GamepadDevice& device, int buttonIndex);
	static GamepadLayout DetectLayout(GamepadInfo const& info);
};

} // namespace weave::input::wayland
