#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"

#include <wayland-client.h>
#include <wayland-util.h>

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))
#endif

static const struct wl_interface *zxdg_decoration_manager_v1_requests[] = {
	NULL,
	&xdg_toplevel_interface,
};

static const struct wl_message zxdg_decoration_manager_v1_messages[] = {
	{ "destroy", "", zxdg_decoration_manager_v1_requests + 0 },
	{ "get_toplevel_decoration", "no", zxdg_decoration_manager_v1_requests + 1 },
};

const struct wl_interface zxdg_decoration_manager_v1_interface = {
	"zxdg_decoration_manager_v1", 1,
	ARRAY_LENGTH(zxdg_decoration_manager_v1_messages), zxdg_decoration_manager_v1_messages,
	0, NULL,
};

static const struct wl_interface *zxdg_toplevel_decoration_v1_requests[] = {
	NULL,
	NULL,
};

static const struct wl_message zxdg_toplevel_decoration_v1_messages[] = {
	{ "destroy", "", zxdg_toplevel_decoration_v1_requests + 0 },
	{ "set_mode", "u", zxdg_toplevel_decoration_v1_requests + 1 },
	{ "unset_mode", "", zxdg_toplevel_decoration_v1_requests + 0 },
};

static const struct wl_message zxdg_toplevel_decoration_v1_events[] = {
	{ "configure", "u", NULL },
};

const struct wl_interface zxdg_toplevel_decoration_v1_interface = {
	"zxdg_toplevel_decoration_v1", 1,
	ARRAY_LENGTH(zxdg_toplevel_decoration_v1_messages), zxdg_toplevel_decoration_v1_messages,
	ARRAY_LENGTH(zxdg_toplevel_decoration_v1_events), zxdg_toplevel_decoration_v1_events,
};

