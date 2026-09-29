#include "part_t.hpp"
#include "players.hpp"
#include "../../cheat/initialise.hpp"
#include "../offsets.hpp"
#include <chrono>

math::vector3_t rbx::part_t::get_position()
{
	return memory->read<math::vector3_t>(this->address + Offsets::BasePart::Position);
}

math::matrix3_t rbx::part_t::get_rotation()
{
	return memory->read<math::matrix3_t>(this->address + Offsets::BasePart::Rotation);
}

math::vector3_t rbx::part_t::get_size()
{
	return memory->read<math::vector3_t>(this->address + Offsets::BasePart::Size);

}


math::vector3_t rbx::part_t::get_part_size() const
{
    rbx::instance_t primitive_instance = this->get_primitive();
    return memory->read<math::vector3_t>(primitive_instance.address + Offsets::BasePart::Primitive);
}

math::vector3_t rbx::part_t::get_transparency()
{
	return memory->read<math::vector3_t>(this->address + Offsets::BasePart::Transparency);

}

void rbx::part_t::write_velocity(math::vector3_t velo) {
    auto primitive = memory->read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);
    for (int lawwtf_bestcoder = 0; lawwtf_bestcoder < 1; lawwtf_bestcoder++) {
        memory->write<math::vector3_t>(primitive + Offsets::BasePart::AssemblyLinearVelocity, velo);
    }
}

bool rbx::part_t::get_anchored()
{
    return memory->read<bool>(this->address + Offsets::PrimitiveFlags::Anchored);
}

math::vector3_t rbx::part_t::get_velocity() {
    if (!this->address)
        return math::vector3_t{};

    auto primitive = memory->read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);
    if (!primitive)
        return math::vector3_t{};

    return memory->read<math::vector3_t>(primitive + Offsets::BasePart::AssemblyAngularVelocity);
}

math::vector3_t rbx::part_t::get_coordinate_frame()
{
	return memory->read<math::vector3_t>(this->address + Offsets::BasePart::Rotation);

}

rbx::part_t rbx::part_t::get_primitive() const
{
	return memory->read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);

}

math::vector3_t rbx::part_t::get_part_position() {

    auto primitive = memory->read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);

    return memory->read<math::vector3_t>(primitive + Offsets::BasePart::Position);
}


void rbx::part_t::set_part_postion(math::vector3_t position) {
    if (!this->address)
        return;

    auto primitive = memory->read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);
    if (!primitive)
        return;

    memory->write<math::vector3_t>(primitive + Offsets::BasePart::Position, position);
}

void rbx::part_t::set_rotation(const math::matrix3_t& rot) {
    memory->write<math::matrix3_t>(this->address + Offsets::BasePart::Rotation, rot);
}

math::vector3_t rbx::part_t::get_part_pos()
{
    math::vector3_t res{};

    auto primitive = this->get_primitive();

    if (!primitive.address)
        return res;

    res = memory->read<vector3_t>(primitive.address + Offsets::BasePart::Position);
    return res;
}
