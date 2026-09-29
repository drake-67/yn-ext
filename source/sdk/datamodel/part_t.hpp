#pragma once
#include <cstdint>
#include "instance_t.hpp"
namespace rbx {

	struct part_t final : public instance_t
	{
		using instance_t::instance_t;
		math::vector3_t get_position();
		math::matrix3_t get_rotation();
		math::vector3_t get_size();
		math::vector3_t get_part_size() const;
		math::vector3_t get_transparency();
		void write_velocity(math::vector3_t velo);
		bool get_anchored();
		math::vector3_t get_velocity();
		math::vector3_t get_coordinate_frame();
		rbx::part_t get_primitive() const;
		math::vector3_t get_part_position();
		void set_part_postion(math::vector3_t position);
		void set_rotation(const math::matrix3_t& rot);
		math::vector3_t get_part_pos();

	};


} // namespace rbx