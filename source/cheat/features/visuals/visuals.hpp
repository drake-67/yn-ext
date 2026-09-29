#pragma once
#include <vector>
#include "../../../sdk/datamodel/instance_t.hpp"
#include "../cache/cache.hpp"
#include "../../../cheat/drawing/imgui/imgui.h"
#include "../../../globals/globals.h"

namespace visuals_t {

	namespace parts {
		std::vector<const rbx::instance_t*> get_parts(const rbx::player_t& player);
	}



	void renderr();
}