#pragma once
#include <cstdint>
#include "instance_t.hpp"
namespace rbx {

	struct humanoid_t final : public instance_t
	{
		using instance_t::instance_t;
		float get_health() const;
		void set_health(float value);
		void write_gravity(float gravity);
		float get_max_health() const;
		void walkspeedloop(float value);
		void set_jumppower(float jumppower);
		void set_hip_height(float HipHeight);
		float read_walkspeed();
		std::int32_t get_rig_type();
		int get_humanoid_state() const;

	};


} // namespace rbx