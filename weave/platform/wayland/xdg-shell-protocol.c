#include "xdg-shell-client-protocol.h"

#include <stdbool.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))
#endif

static const struct wl_interface *xdg_wm_base_request_signature[] = {
	NULL,
	NULL,
	&wl_surface_interface,
	NULL,
};

static const struct wl_interface *xdg_positioner_request_signature[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

static const struct wl_interface *xdg_surface_request_signature[] = {
	NULL,
	&xdg_toplevel_interface,
	&xdg_popup_interface,
	NULL,
	NULL,
};

static const struct wl_interface *xdg_toplevel_request_signature[] = {
	NULL,
	&xdg_toplevel_interface,
	NULL,
	NULL,
	&wl_seat_interface,
	&wl_seat_interface,
	&wl_seat_interface,
	NULL,
	NULL,
	NULL,
	NULL,
	&wl_output_interface,
	NULL,
};

static const struct wl_interface *xdg_popup_request_signature[] = {
	NULL,
	&wl_seat_interface,
};

static const struct wl_message xdg_wm_base_requests[] = {
	{ "destroy", "", xdg_wm_base_request_signature + 0 },
	{ "create_positioner", "n", xdg_wm_base_request_signature + 1 },
	{ "get_xdg_surface", "no", xdg_wm_base_request_signature + 2 },
	{ "pong", "u", xdg_wm_base_request_signature + 3 },
};

static const struct wl_message xdg_wm_base_events[] = {
	{ "ping", "u", NULL },
};

static const struct wl_message xdg_positioner_requests[] = {
	{ "destroy", "", xdg_positioner_request_signature + 0 },
	{ "set_size", "ii", xdg_positioner_request_signature + 1 },
	{ "set_anchor_rect", "iiii", xdg_positioner_request_signature + 2 },
	{ "set_anchor", "u", xdg_positioner_request_signature + 3 },
	{ "set_gravity", "u", xdg_positioner_request_signature + 4 },
	{ "set_constraint_adjustment", "u", xdg_positioner_request_signature + 5 },
	{ "set_offset", "ii", xdg_positioner_request_signature + 6 },
};

static const struct wl_message xdg_surface_requests[] = {
	{ "destroy", "", xdg_surface_request_signature + 0 },
	{ "get_toplevel", "n", xdg_surface_request_signature + 1 },
	{ "get_popup", "nno", xdg_surface_request_signature + 2 },
	{ "set_window_geometry", "iiii", xdg_surface_request_signature + 3 },
	{ "ack_configure", "u", xdg_surface_request_signature + 4 },
};

static const struct wl_message xdg_surface_events[] = {
	{ "configure", "u", NULL },
};

static const struct wl_message xdg_toplevel_requests[] = {
	{ "destroy", "", xdg_toplevel_request_signature + 0 },
	{ "set_parent", "?o", xdg_toplevel_request_signature + 1 },
	{ "set_title", "s", xdg_toplevel_request_signature + 2 },
	{ "set_app_id", "s", xdg_toplevel_request_signature + 3 },
	{ "show_window_menu", "ouii", xdg_toplevel_request_signature + 4 },
	{ "move", "ou", xdg_toplevel_request_signature + 5 },
	{ "resize", "ouu", xdg_toplevel_request_signature + 6 },
	{ "set_max_size", "ii", xdg_toplevel_request_signature + 7 },
	{ "set_min_size", "ii", xdg_toplevel_request_signature + 8 },
	{ "set_maximized", "", xdg_toplevel_request_signature + 9 },
	{ "unset_maximized", "", xdg_toplevel_request_signature + 10 },
	{ "set_fullscreen", "?o", xdg_toplevel_request_signature + 11 },
	{ "unset_fullscreen", "", xdg_toplevel_request_signature + 12 },
	{ "set_minimized", "", xdg_toplevel_request_signature + 13 },
};

static const struct wl_message xdg_toplevel_events[] = {
	{ "configure", "iia", NULL },
	{ "close", "", NULL },
};

static const struct wl_message xdg_popup_requests[] = {
	{ "destroy", "", xdg_popup_request_signature + 0 },
	{ "grab", "ou", xdg_popup_request_signature + 1 },
};

static const struct wl_message xdg_popup_events[] = {
	{ "configure", "iiii", NULL },
	{ "popup_done", "", NULL },
	{ "repositioned", "u", NULL },
};

const struct wl_interface xdg_wm_base_interface = {
	"xdg_wm_base", 6,
	ARRAY_LENGTH(xdg_wm_base_requests), xdg_wm_base_requests,
	ARRAY_LENGTH(xdg_wm_base_events), xdg_wm_base_events,
};

const struct wl_interface xdg_positioner_interface = {
	"xdg_positioner", 6,
	ARRAY_LENGTH(xdg_positioner_requests), xdg_positioner_requests,
	0, NULL,
};

const struct wl_interface xdg_surface_interface = {
	"xdg_surface", 6,
	ARRAY_LENGTH(xdg_surface_requests), xdg_surface_requests,
	ARRAY_LENGTH(xdg_surface_events), xdg_surface_events,
};

const struct wl_interface xdg_toplevel_interface = {
	"xdg_toplevel", 6,
	ARRAY_LENGTH(xdg_toplevel_requests), xdg_toplevel_requests,
	ARRAY_LENGTH(xdg_toplevel_events), xdg_toplevel_events,
};

const struct wl_interface xdg_popup_interface = {
	"xdg_popup", 6,
	ARRAY_LENGTH(xdg_popup_requests), xdg_popup_requests,
	ARRAY_LENGTH(xdg_popup_events), xdg_popup_events,
};
