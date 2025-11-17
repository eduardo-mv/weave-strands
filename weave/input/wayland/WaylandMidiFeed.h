#pragma once

#include <alsa/asoundlib.h>
#include <poll.h>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "weave/input/system/InputPipeline.h"
#include "weave/input/system/VirtualDevices.h"

namespace weave::input::wayland {

class WaylandMidiFeed {
public:
    struct MidiSource {
        int client{};
        int port{};
        std::string name;
        bool isHardware{};
    };

    struct MidiDevice {
        MidiSource midiSource;
        VirtualDevice virtualDevice = VirtualDevice::None;
        bool subscribed = false;
    };

    WaylandMidiFeed() = default;
    ~WaylandMidiFeed();

    size_t EnumerateMidiSources(bool hardwareOnly, InputPipeline& pipeline);

	std::vector<MidiSource> GetAllMidiSources() const;
	std::vector<MidiSource> GetLinkedMidiSources(VirtualDevice vDev) const;

    bool LinkMidiSource(MidiSource const& endpoint, VirtualDevice device);
	void UnlinkMidiSource(MidiSource const& id);
    void UnlinkVirtualDevice(VirtualDevice device);

	void PollInput(std::chrono::milliseconds blockTimeout);

    void Test();

private:
    

    bool EnsureSequencerLocked();
    void UpdatePollDescriptorsLocked();
    std::vector<MidiSource> DiscoverSources(bool hardwareOnly) const;
    MidiSource MakeSourceFromInfo(snd_seq_client_info_t* cinfo, snd_seq_port_info_t* pinfo) const;
    
    void AddSource(uint8_t client, uint8_t port);
    void RemoveSource(uint8_t client, uint8_t port);
    void RefreshSource(uint8_t client, uint8_t port);
    
    void RebuildSourceIndexLocked();

    bool SubscribeDevice(MidiDevice& device);
    void UnsubscribeDevice(MidiDevice& device);
    std::vector<MidiSource> GetLinkedMidiSourcesLocked(VirtualDevice vDev) const;
    std::optional<size_t> FindDeviceIndexBySourceLocked(int client, int port) const;
    bool ProcessAnnouncementEvent(snd_seq_event_t const& ev);
    void ProcessDeviceEvent(VirtualDevice device, snd_seq_event_t const& ev);
    bool SubscribeToAnnouncementsLocked();

    std::vector<MidiDevice> midiDevices;
    std::unordered_map<uint64_t, size_t> midiSourceIndex;
	mutable std::mutex midiDevicesMutex;

    snd_seq_t* seqHandle{ nullptr };
    int inputPort{ -1 };
    bool announceSubscribed{ false };
    std::vector<pollfd> pollFds;
    InputPipeline* inputPipeline{ nullptr };
};

} // namespace weave::input::wayland
