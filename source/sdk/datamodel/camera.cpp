#include "camera.hpp"
#include "../../cheat/initialise.hpp"
#include "../offsets.hpp"


math::vector3_t rbx::camera_t::get_camera_position()
{
	return memory->read<math::vector3_t>(this->address + Offsets::Camera::Position);
}


void rbx::camera_t::set_camera_rotation(const math::matrix3_t& rot)
{
	memory->write<math::matrix3_t>(this->address + Offsets::Camera::Rotation, rot);
}

math::matrix3_t rbx::camera_t::get_camera_rotation()
{
	return memory->read<math::matrix3_t>(this->address + Offsets::Camera::Rotation);
}

void rbx::camera_t::set_camera_position(const math::vector3_t& pos)
{
	memory->write<math::vector3_t>(this->address + Offsets::Camera::Position, pos);
}