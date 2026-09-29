#include "visualengine.hpp"
#include "../../utils/math/math.hpp"
#include "../../cheat/initialise.hpp"
#include "../offsets.hpp"


namespace rbx {

	math::vector2_t rbx::visualengine_t::get_dimensions()
	{
		return memory->read<math::vector2_t>(this->address + Offsets::VisualEngine::Dimensions);
	}

	math::matrix4_t rbx::visualengine_t::get_viewmatrix()
	{
		return memory->read<math::matrix4_t>(this->address + Offsets::VisualEngine::ViewMatrix);
	}

    math::vector2_t rbx::visualengine_t::world_to_screen(math::vector3_t world, math::vector2_t dimensions, math::matrix4_t viewmatrix) {
        math::vector4_t clipCoords = {
            world.x * viewmatrix.data[0] + world.y * viewmatrix.data[1] + world.z * viewmatrix.data[2] + viewmatrix.data[3],
            world.x * viewmatrix.data[4] + world.y * viewmatrix.data[5] + world.z * viewmatrix.data[6] + viewmatrix.data[7],
            world.x * viewmatrix.data[8] + world.y * viewmatrix.data[9] + world.z * viewmatrix.data[10] + viewmatrix.data[11],
            world.x * viewmatrix.data[12] + world.y * viewmatrix.data[13] + world.z * viewmatrix.data[14] + viewmatrix.data[15]
        };
         bool non_zero_check = true;
        if (non_zero_check) {
            if (clipCoords.w <= 1e-6f) {
                return { -1.0f, -1.0f };
            }
        }

        float inv_w = 1.0f / clipCoords.w;
        math::vector3_t ndc = { clipCoords.x * inv_w, clipCoords.y * inv_w, clipCoords.z * inv_w };

        return {
            (dimensions.x / 2.0f) * (ndc.x + 1.0f),
            (dimensions.y / 2.0f) * (1.0f - ndc.y)
        };
    }

}