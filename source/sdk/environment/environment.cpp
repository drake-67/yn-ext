#include "environment.hpp"

#include "environment.hpp"
#include "../../cheat/initialise.hpp"
#include "../offsets.hpp"

// Ambient
math::vector3_t rbx::environment_t::get_ambient() const
{
	return memory->read<vector3_t>(this->address + Offsets::Lighting::Ambient);
}

void rbx::environment_t::set_ambient(const vector3_t& ambient)
{
	memory->write<vector3_t>(this->address + Offsets::Lighting::Ambient, ambient);
}

// Brightness
float rbx::environment_t::get_brightness() const
{
	return memory->read<float>(this->address + Offsets::Lighting::Brightness);
}

void rbx::environment_t::set_brightness(float brightness)
{
	memory->write<float>(this->address + Offsets::Lighting::Brightness, brightness);
}

// ClockTime
float rbx::environment_t::get_clock_time() const
{
	return memory->read<float>(this->address + Offsets::Lighting::ClockTime);
}

void rbx::environment_t::set_clock_time(float clock_time)
{
	memory->write<float>(this->address + Offsets::Lighting::ClockTime, clock_time);
}

// ColorShift_Bottom
math::vector3_t rbx::environment_t::get_color_shift_bottom() const
{
	return memory->read<vector3_t>(this->address + Offsets::Lighting::ColorShift_Bottom);
}

void rbx::environment_t::set_color_shift_bottom(const vector3_t& color)
{
	memory->write<vector3_t>(this->address + Offsets::Lighting::ColorShift_Bottom, color);
}

// ColorShift_Top
math::vector3_t rbx::environment_t::get_color_shift_top() const
{
	return memory->read<vector3_t>(this->address + Offsets::Lighting::ColorShift_Top);
}

void rbx::environment_t::set_color_shift_top(const vector3_t& color)
{
	memory->write<vector3_t>(this->address + Offsets::Lighting::ColorShift_Top, color);
}

// ExposureCompensation
float rbx::environment_t::get_exposure_compensation() const
{
	return memory->read<float>(this->address + Offsets::Lighting::ExposureCompensation);
}

void rbx::environment_t::set_exposure_compensation(float exposure)
{
	memory->write<float>(this->address + Offsets::Lighting::ExposureCompensation, exposure);
}

// FogColor
math::vector3_t rbx::environment_t::get_fog_color() const
{
	return memory->read<vector3_t>(this->address + Offsets::Lighting::FogColor);
}

void rbx::environment_t::set_fog_color(const vector3_t& color)
{
	memory->write<vector3_t>(this->address + Offsets::Lighting::FogColor, color);
}

// FogEnd
float rbx::environment_t::get_fog_end() const
{
	return memory->read<float>(this->address + Offsets::Lighting::FogEnd);
}

void rbx::environment_t::set_fog_end(float fog_end)
{
	memory->write<float>(this->address + Offsets::Lighting::FogEnd, fog_end);
}

// FogStart
float rbx::environment_t::get_fog_start() const
{
	return memory->read<float>(this->address + Offsets::Lighting::FogStart);
}

void rbx::environment_t::set_fog_start(float fog_start)
{
	memory->write<float>(this->address + Offsets::Lighting::FogStart, fog_start);
}

// GeographicLatitude
float rbx::environment_t::get_geographic_latitude() const
{
	return memory->read<float>(this->address + Offsets::Lighting::GeographicLatitude);
}

void rbx::environment_t::set_geographic_latitude(float latitude)
{
	memory->write<float>(this->address + Offsets::Lighting::GeographicLatitude, latitude);
}

// OutdoorAmbient
math::vector3_t rbx::environment_t::get_outdoor_ambient() const
{
	return memory->read<vector3_t>(this->address + Offsets::Lighting::OutdoorAmbient);
}

void rbx::environment_t::set_outdoor_ambient(const vector3_t& ambient)
{
	memory->write<vector3_t>(this->address + Offsets::Lighting::OutdoorAmbient, ambient);
}