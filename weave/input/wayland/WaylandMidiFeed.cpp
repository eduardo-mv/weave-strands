#include "WaylandMidiFeed.h"

#include <algorithm>
#include <array>
#include <set>
#include <cerrno>
#include <iostream>
#include <unordered_set>

#include "weave/input/system/InputEventData.h"
#include "weave/input/system/VirtualKeys.h"

using namespace weave::input;
using namespace weave::input::wayland;

namespace {
constexpr uint64_t MakeEndpointKey(int client, int port) {
    return (static_cast<uint64_t>(client) << 32) | static_cast<uint32_t>(port);
}

VirtualDevice nextAvailableVirtualDevice(std::vector<WaylandMidiFeed::MidiDevice> const& midiDevices){
    VirtualDevice nextAvailableDevice = VirtualDevice::Midi0;
    while(nextAvailableDevice <= VirtualDevice::Midi15) {
        auto it = std::ranges::find_if(midiDevices, [&](auto const& dev){
            return dev.virtualDevice == nextAvailableDevice;
        });

        if(it == midiDevices.end()) {
            break;
        }

        nextAvailableDevice = weave::input::OffsetDevice(nextAvailableDevice, 1);
    }

    if (nextAvailableDevice > VirtualDevice::Midi15) {
        nextAvailableDevice = VirtualDevice::Midi0;
    }
    return nextAvailableDevice;
};

}

WaylandMidiFeed::~WaylandMidiFeed() {
    std::scoped_lock lock(midiDevicesMutex);

    if (seqHandle) {
        for (auto& device : midiDevices) {
            UnsubscribeDevice(device);
        }
        snd_seq_close(seqHandle);
        seqHandle = nullptr;
        inputPort = -1;
        announceSubscribed = false;
    }
}

size_t WaylandMidiFeed::EnumerateMidiSources(bool hardwareOnly, InputPipeline& pipeline) {
    if(inputPipeline) {
        std::scoped_lock lock(midiDevicesMutex);
        return midiDevices.size(); 
    }

    inputPipeline = &pipeline;
    std::vector<InputEventData> events;
    size_t deviceCount = 0;

    {
        std::scoped_lock lock(midiDevicesMutex);

        if (!EnsureSequencerLocked()) {
            for (auto& device : midiDevices) {
                UnsubscribeDevice(device);
            }
            if (inputPipeline) {
                for (auto const& device : midiDevices) {
                    if (device.virtualDevice != VirtualDevice::None) {
                        inputPipeline->FeedEvent(
                            device.virtualDevice, VirtualKey::None,
                            DeviceStateEvent{ DeviceState::Disconnected });
                    }
                }
            }
            midiDevices.clear();
            midiSourceIndex.clear();
            return 0;
        }

        auto sources = DiscoverSources(hardwareOnly);
        for(auto const& source : sources) {
            MidiDevice entry;
            entry.midiSource = source;
            entry.subscribed = false;
            entry.virtualDevice = nextAvailableVirtualDevice(midiDevices);
            if(entry.virtualDevice != VirtualDevice::None) {
                midiDevices.emplace_back(entry);
                SubscribeDevice(entry);
                events.emplace_back(InputEventData{entry.virtualDevice, VirtualKey::None,
                    DeviceStateEvent{ DeviceState::Idle }});
            }
        }

        RebuildSourceIndexLocked();

        deviceCount = midiDevices.size();
    }

    for(auto &event : events) {
        inputPipeline->FeedEvent(std::move(event));        
    }
    
    return deviceCount;
}

std::vector<WaylandMidiFeed::MidiSource> WaylandMidiFeed::GetAllMidiSources() const {
    std::scoped_lock lock(midiDevicesMutex);
    return DiscoverSources(false);
}

std::vector<WaylandMidiFeed::MidiSource> WaylandMidiFeed::GetLinkedMidiSources(VirtualDevice vDev) const {
    std::scoped_lock lock(midiDevicesMutex);
    return GetLinkedMidiSourcesLocked(vDev);
}

std::vector<WaylandMidiFeed::MidiSource> WaylandMidiFeed::GetLinkedMidiSourcesLocked(VirtualDevice vDev) const {
    std::vector<MidiSource> result;
    for (auto const& device : midiDevices) {
        if (device.virtualDevice == vDev) {
            result.push_back(device.midiSource);
        }
    }
    return result;
}

bool WaylandMidiFeed::LinkMidiSource(MidiSource const& endpoint, VirtualDevice device) {
    std::scoped_lock lock(midiDevicesMutex);
    if (!EnsureSequencerLocked() || !inputPipeline) {
        return false;
    }

    auto deviceIndex = FindDeviceIndexBySourceLocked(endpoint.client, endpoint.port);
    if (!deviceIndex.has_value()) {
        MidiDevice entry;
        entry.midiSource = endpoint;
        midiDevices.emplace_back(std::move(entry));
        RebuildSourceIndexLocked();
        deviceIndex = midiDevices.size() - 1;
    }

    auto& entry = midiDevices[*deviceIndex];
    auto previousDevice = entry.virtualDevice;
    entry.virtualDevice = device;
    
    if (previousDevice != VirtualDevice::None) {
        if (auto existing = GetLinkedMidiSourcesLocked(previousDevice); existing.empty()) {
            UnsubscribeDevice(entry);
            inputPipeline->FeedEvent(
                previousDevice, VirtualKey::None,
                DeviceStateEvent{ DeviceState::Disconnected });
        }        
    }

    if(device != VirtualDevice::None) {
        if (!entry.subscribed) {
            SubscribeDevice(entry);
        }

        inputPipeline->FeedEvent(
            device, VirtualKey::None, DeviceStateEvent{ DeviceState::Idle });
    }

    return true;
}

void WaylandMidiFeed::UnlinkMidiSource(MidiSource const& src) {
    LinkMidiSource(src, VirtualDevice::None);
}

void WaylandMidiFeed::UnlinkVirtualDevice(VirtualDevice device) {
    std::scoped_lock lock(midiDevicesMutex);
    if (!IsMidiDevice(device)) {
        return;
    }

    for (auto& entry : midiDevices) {
        if (entry.virtualDevice == device) {
            entry.virtualDevice = VirtualDevice::None;
            UnsubscribeDevice(entry);
            if (inputPipeline) {
                inputPipeline->FeedEvent(
                    device, VirtualKey::None,
                    DeviceStateEvent{ DeviceState::Disconnected });
            }
        }
    }
}

bool WaylandMidiFeed::EnsureSequencerLocked() {
    if (seqHandle && inputPort >= 0) {
        return true;
    }

    if (snd_seq_open(&seqHandle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
        seqHandle = nullptr;
        inputPort = -1;
        return false;
    }

    snd_seq_set_client_name(seqHandle, "Weave Wayland MIDI");

    inputPort = snd_seq_create_simple_port(
        seqHandle,
        "Weave MIDI Input",
        SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_APPLICATION);

    if (inputPort < 0) {
        snd_seq_close(seqHandle);
        seqHandle = nullptr;
        inputPort = -1;
        return false;
    }
    SubscribeToAnnouncementsLocked();

    return true;
}

void WaylandMidiFeed::UpdatePollDescriptorsLocked() {
    pollFds.clear();
    if (!seqHandle) {
        return;
    }
    int count = snd_seq_poll_descriptors_count(seqHandle, POLLIN);
    pollFds.resize(count);
    snd_seq_poll_descriptors(seqHandle, pollFds.data(), count, POLLIN);
}

std::vector<WaylandMidiFeed::MidiSource> WaylandMidiFeed::DiscoverSources(bool hardwareOnly) const {
    std::vector<MidiSource> result;
    if (!seqHandle) {
        return result;
    }

    snd_seq_client_info_t* cinfo;
    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_t* pinfo;
    snd_seq_port_info_alloca(&pinfo);

    snd_seq_client_info_set_client(cinfo, -1);

    while (snd_seq_query_next_client(seqHandle, cinfo) >= 0) {

        int client = snd_seq_client_info_get_client(cinfo);
        snd_seq_port_info_set_client(pinfo, client);
        snd_seq_port_info_set_port(pinfo, -1);

        while (snd_seq_query_next_port(seqHandle, pinfo) >= 0) {
            unsigned caps = snd_seq_port_info_get_capability(pinfo);
            if ((caps & SND_SEQ_PORT_CAP_READ) == 0 &&
                (caps & SND_SEQ_PORT_CAP_SUBS_READ) == 0) {
                continue;
            }

            MidiSource src = MakeSourceFromInfo(cinfo, pinfo);
            if (!hardwareOnly || src.isHardware) {
                result.emplace_back(std::move(src));
            }
        }
    }

    return result;
}

WaylandMidiFeed::MidiSource WaylandMidiFeed::MakeSourceFromInfo(snd_seq_client_info_t* cinfo, snd_seq_port_info_t* pinfo) const {
    
    int client = snd_seq_client_info_get_client(cinfo);
    const char* clientName = snd_seq_client_info_get_name(cinfo);

    MidiSource src;
    src.client = client;
    src.port = snd_seq_port_info_get_port(pinfo);
    src.name = clientName ? clientName : "Unknown";
    if (const char* pname = snd_seq_port_info_get_name(pinfo); pname) {
        src.name += " - ";
        src.name += pname;
    }

    unsigned type = snd_seq_port_info_get_type(pinfo);
    src.isHardware = (type & SND_SEQ_PORT_TYPE_HARDWARE) != 0;

    return src;
}

void WaylandMidiFeed::AddSource(uint8_t client, uint8_t port) {

    auto it = std::ranges::find_if(midiDevices, [&](auto const& dev) {
        return dev.midiSource.client == client && dev.midiSource.port == port;
    });

    if(it != midiDevices.end()) {
        return;
    }

    snd_seq_client_info_t* cinfo;
    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_t* pinfo;
    snd_seq_port_info_alloca(&pinfo);

    snd_seq_get_any_client_info(seqHandle, client, cinfo);
    snd_seq_get_any_port_info(seqHandle, client, port, pinfo);

    
    MidiDevice midiDevice;
    midiDevice.midiSource = MakeSourceFromInfo(cinfo, pinfo);
    midiDevice.virtualDevice = nextAvailableVirtualDevice(midiDevices);
    SubscribeDevice(midiDevice);
    midiDevices.emplace_back(std::move(midiDevice));

    RebuildSourceIndexLocked();


    inputPipeline->FeedEvent(midiDevice.virtualDevice, VirtualKey::None, DeviceStateEvent{ .state = DeviceState::Idle});
}

void WaylandMidiFeed::RemoveSource(uint8_t client, uint8_t port){
    auto it = std::ranges::find_if(midiDevices, [&](auto const& dev) {
        return dev.midiSource.client == client && dev.midiSource.port == port;
    });

    if(it != midiDevices.end()) {
        auto vdevice = it->virtualDevice;
        midiDevices.erase(it);

        RebuildSourceIndexLocked();

        inputPipeline->FeedEvent(vdevice, VirtualKey::None, DeviceStateEvent{ .state = DeviceState::Disconnected});
    }

}

void WaylandMidiFeed::RefreshSource(uint8_t client, uint8_t port){
    auto it = std::ranges::find_if(midiDevices, [&](auto const& dev) {
        return dev.midiSource.client == client && dev.midiSource.port == port;
    });

    if(it != midiDevices.end()) {
        auto vdevice = it->virtualDevice;
        midiDevices.erase(it);
        AddSource(client, port);
        midiDevices.back().virtualDevice = vdevice;
    }
}

void WaylandMidiFeed::RebuildSourceIndexLocked() {
    midiSourceIndex.clear();
    for (size_t i = 0; i < midiDevices.size(); ++i) {
        auto const& src = midiDevices[i].midiSource;
        midiSourceIndex[MakeEndpointKey(src.client, src.port)] = i;
    }
}

bool WaylandMidiFeed::SubscribeDevice(MidiDevice& device) {
    if (device.subscribed) {
        return true;
    }
    if (!seqHandle || inputPort < 0) {
        return false;
    }

    int clientId = snd_seq_client_id(seqHandle);
    if (clientId < 0) {
        return false;
    }

    snd_seq_port_subscribe_t* sub;
    snd_seq_port_subscribe_alloca(&sub);
    snd_seq_addr_t sender{};
    sender.client = device.midiSource.client;
    sender.port = device.midiSource.port;
    snd_seq_addr_t dest{};
    dest.client = clientId;
    dest.port = inputPort;

    snd_seq_port_subscribe_set_sender(sub, &sender);
    snd_seq_port_subscribe_set_dest(sub, &dest);
    snd_seq_port_subscribe_set_time_real(sub, 0);
    snd_seq_port_subscribe_set_time_update(sub, 0);

    int rc = snd_seq_subscribe_port(seqHandle, sub);
    if (rc < 0) {
        if (rc == -EEXIST || rc == -EBUSY) {
            device.subscribed = true;
            return true;
        }
        std::cerr << "[WaylandMidiFeed] Failed to subscribe MIDI source "
                  << device.midiSource.client << ':' << device.midiSource.port
                  << " -> " << snd_strerror(rc) << '\n';
        return false;
    }

    device.subscribed = true;
    return true;
}

void WaylandMidiFeed::UnsubscribeDevice(MidiDevice& device) {
    if (!device.subscribed || !seqHandle || inputPort < 0) {
        device.subscribed = false;
        return;
    }

    int clientId = snd_seq_client_id(seqHandle);
    if (clientId < 0) {
        device.subscribed = false;
        return;
    }

    snd_seq_port_subscribe_t* sub;
    snd_seq_port_subscribe_alloca(&sub);
    snd_seq_addr_t sender{};
    sender.client = device.midiSource.client;
    sender.port = device.midiSource.port;
    snd_seq_addr_t dest{};
    dest.client = clientId;
    dest.port = inputPort;

    snd_seq_port_subscribe_set_sender(sub, &sender);
    snd_seq_port_subscribe_set_dest(sub, &dest);
    snd_seq_port_subscribe_set_time_real(sub, 0);
    snd_seq_port_subscribe_set_time_update(sub, 0);

    int rc = snd_seq_unsubscribe_port(seqHandle, sub);
    if (rc < 0 && rc != -ENOENT) {
        std::cerr << "[WaylandMidiFeed] Failed to unsubscribe MIDI source "
                  << device.midiSource.client << ':' << device.midiSource.port
                  << " -> " << snd_strerror(rc) << '\n';
    }

    device.subscribed = false;
}

std::optional<size_t> WaylandMidiFeed::FindDeviceIndexBySourceLocked(int client, int port) const {
    auto key = MakeEndpointKey(client, port);
    auto it = midiSourceIndex.find(key);
    if (it != midiSourceIndex.end() && it->second < midiDevices.size()) {
        return it->second;
    }
    return std::nullopt;
}

void WaylandMidiFeed::PollInput(std::chrono::milliseconds blockTimeout) {
    {
        std::unique_lock lock(midiDevicesMutex);
        if (!inputPipeline) {
            return;
        }
        if (!EnsureSequencerLocked()) {
            return;
        }

        if(pollFds.empty()) {
            UpdatePollDescriptorsLocked();
        }
        lock.unlock();
    }

    if (pollFds.empty()) {
        return;
    }

    int timeout = static_cast<int>(blockTimeout.count());
    int ret = poll(pollFds.data(), pollFds.size(), timeout);
    if (ret <= 0) {
        return;
    }

    std::unique_lock lock(midiDevicesMutex);
    if (!seqHandle || !inputPipeline) {
        return;
    }

    snd_seq_event_t* ev = nullptr;
    while (snd_seq_event_input(seqHandle, &ev) >= 0) {
        if (!ProcessAnnouncementEvent(*ev)) {
            auto deviceIndex = FindDeviceIndexBySourceLocked(ev->source.client, ev->source.port);
            if (deviceIndex) {
                auto const& device = midiDevices[*deviceIndex];
                if (device.virtualDevice != VirtualDevice::None) {
                    ProcessDeviceEvent(device.virtualDevice, *ev);
                }
            }
        }

        snd_seq_free_event(ev);
    }
    lock.unlock();
}

bool WaylandMidiFeed::ProcessAnnouncementEvent(snd_seq_event_t const& ev) {
    switch (ev.type) {
        case SND_SEQ_EVENT_CLIENT_START:
        case SND_SEQ_EVENT_PORT_START: {
            AddSource(ev.data.connect.sender.client, ev.data.connect.sender.port);
            break;
        }
        
        case SND_SEQ_EVENT_CLIENT_EXIT:
        case SND_SEQ_EVENT_PORT_EXIT: {
            RemoveSource(ev.data.connect.sender.client, ev.data.connect.sender.port);
            break;
        }

        case SND_SEQ_EVENT_CLIENT_CHANGE: 
        case SND_SEQ_EVENT_PORT_CHANGE: {
            RefreshSource(ev.data.connect.sender.client, ev.data.connect.sender.port);
            break;
        }
        case SND_SEQ_EVENT_PORT_SUBSCRIBED:
        case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
        break;
        default:
            return false;
    }

    UpdatePollDescriptorsLocked();

    return true;
}

void WaylandMidiFeed::ProcessDeviceEvent(VirtualDevice device, snd_seq_event_t const& ev) {
    switch (ev.type) {
    case SND_SEQ_EVENT_NOTEON:
    case SND_SEQ_EVENT_NOTEOFF: {
        bool pressed = (ev.type == SND_SEQ_EVENT_NOTEON) && ev.data.note.velocity > 0;
        if (inputPipeline) {
            inputPipeline->FeedEvent(device, VirtualKey::MidiNote,
            MidiNoteEvent{ static_cast<uint8_t>(ev.data.note.note),
                           static_cast<uint8_t>(ev.data.note.velocity),
                           static_cast<uint8_t>(ev.data.note.channel),
                           pressed });
        }
        break;
    }
    case SND_SEQ_EVENT_CONTROLLER: {
        if (inputPipeline) {
            inputPipeline->FeedEvent(device, VirtualKey::MidiControl,
            MidiControlEvent{ static_cast<uint8_t>(ev.data.control.param),
                              static_cast<uint8_t>(ev.data.control.value),
                              static_cast<uint8_t>(ev.data.control.channel) });
        }
        break;
    }
    case SND_SEQ_EVENT_PITCHBEND: {
        constexpr int kPitchMin = -8192;
        constexpr int kPitchMax = 8191;
        int clampedValue = std::clamp(ev.data.control.value, kPitchMin, kPitchMax);
        if (inputPipeline) {
            inputPipeline->FeedEvent(device, VirtualKey::MidiPitchBend,
            MidiPitchBendEvent{ static_cast<uint8_t>(ev.data.control.channel),
                                clampedValue });
        }
        break;
    }
    case SND_SEQ_EVENT_PGMCHANGE: {
        if (inputPipeline) {
            inputPipeline->FeedEvent(device, VirtualKey::MidiProgramChange,
            MidiProgramEvent{ static_cast<uint8_t>(ev.data.control.channel),
                              static_cast<uint8_t>(ev.data.control.value) });
        }
        break;
    }
    case SND_SEQ_EVENT_CHANPRESS: {
        if (inputPipeline) {
            inputPipeline->FeedEvent(device, VirtualKey::MidiChannelPressure,
                MidiChannelPressureEvent{ static_cast<uint8_t>(ev.data.control.channel),
                                          static_cast<uint8_t>(ev.data.control.value) });
        }
        break;
    }
    case SND_SEQ_EVENT_KEYPRESS: {
        if (inputPipeline) {
            inputPipeline->FeedEvent(device, VirtualKey::MidiPolyPressure,
                MidiPolyPressureEvent{ static_cast<uint8_t>(ev.data.note.note),
                                       static_cast<uint8_t>(ev.data.note.channel),
                                       static_cast<uint8_t>(ev.data.note.velocity) });
        }
        break;
    }
    default:
        break;
    }
}

bool WaylandMidiFeed::SubscribeToAnnouncementsLocked() {
    if (announceSubscribed) {
        return true;
    }
    if (!seqHandle || inputPort < 0) {
        return false;
    }

    int clientId = snd_seq_client_id(seqHandle);
    if (clientId < 0) {
        return false;
    }

    snd_seq_port_subscribe_t* sub;
    snd_seq_port_subscribe_alloca(&sub);
    snd_seq_addr_t sender{};
    sender.client = SND_SEQ_CLIENT_SYSTEM;
    sender.port = SND_SEQ_PORT_SYSTEM_ANNOUNCE;
    snd_seq_addr_t dest{};
    dest.client = clientId;
    dest.port = inputPort;

    snd_seq_port_subscribe_set_sender(sub, &sender);
    snd_seq_port_subscribe_set_dest(sub, &dest);
    snd_seq_port_subscribe_set_time_real(sub, 0);
    snd_seq_port_subscribe_set_time_update(sub, 0);

    int rc = snd_seq_subscribe_port(seqHandle, sub);
    if (rc < 0 && rc != -EEXIST) {
        std::cerr << "[WaylandMidiFeed] Failed to subscribe to ALSA announce port -> "
                  << snd_strerror(rc) << '\n';
        return false;
    }

    announceSubscribed = true;
    return true;
}

// IGNORE this function
void WaylandMidiFeed::Test() {
    snd_seq_t* seq_handle = nullptr;
    if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
        std::cerr << "Error: cannot open ALSA sequencer.\n";
        return;
    }

    snd_seq_set_client_name(seq_handle, "Weave MIDI Test");

    int port_in = snd_seq_create_simple_port(
        seq_handle,
        "Weave MIDI Test Input",
        SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_APPLICATION);

    int out_port = snd_seq_create_simple_port(
        seq_handle,
        "Weave MIDI Test Output",
        SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
        SND_SEQ_PORT_TYPE_APPLICATION);

    if (port_in < 0 || out_port < 0) {
        std::cerr << "Error: cannot create MIDI test port.\n";
        snd_seq_close(seq_handle);
        return;
    }

    struct Note { int note; int velocity; int duration_ms; };
    const Note tune[] = {
        {60, 30, 300}, {64, 30, 300}, {67, 30, 300}, {72, 30, 600},
        {67, 30, 300}, {64, 30, 300}, {60, 30, 600},
    };

    std::jthread play([seq_handle, out_port, tune] {
        while (true) {
            for (auto const& n : tune) {
                snd_seq_event_t ev;
                snd_seq_ev_clear(&ev);
                snd_seq_ev_set_source(&ev, out_port);
                snd_seq_ev_set_subs(&ev);
                snd_seq_ev_set_direct(&ev);
                ev.type = SND_SEQ_EVENT_NOTEON;
                ev.data.note.channel = 0;
                ev.data.note.note = n.note;
                ev.data.note.velocity = n.velocity;
                snd_seq_event_output_direct(seq_handle, &ev);
                std::this_thread::sleep_for(std::chrono::milliseconds(n.duration_ms));
                snd_seq_ev_clear(&ev);
                snd_seq_ev_set_source(&ev, out_port);
                snd_seq_ev_set_subs(&ev);
                snd_seq_ev_set_direct(&ev);
                ev.type = SND_SEQ_EVENT_NOTEOFF;
                ev.data.note.channel = 0;
                ev.data.note.note = n.note;
                ev.data.note.velocity = 0;
                snd_seq_event_output_direct(seq_handle, &ev);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    snd_seq_client_info_t* cinfo;
    snd_seq_port_info_t* pinfo;
    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_alloca(&pinfo);
    snd_seq_client_info_set_client(cinfo, -1);

    std::cout << "=== ALSA Sequencer Clients ===" << std::endl;
    while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
        int client = snd_seq_client_info_get_client(cinfo);
        const char* cname = snd_seq_client_info_get_name(cinfo);
        std::cout << "Client " << client << ": " << cname << std::endl;
        snd_seq_port_info_set_client(pinfo, client);
        snd_seq_port_info_set_port(pinfo, -1);
        while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
            int port = snd_seq_port_info_get_port(pinfo);
            const char* pname = snd_seq_port_info_get_name(pinfo);
            unsigned caps = snd_seq_port_info_get_capability(pinfo);
            unsigned type = snd_seq_port_info_get_type(pinfo);
            std::cout << "  Port " << port << ": " << pname << std::endl;
            std::cout << "    Caps: 0x" << std::hex << caps << std::dec << std::endl;
            std::cout << "    Type: 0x" << std::hex << type << std::dec << std::endl;
        }
    }

    snd_seq_close(seq_handle);
}
