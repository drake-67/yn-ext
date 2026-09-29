#include "humanoid.hpp"
#include "../../cheat/initialise.hpp"
#include "../offsets.hpp"
#include "../../globals/globals.h"

namespace rbx {
	float rbx::humanoid_t::get_health() const
	{
		auto one = memory->read<std::uint64_t>(this->address + Offsets::Humanoid::Health);
		auto two = memory->read<std::uint64_t>(memory->read<std::uint64_t>(this->address + Offsets::Humanoid::Health));

		union { std::uint64_t hex; float f; } conv;
		conv.hex = one ^ two;
		return conv.f;
	}

	void rbx::humanoid_t::set_health(float value)
	{
		memory->write<std::uint64_t>(this->address + Offsets::Humanoid::Health, value);
	}

	void rbx::humanoid_t::write_gravity(float gravity)
	{
		memory->write<float>(memory->read<uintptr_t>(this->address + Offsets::BasePart::Primitive) + Offsets::Workspace::ReadOnlyGravity, gravity);
	}

	float rbx::humanoid_t::get_max_health() const
	{
		auto one = memory->read<std::uint64_t>(this->address + Offsets::Humanoid::MaxHealth);
		auto two = memory->read<std::uint64_t>(memory->read<std::uint64_t>(this->address + Offsets::Humanoid::MaxHealth));

		union { std::uint64_t hex; float f; } conv;
		conv.hex = one ^ two;
		return conv.f;
	}

	void rbx::humanoid_t::walkspeedloop(float value)
	{
		auto& humanoid = globals::game::local_player.humanoid;

		if (humanoid.address == 0)
			std::cout << "walkspeedloop no humanoid" << std::endl;

			for (int i = 0; i < 12500; i++) {
				memory->write<float>(humanoid.address + Offsets::Humanoid::Walkspeed, value);
				memory->write<float>(humanoid.address + Offsets::Humanoid::WalkspeedCheck, value);

			}
	}

	void rbx::humanoid_t::set_jumppower(float jumppower) {
		memory->write<float>(this->address + Offsets::Humanoid::JumpPower, jumppower);
	}

	void rbx::humanoid_t::set_hip_height(float HipHeight) {
		memory->write<float>(this->address + Offsets::Humanoid::HipHeight, HipHeight);
	}


	float rbx::humanoid_t::read_walkspeed()
	{
		auto& humanoid = globals::game::local_player.humanoid;

		if (humanoid.address == 0)
			std::cout << "read_walkspeed no humanoid" << std::endl;

		return memory->read<float>(humanoid.address + Offsets::Humanoid::Walkspeed);
	}



	std::int32_t rbx::humanoid_t::get_rig_type() {

		std::uint8_t rigType = memory->read<std::uint8_t>(this->address + Offsets::Humanoid::RigType);
		return static_cast<std::int32_t>(rigType);
	}


	int rbx::humanoid_t::get_humanoid_state() const
	{
		uint64_t humanoid_state = memory->read<uint64_t>(this->address + Offsets::Humanoid::HumanoidState);
		int16_t humanoid_state_id = memory->read<int16_t>(humanoid_state + Offsets::Humanoid::HumanoidStateID);
		return humanoid_state_id;
	}
}