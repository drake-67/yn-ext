#pragma once
#include <cstdint>
#include "../datamodel/instance_t.hpp"

namespace rbx {
	struct environment_t final : public instance_t
	{
		using instance_t::instance_t;

		// Ambient
		vector3_t get_ambient() const;
		void set_ambient(const vector3_t& ambient);

		// Brightness
		float get_brightness() const;
		void set_brightness(float brightness);

		// ClockTime
		float get_clock_time() const;
		void set_clock_time(float clock_time);

		// ColorShift_Bottom
		vector3_t get_color_shift_bottom() const;
		void set_color_shift_bottom(const vector3_t& color);

		// ColorShift_Top
		vector3_t get_color_shift_top() const;
		void set_color_shift_top(const vector3_t& color);

		// ExposureCompensation
		float get_exposure_compensation() const;
		void set_exposure_compensation(float exposure);

		// FogColor
		vector3_t get_fog_color() const;
		void set_fog_color(const vector3_t& color);

		// FogEnd
		float get_fog_end() const;
		void set_fog_end(float fog_end);

		// FogStart
		float get_fog_start() const;
		void set_fog_start(float fog_start);

		// GeographicLatitude
		float get_geographic_latitude() const;
		void set_geographic_latitude(float latitude);

		// OutdoorAmbient
		vector3_t get_outdoor_ambient() const;
		void set_outdoor_ambient(const vector3_t& ambient);
	};
} // namespace rbx