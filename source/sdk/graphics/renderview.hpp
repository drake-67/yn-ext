#pragma once
#include <cstdint>
#include "../../sdk/datamodel/instance_t.hpp"

namespace rbx {
	class renderview_t : public instance_t {
	public:
		std::uint64_t address = 0;
		std::uint64_t get_render_view();

	};
} // namespace rbx