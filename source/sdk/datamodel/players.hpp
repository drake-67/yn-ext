#pragma once
#include <cstdint>
#include "instance_t.hpp"
namespace rbx {

	struct players_t final : public instance_t
	{
		using instance_t::instance_t;
		std::uint64_t get_userid();
		std::uint64_t get_team();
		players_t get_localpl();
	};


} // namespace rbx