#pragma once
#include <cstdint>
#include "../../sdk/datamodel/instance_t.hpp"

namespace rbx {

	struct visualengine_t final : public addressable_t
	{
		math::vector2_t get_dimensions();
		math::matrix4_t get_viewmatrix();
		math::vector2_t world_to_screen(math::vector3_t world, math::vector2_t dimensions, math::matrix4_t viewmatrix);
	};
} // namespace rbx