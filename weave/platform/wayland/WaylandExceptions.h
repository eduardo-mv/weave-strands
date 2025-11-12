#pragma once

#include <stdexcept>
#include <string>

namespace weave::wayland {

class WaylandError : public std::runtime_error {
public:
	explicit WaylandError(const std::string& message);
};

} // namespace weave::wayland

