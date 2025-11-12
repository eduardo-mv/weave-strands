#include "WaylandMidiFeed.h"
#include <iostream>
#include <thread>

using namespace weave::input::wayland;

void WaylandMidiFeed::Test() {
    snd_seq_t *seq_handle = nullptr;

    // 1. Open ALSA sequencer
    if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
        std::cerr << "Error: cannot open ALSA sequencer.\n";
        return;
    }

    snd_seq_set_client_name(seq_handle, "MIDI Input Example (C++)");

    // 2. Create an input port
    int port_in = snd_seq_create_simple_port(
        seq_handle,
        "Input Port",
        SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_APPLICATION
    );

    int port_in2 = snd_seq_create_simple_port(
        seq_handle,
        "Input Port",
        SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_APPLICATION
    );
    (void) port_in2;

    if (port_in < 0) {
        std::cerr << "Error: cannot create sequencer port.\n";
        snd_seq_close(seq_handle);
        return;
    }

    int out_port = snd_seq_create_simple_port(
        seq_handle,
        "Output Port",
        SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
        SND_SEQ_PORT_TYPE_APPLICATION
    );

    if (out_port < 0) {
        std::cerr << "Error: cannot create port.\n";
        return;
    }

    // Send repeated notes to whoever is connected
    // Simple arpeggio pattern (C major)
    if (snd_seq_connect_to(seq_handle, out_port, 36, 0) < 0) {
        std::cerr << "Error: cannot connect from client " << std::endl;
        return;
    }

    struct Note {
        int note;
        int velocity;
        int duration_ms;
    };
    const Note tune[] = {
        {60, 30, 300},  // C4
        {64, 30, 300},  // E4
        {67, 30, 300},  // G4
        {72, 30, 600},  // C5
        {67, 30, 300},  // G4
        {64, 30, 300},  // E4
        {60, 30, 600},  // C4
    };

    std::jthread play([&]{
        while (true) {
            for (const auto &n : tune) {
                snd_seq_event_t ev;

                // NOTE ON
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

                // NOTE OFF
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


    // LISTING
    snd_seq_client_info_t *cinfo;
    snd_seq_port_info_t *pinfo;

    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_alloca(&pinfo);

    // Start with client = -1 to get the first one
    snd_seq_client_info_set_client(cinfo, -1);

    std::cout << "=== ALSA Sequencer Clients ===" << std::endl;

    while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
        int client = snd_seq_client_info_get_client(cinfo);
        const char *cname = snd_seq_client_info_get_name(cinfo);

        std::cout << "Client " << client << ": " << cname << std::endl;

        snd_seq_port_info_set_client(pinfo, client);
        snd_seq_port_info_set_port(pinfo, -1);

        while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
            int port = snd_seq_port_info_get_port(pinfo);
            const char *pname = snd_seq_port_info_get_name(pinfo);
            unsigned int caps = snd_seq_port_info_get_capability(pinfo);
            unsigned int type = snd_seq_port_info_get_type(pinfo);

            std::cout << "  Port " << port << ": " << pname << std::endl;
            std::cout << "    Caps: 0x" << std::hex << caps << std::dec;
            if (caps & SND_SEQ_PORT_CAP_READ)  std::cout << " READ";
            if (caps & SND_SEQ_PORT_CAP_WRITE) std::cout << " WRITE";
            if (caps & SND_SEQ_PORT_CAP_SUBS_READ)  std::cout << " SUBS_READ";
            if (caps & SND_SEQ_PORT_CAP_SUBS_WRITE) std::cout << " SUBS_WRITE";
            std::cout << std::endl;

            std::cout << "    Type: 0x" << std::hex << type << std::dec << std::endl;
        }
    }

    // CONNECTING
    int dest_client = 36;
    int dest_port = 0;
    if (snd_seq_connect_from(seq_handle, port_in, dest_client, dest_port) < 0) {
        std::cerr << "Error: cannot connect from client " << std::endl;
        return;
    }

    // WRTING directly to a client
    // Prepare MIDI "Note On" event
    {
        snd_seq_event_t ev;
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_source(&ev, out_port);
        snd_seq_ev_set_dest(&ev, dest_client, dest_port);
        snd_seq_ev_set_subs(&ev);           // Send to all subscribers, if any
        snd_seq_ev_set_direct(&ev);         // Send immediately

        ev.type = SND_SEQ_EVENT_NOTEON;
        ev.data.note.channel = 0;
        ev.data.note.note = 60;     // Middle C
        ev.data.note.velocity = 100;

        // Output the event
        snd_seq_event_output_direct(seq_handle, &ev);

        std::cout << "Sent NOTE ON (C4)" << std::endl;

        // Wait 500 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Send Note Off
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_source(&ev, out_port);
        snd_seq_ev_set_dest(&ev, dest_client, dest_port);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);
        ev.type = SND_SEQ_EVENT_NOTEOFF;
        ev.data.note.channel = 0;
        ev.data.note.note = 60;
        ev.data.note.velocity = 0;

        snd_seq_event_output_direct(seq_handle, &ev);

        std::cout << "Sent NOTE OFF (C4)" << std::endl;
    }


    // 3. Prepare polling descriptors
    int npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    std::vector<pollfd> pfds(npfd);
    snd_seq_poll_descriptors(seq_handle, pfds.data(), npfd, POLLIN);

    std::cout << "Listening for MIDI input..." << std::endl;
    std::cout << "Use 'aconnect -l' to list and 'aconnect <src> <dest>' to connect." << std::endl;

    // 4. Main event loop
    while (true) {
        if (poll(pfds.data(), npfd, -1) > 0) {
            snd_seq_event_t *ev = nullptr;
            while (snd_seq_event_input(seq_handle, &ev) >= 0) {
                switch (ev->type) {
                    case SND_SEQ_EVENT_NOTEON:
                        std::cout << "Port: " << int(ev->dest.port) << " From: " << int(ev->source.client) << ":" << int(ev->source.port)
                                  << " Note On: ch " << int(ev->data.note.channel)
                                  << " note " << int(ev->data.note.note)
                                  << " vel " << int(ev->data.note.velocity)
                                  << std::endl;
                        break;
                    case SND_SEQ_EVENT_NOTEOFF:
                        std::cout << "Port: " << int(ev->dest.port) << " From: " << int(ev->source.client) << ":" << int(ev->source.port)
                                  << " Note Off: ch " << int(ev->data.note.channel)
                                  << " note " << int(ev->data.note.note)
                                  << " vel " << int(ev->data.note.velocity)
                                  << std::endl;
                        break;
                    default:
                        std::cout << "Other event type: " << ev->type << std::endl;
                        break;
                }
                snd_seq_free_event(ev);
            }
        }
    }

    play.join();

    snd_seq_close(seq_handle);
}