#pragma once
#include <cstdint>
#include "../../utils/math/math.hpp"
#include "../../sdk/datamodel/instance_t.hpp"
namespace rbx {

	struct datamodel_t final : public instance_t
	{
		using instance_t::instance_t;
		std::uint64_t get_place_id();
		std::uint64_t get_game_id();
		std::uint64_t get_creator_id();
		std::uint64_t get_server_ip() const;
	};


}
