#pragma once
#include "../../cheat/drawing/imgui/imgui.h"

namespace world {

	void initialize();
	void shutdown();

	void start_snow_thread();
	void stop_snow_thread();

	void render_snow(ImDrawList* draw_list);
	void set_snow_particle_count(int count);

} // namespace world