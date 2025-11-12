#include "WaylandExceptions.h"

namespace weave::wayland {

WaylandError::WaylandError(const std::string& message)
	: std::runtime_error(message) {}

} // namespace weave::wayland

