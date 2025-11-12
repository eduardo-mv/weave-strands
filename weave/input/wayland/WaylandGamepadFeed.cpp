#include "WaylandGamepadFeed.h"

#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cctype>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string_view>

#include <libudev.h>
#include <libevdev/libevdev.h>

#include "weave/input/system/GamepadLayouts.h"

namespace weave::input::wayland {

/*
 * Linux gamepad stack overview:
 *   1. EnumerateGamepads() boots a udev context (udev_new) plus a netlink monitor
 *      (udev_monitor_new_from_netlink + udev_monitor_enable_receiving) so we receive
 *      hotplug notifications straight from the kernel input subsystem.
 *   2. Run() is the worker thread. It poll()s a single fd set that mixes the udev
 *      monitor socket with every opened /dev/input/js* file descriptor, so a wakeup
 *      tells us whether we have a hotplug event or pending controller input without
 *      spinning.
 *   3. AddDevice() opens the device node with open(O_NONBLOCK), hands the fd to
 *      libevdev_new_from_fd so libevdev will issue the required ioctl() capability
 *      probes, and configures per-device deadzones/layouts before the fd enters the
 *      poll loop.
 *   4. ProcessDevice() repeatedly calls libevdev_next_event (which internally reads
 *      from the fd via read() and handles SYN_DROPPED recovery) and forwards the
 *      translated events to the input pipeline.
 */

namespace {
using weave::input::gamepad::kStandardLayouts;

constexpr float kDefaultStickDeadzone = 0.08f;

VirtualKeyState ToVirtualKeyState(int value) {
	switch (value) {
	case 0: return VirtualKeyState::Up;
	case 1: return VirtualKeyState::Down;
	default: return VirtualKeyState::Hold;
	}
}

bool IsJoystickDevice(udev_device* device) {
	if (!device) {
		return false;
	}
	if (const char* prop = udev_device_get_property_value(device, "ID_INPUT_JOYSTICK")) {
		return std::strcmp(prop, "1") == 0;
	}
	if (const char* prop = udev_device_get_property_value(device, "ID_INPUT_GAMEPAD")) {
		return std::strcmp(prop, "1") == 0;
	}
	return false;
}

bool IsGamepadDevice(libevdev* dev) {
	if (!dev) {
		return false;
	}
	if (!libevdev_has_event_type(dev, EV_KEY)) {
		return false;
	}
	if (libevdev_has_event_code(dev, EV_KEY, BTN_GAMEPAD) ||
	    libevdev_has_event_code(dev, EV_KEY, BTN_SOUTH) ||
	    libevdev_has_event_code(dev, EV_KEY, BTN_EAST) ||
	    libevdev_has_event_code(dev, EV_KEY, BTN_NORTH) ||
	    libevdev_has_event_code(dev, EV_KEY, BTN_WEST)) {
		return true;
	}
	return false;
}


std::string ToLowerCopy(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return str;
}

} // namespace

WaylandGamepadFeed::~WaylandGamepadFeed() {
	{
		std::scoped_lock lock(deviceMutex);
		for (auto& pad : devices) {
			if (pad.evdev) {
				libevdev_free(pad.evdev);
			}
			if (pad.fd >= 0) {
				close(pad.fd);
			}
		}

		devices.clear();
	}

	if (monitor) {
		udev_monitor_unref(monitor);
		monitor = nullptr;
	}
	if (udevCtx) {
		udev_unref(udevCtx);
		udevCtx = nullptr;
	}
}

// Entry point: bootstrap udev + monitoring socket, then spawn the poll loop.
size_t WaylandGamepadFeed::EnumerateGamepads(InputPipeline* pipeline) {
	
	if (!udevCtx) {
		if(!InitializeUdev()) {
			return 0;
		}
	}
	
	udev_enumerate* enumerate = udev_enumerate_new(udevCtx);
	if (!enumerate) {
		return 0;
	}

	udev_enumerate_add_match_subsystem(enumerate, "input");
	udev_enumerate_add_match_property(enumerate, "ID_INPUT_JOYSTICK", "1");
	udev_enumerate_scan_devices(enumerate);

	udev_list_entry* devicesList = udev_enumerate_get_list_entry(enumerate);
	for (udev_list_entry* entry = devicesList; entry != nullptr; entry = udev_list_entry_get_next(entry)) {
		const char* path = udev_list_entry_get_name(entry);
		if (!path) {
			continue;
		}
		udev_device* dev = udev_device_new_from_syspath(udevCtx, path);
		if (!dev) {
			continue;
		}
		if (IsJoystickDevice(dev)) {
			AddDevice(udev_device_get_devnode(dev), pipeline);
		}
		udev_device_unref(dev);
	}

	udev_enumerate_unref(enumerate);
	return GetEnumeratedGamepadCount();
}

void WaylandGamepadFeed::PollInput(InputPipeline& pipeline, std::chrono::milliseconds blockTimeout) {
	const bool hasMonitor = monitorFd >= 0;
	pollFds.clear();
	{
		std::scoped_lock lock(deviceMutex);
		for (auto const& slot : devices) {
			pollfd fd{};
			fd.fd = slot.fd;
			fd.events = POLLIN;
			pollFds.push_back(fd);
		}
	}

	if (hasMonitor) {
		pollfd mon{};
		mon.fd = monitorFd;
		mon.events = POLLIN;
		pollFds.push_back(mon);
	}

	int ret = poll(pollFds.data(), pollFds.size(), blockTimeout.count());
	
	if (ret <= 0) {
		return;
	}

	{
		std::scoped_lock lock(deviceMutex);
		size_t pollIndex = 0;

		for (auto& device : devices) {
			if (pollIndex >= pollFds.size()) {
				break;
			}
			const auto revents = pollFds[pollIndex].revents;

			if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
				RemoveDevice(device.info.id.deviceNode, pipeline);
				continue;
			}
			if (revents & POLLIN) {
				ProcessDevice(device, pipeline);
			}

			++pollIndex;
		}
	}

	if (hasMonitor && !pollFds.empty()) {
		if (pollFds.back().revents & POLLIN) {
			HandleUdevEvents(pipeline);
		}
	}
}

bool WaylandGamepadFeed::InitializeUdev() {
	udevCtx = udev_new();
	if (!udevCtx) {
		std::cerr << "[WaylandGamepadFeed] Failed to create udev context\n";
		return false;
	}

	monitor = udev_monitor_new_from_netlink(udevCtx, "udev");
	if (!monitor) {
		std::cerr << "[WaylandGamepadFeed] Failed to create udev monitor\n";
		udev_unref(udevCtx);
		udevCtx = nullptr;
		return false;
	}

	if (udev_monitor_filter_add_match_subsystem_devtype(monitor, "input", nullptr) < 0 ||
	    udev_monitor_enable_receiving(monitor) < 0) {
		std::cerr << "[WaylandGamepadFeed] Failed to enable udev monitor\n";
		udev_monitor_unref(monitor);
		monitor = nullptr;
		udev_unref(udevCtx);
		udevCtx = nullptr;
		return false;
	}


	monitorFd = udev_monitor_get_fd(monitor);

	return true;
}

void WaylandGamepadFeed::HandleUdevEvents(InputPipeline& pipeline) {
	if (!monitor) {
		return;
	}
	while (true) {
		udev_device* dev = udev_monitor_receive_device(monitor);
		if (!dev) {
			break;
		}

		const char* subsystem = udev_device_get_subsystem(dev);
		const char* action = udev_device_get_action(dev);

		if (subsystem && std::strcmp(subsystem, "input") == 0 && action) {
			if (std::strcmp(action, "add") == 0) {
				if (IsJoystickDevice(dev)) {
					AddDevice(udev_device_get_devnode(dev), &pipeline);
				}
			} else if (std::strcmp(action, "remove") == 0) {
				const char* devnode = udev_device_get_devnode(dev);
				if (devnode) {
					RemoveDevice(devnode, pipeline);
				}
			}
		}
		udev_device_unref(dev);
	}
}

size_t WaylandGamepadFeed::GetEnumeratedGamepadCount() const {
	std::scoped_lock lock(deviceMutex);
	return devices.size();
}

std::vector<WaylandGamepadFeed::GamepadInfo> WaylandGamepadFeed::GetEnumeratedGamepadInfo() const {
	std::vector<GamepadInfo> result;
	std::scoped_lock lock(deviceMutex);
	for (auto const& slot : devices) {
		result.push_back(slot.info);
	}
	return result;
}

std::vector<WaylandGamepadFeed::GamepadId> WaylandGamepadFeed::GetLinkedGamepadIds(VirtualDevice vDev) const {
	std::vector<GamepadId> result;
	if (!weave::input::IsGamepadDevice(vDev)) {
		return result;
	}

	std::scoped_lock lock(deviceMutex);
	for (auto const& slot : devices) {
		if (slot.info.virtualDevice == vDev) {
			result.push_back(slot.info.id);
		}
	}
	return result;
}

WaylandGamepadFeed::GamepadInfo WaylandGamepadFeed::GetGamepadInfo(GamepadId const& gamepadId) const {
	std::scoped_lock lock(deviceMutex);
	for (auto const& slot : devices) {
		if (slot.info.id == gamepadId) {
			return slot.info;
		}
	}

	return {};
}

void WaylandGamepadFeed::LinkGamepad(GamepadId const& id, VirtualDevice virtualDevice) {
	if (id.Empty() || !weave::input::IsGamepadDevice(virtualDevice)) {
		return;
	}

	std::scoped_lock lock(deviceMutex);
	if(auto* device = FindDeviceById(id)) {
		device->info.virtualDevice = virtualDevice;		
	}
}

void WaylandGamepadFeed::UnlinkGamepad(GamepadId const& id) {
	if (id.Empty()) {
		return;
	}
	std::scoped_lock lock(deviceMutex);
	if(auto* device = FindDeviceById(id)) {
		device->info.virtualDevice = VirtualDevice::None;
	}
}

void WaylandGamepadFeed::UnlinkVirtualDevice(VirtualDevice vDev) {
	if (!weave::input::IsGamepadDevice(vDev)) {
		return;
	}
	auto ids = GetLinkedGamepadIds(vDev);
	for (auto const& id : ids) {
		UnlinkGamepad(id);
	}
}

void WaylandGamepadFeed::SetDeadzones(GamepadId const& id, float leftDeadzone, float rightDeadzone) {
	if (id.Empty()) {
		return;
	}
	leftDeadzone = std::clamp(leftDeadzone, 0.0f, 0.95f);
	rightDeadzone = std::clamp(rightDeadzone, 0.0f, 0.95f);

	std::scoped_lock lock(deviceMutex);
	if(auto* device = FindDeviceById(id)) {
		device->leftDeadzone = leftDeadzone;
		device->rightDeadzone = rightDeadzone;
	}
}

// Open the /dev/input node, let libevdev run the ioctl() capability probes, then register the fd.
bool WaylandGamepadFeed::AddDevice(const char* devnode, InputPipeline* pipeline) {
	if (!devnode) {
		return false;
	}

	{
		std::scoped_lock lock(deviceMutex);
		for (const auto& slot : devices) {
			if (slot.info.id.deviceNode == devnode) {
				return true;
			}
		}

	}

	int fd = open(devnode, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		std::cerr << "[WaylandGamepadFeed] Failed to open " << devnode << ": " << std::strerror(errno) << '\n';
		return false;
	}

	libevdev* evdev = nullptr;
	int err = libevdev_new_from_fd(fd, &evdev);
	if (err < 0) {
		std::cerr << "[WaylandGamepadFeed] libevdev_new_from_fd failed for " << devnode << ": " << std::strerror(-err) << '\n';
		close(fd);
		return false;
	}

	if (!IsGamepadDevice(evdev)) {
		libevdev_free(evdev);
		close(fd);
		return false;
	}

	GamepadDevice newDevice;
	newDevice.info.id.deviceNode = devnode;
	newDevice.fd = fd;
	newDevice.evdev = evdev;
	
	if (const char* name = libevdev_get_name(evdev)) {
		newDevice.info.name = name;
	} else {
		newDevice.info.name = "Gamepad";
	}

	newDevice.info.vendorId = static_cast<uint16_t>(libevdev_get_id_vendor(evdev));
	newDevice.info.productId = static_cast<uint16_t>(libevdev_get_id_product(evdev));
	newDevice.info.detectedLayout = DetectLayout(newDevice.info);
	newDevice.info.currentLayout = newDevice.info.detectedLayout;
	newDevice.leftDeadzone = kDefaultStickDeadzone;
	newDevice.rightDeadzone = kDefaultStickDeadzone;

	VirtualDevice targetDevice = VirtualDevice::None;
	for (auto vDev = static_cast<unsigned short>(VirtualDevice::_First_Gamepad); vDev <= static_cast<unsigned short>(VirtualDevice::_Last_Gamepad); ++vDev) {
		//Look for the virtual device on the registered device list
		bool found = false;
		for (auto const& pad : devices) {
			if (pad.info.virtualDevice == static_cast<VirtualDevice>(vDev)) {
				found = true;
				break;
			}
		}

		//If the device was not found, we've found a free device to assign!
		if (!found) {
			targetDevice = static_cast<VirtualDevice>(vDev);
			break;
		}
	}

	newDevice.info.virtualDevice = targetDevice;
	devices.emplace_back(std::move(newDevice));

	if(pipeline) {
		pipeline->FeedDeviceEvent(targetDevice, DeviceState::Idle);
	}

	std::cout << "[WaylandGamepadFeed] Added gamepad " << devnode << " -> "
	          << VirtualDeviceName(targetDevice) << '\n';
	return true;
}

void WaylandGamepadFeed::RemoveDevice(const std::string& devnode, InputPipeline& pipeline) {
	std::shared_ptr<GamepadDevice> removed;
	{
		std::scoped_lock lock(deviceMutex);
		std::erase_if(devices, [this, &devnode, &pipeline](auto const& pad) {
			if(pad.info.id.deviceNode == devnode) {
				pipeline.FeedDeviceEvent(pad.info.virtualDevice, DeviceState::Disconnected);
				if (pad.evdev) {
					libevdev_free(pad.evdev);
				}
				if (pad.fd >= 0) {
					close(pad.fd);
				}
				std::cout << "[WaylandGamepadFeed] Removed gamepad " << devnode << '\n';
				return true;
			}

			return false;
		});
	}
}

// Drive libevdev_next_event()/libevdev_read_status_sync and forward the resulting input_event records.
void WaylandGamepadFeed::ProcessDevice(GamepadDevice& device, InputPipeline& pipeline) {
	if (!device.evdev) {
		return;
	}

	input_event ev{};
	while (true) {
		int rc = libevdev_next_event(device.evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
		if (rc == -EAGAIN || rc < 0) {
			break;
		}

		if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
			ProcessEvent(device, ev, pipeline);
		}

		while (rc == LIBEVDEV_READ_STATUS_SYNC) {
			ProcessEvent(device, ev, pipeline);
			rc = libevdev_next_event(device.evdev, LIBEVDEV_READ_FLAG_SYNC, &ev);
		}
	}
}

void WaylandGamepadFeed::ProcessEvent(GamepadDevice& device, const input_event& ev, InputPipeline& pipeline) {
	switch (ev.type) {
	case EV_ABS:
		ProcessAbsEvent(device, ev, pipeline);
		break;
	case EV_KEY:
		ProcessKeyEvent(device, ev, pipeline);
		break;
	default:
		break;
	}
}

void WaylandGamepadFeed::ProcessAbsEvent(GamepadDevice& device, const input_event& ev, InputPipeline& pipeline) {
	const input_absinfo* info = libevdev_get_abs_info(device.evdev, ev.code);

	// Hat values
	switch(ev.code) {
		case ABS_HAT0X:
		UpdateHat(device, true, ev.value, pipeline);
		return;
	case ABS_HAT0Y:
		UpdateHat(device, false, ev.value, pipeline);
		return;
	default:
		break;
	}

	if(!info)
		return;

	// Stick values
	float value = TriggerValueDeadzone(device, *info, ev);

	switch (ev.code) {
	case ABS_X: {
		float delta = value - device.leftX;
		device.leftX = value;
		if(delta != 0.0f) {
			pipeline.FeedCursorPositionAndDelta(device.info.virtualDevice, VirtualKey::Lstick, device.leftX, device.leftY, 0.0f, delta, 0.0f, 0.0f);
		}
		break;
	}
	case ABS_Y: {
		float delta = value - device.leftY;
		device.leftY = value;
		if(delta != 0.0f) {
			pipeline.FeedCursorPositionAndDelta(device.info.virtualDevice, VirtualKey::Lstick, device.leftX, device.leftY, 0.0f, 0.0f, delta, 0.0f);
		}
		break;
	}
	case ABS_RX: {
		float delta = value - device.rightX;
		device.rightX = value;
		if(delta != 0.0f) {
			pipeline.FeedCursorPositionAndDelta(device.info.virtualDevice, VirtualKey::Rstick, device.rightX, device.rightY, 0.0f, delta, 0.0f, 0.0f);
		}
		break;
	}
	case ABS_RY: {
		float delta = value - device.rightY;
		device.rightY = value;
		if(delta != 0.0f) {
			pipeline.FeedCursorPositionAndDelta(device.info.virtualDevice, VirtualKey::Rstick, device.rightX, device.rightY, 0.0f, 0.0f, delta, 0.0f);
		}
		break;
	}
	case ABS_Z: {
		float value = NormalizeTrigger(*info, ev.value);
		float delta = value - device.triggerL;
		device.triggerL = value;
		pipeline.FeedKeyPressure(device.info.virtualDevice, VirtualKey::Trigger0, value);
		pipeline.FeedKeyPressureDelta(device.info.virtualDevice, VirtualKey::Trigger0, delta);
		break;
	}
	case ABS_RZ: {
		float value = NormalizeTrigger(*info, ev.value);
		float delta = value - device.triggerR;
		device.triggerR = value;
		pipeline.FeedKeyPressure(device.info.virtualDevice, VirtualKey::Trigger1, value);
		pipeline.FeedKeyPressureDelta(device.info.virtualDevice, VirtualKey::Trigger1, delta);
		break;
	}
	
	}
}

void WaylandGamepadFeed::ProcessKeyEvent(GamepadDevice& device, const input_event& ev, InputPipeline& pipeline) {
	VirtualKey mapped = MapEvdevButton(device, ev.code);
	if (mapped != VirtualKey::None) {
		VirtualKeyState state = ToVirtualKeyState(ev.value);
		pipeline.FeedKeyEvent(device.info.virtualDevice, mapped, state);
	}
}

void WaylandGamepadFeed::UpdateHat(GamepadDevice& device, bool horizontal, int value, InputPipeline& pipeline) {
	value = std::clamp(value, -1, 1);
	if (horizontal) {
		if (value == device.hatX) {
			return;
		}
		if (device.hatX == -1) {
			pipeline.FeedKeyEvent(device.info.virtualDevice, VirtualKey::West, VirtualKeyState::Down);
		} else if (device.hatX == 1) {
			pipeline.FeedKeyEvent(device.info.virtualDevice, VirtualKey::East, VirtualKeyState::Down);
		}
		device.hatX = value;
		if (value == -1) {
			pipeline.FeedKeyEvent(device.info.virtualDevice, VirtualKey::West, VirtualKeyState::Up);
		} else if (value == 1) {
			pipeline.FeedKeyEvent(device.info.virtualDevice, VirtualKey::East, VirtualKeyState::Up);
		}
	} else {
		if (value == device.hatY) {
			return;
		}
		if (device.hatY == -1) {
			pipeline.FeedKeyEvent(device.info.virtualDevice, VirtualKey::North, VirtualKeyState::Down);
		} else if (device.hatY == 1) {
			pipeline.FeedKeyEvent(device.info.virtualDevice, VirtualKey::South, VirtualKeyState::Down);
		}
		device.hatY = value;
		if (value == -1) {
			pipeline.FeedKeyEvent(device.info.virtualDevice, VirtualKey::North, VirtualKeyState::Up);
		} else if (value == 1) {
			pipeline.FeedKeyEvent(device.info.virtualDevice, VirtualKey::South, VirtualKeyState::Up);
		}
	}
}

float WaylandGamepadFeed::NormalizeStick(const input_absinfo& info, int value) {
	const float max = static_cast<float>(info.maximum);
	const float min = static_cast<float>(info.minimum);
	const float center = (max + min) * 0.5f;
	const float range = std::max(1.0f, (max - min) * 0.5f);
	return std::clamp((static_cast<float>(value) - center) / range, -1.0f, 1.0f);
}

float WaylandGamepadFeed::NormalizeTrigger(const input_absinfo& info, int value) {
	const float max = static_cast<float>(info.maximum);
	const float min = static_cast<float>(info.minimum);
	const float range = std::max(1.0f, max - min);
	return std::clamp((static_cast<float>(value) - min) / range, 0.0f, 1.0f);
}

float WaylandGamepadFeed::TriggerValueDeadzone(const GamepadDevice& device, const input_absinfo& info, const input_event& ev) {
	float deadzone = (ev.code == ABS_RX || ev.code == ABS_RY) ? device.rightDeadzone : device.leftDeadzone;
	float value = NormalizeStick(info, ev.value);
	float absValue = std::fabs(value);
		
	if (absValue <= deadzone) {
		return 0.0f;
	}

	if(ev.code == ABS_Y || ev.code == ABS_RY) {
		value = -value;
	}

	const float scaled = (absValue - deadzone) / std::max(0.0001f, 1.0f - deadzone);
	return ( value < 0.0f ? -scaled : scaled);
}

int WaylandGamepadFeed::ButtonIndexFromCode(unsigned code) {
	switch (code) {
	case BTN_SOUTH: return 0;
	case BTN_EAST: return 1;
	case BTN_NORTH: return 2;
	case BTN_WEST: return 3;
	case BTN_TL: return 4;
	case BTN_TR: return 5;
	case BTN_TL2: return 6;
	case BTN_TR2: return 7;
	case BTN_SELECT: return 8;
	case BTN_START: return 9;
	case BTN_THUMBL: return 10;
	case BTN_THUMBR: return 11;
	case BTN_MODE: return 12;
	default: return -1;
	}
}

VirtualKey WaylandGamepadFeed::MapLayoutButton(const GamepadDevice& device, int buttonIndex) {
	if (buttonIndex < 0) {
		return VirtualKey::None;
	}
	size_t layoutIdx = static_cast<size_t>(device.info.currentLayout);
	if (layoutIdx >= kStandardLayouts.size()) {
		layoutIdx = 0;
	}
	if (buttonIndex >= 0 && buttonIndex < static_cast<int>(kStandardLayouts[layoutIdx].size())) {
		return kStandardLayouts[layoutIdx][static_cast<size_t>(buttonIndex)];
	}
	return weave::input::OffsetKey(VirtualKey::Button0, buttonIndex);
}

VirtualKey WaylandGamepadFeed::MapEvdevButton(const GamepadDevice& device, unsigned code) {
	switch (code) {
	case BTN_DPAD_UP: return VirtualKey::North;
	case BTN_DPAD_DOWN: return VirtualKey::South;
	case BTN_DPAD_LEFT: return VirtualKey::West;
	case BTN_DPAD_RIGHT: return VirtualKey::East;
	default: return MapLayoutButton(device, ButtonIndexFromCode(code));
	}
	return VirtualKey::None;
}

GamepadLayout WaylandGamepadFeed::DetectLayout(GamepadInfo const& info) {
	auto lowerName = ToLowerCopy(info.name);
	auto contains = [&](std::string_view needle) {
		return lowerName.find(needle) != std::string::npos;
	};

	constexpr uint16_t kSonyVendor = 0x054C;
	constexpr uint16_t kMicrosoftVendor = 0x045E;

	if (info.vendorId == kSonyVendor || contains("sony") || contains("playstation") || contains("dualshock") || contains("dualsense")) {
		switch (info.productId) {
		case 0x0268: return GamepadLayout::PlayStation3;
		case 0x05C4:
		case 0x05C5:
		case 0x09CC: return GamepadLayout::PlayStation4;
		case 0x0CE6: return GamepadLayout::PlayStation5;
		default:
			if (contains("ps5") || contains("dualsense")) {
				return GamepadLayout::PlayStation5;
			}
			if (contains("ps4") || contains("dualshock 4")) {
				return GamepadLayout::PlayStation4;
			}
			if (contains("ps3") || contains("dualshock 3")) {
				return GamepadLayout::PlayStation3;
			}
			return GamepadLayout::PlayStation4;
		}
	}

	if (info.vendorId == kMicrosoftVendor || contains("xbox") || contains("microsoft")) {
		if (info.productId == 0x028E || contains("360")) {
			return GamepadLayout::XBox360Gamepad;
		}
		return GamepadLayout::XBoxOneGamepad;
	}

	return GamepadLayout::Generic;
}

void WaylandGamepadFeed::SetLayout(GamepadId const& id, GamepadLayout layout) {
	if (id.Empty() || layout == GamepadLayout::_LastLayout) {
		return;
	}

	std::scoped_lock lock(deviceMutex);
	if(auto *device = FindDeviceById(id)) {
		device->info.currentLayout = layout;		
	}
}

WaylandGamepadFeed::GamepadDevice* WaylandGamepadFeed::FindDeviceById(GamepadId const& id) {
	if (id.Empty()) {
		return nullptr;
	}
	for (auto& slot : devices) {
		if (slot.info.id == id) {
			return &slot;
		}
	}
	return nullptr;
}

} // namespace weave::input::wayland
