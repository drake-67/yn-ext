#pragma once
#include <cstdint>
#include "instance_t.hpp"

namespace rbx {

	struct camera_t final : public instance_t
	{
		using instance_t::instance_t;
		math::vector3_t get_camera_position();
		math::matrix3_t get_camera_rotation();
		void set_camera_position(const math::vector3_t& pos);
		void set_camera_rotation(const math::matrix3_t& rot);
	};


} // namespace rbx