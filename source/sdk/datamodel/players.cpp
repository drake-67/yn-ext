#include "players.hpp"
#include "../../cheat/initialise.hpp"
#include "../offsets.hpp"

	// << localplayer, userid etc. << //


std::uint64_t rbx::players_t::get_userid()
{
	std::uint64_t userId = memory->read<std::uint64_t>(this->address + Offsets::Player::UserId);
	return userId;
}

std::uint64_t rbx::players_t::get_team() 
{
	return memory->read<uint64_t>(this->address + Offsets::Player::Team);
}

rbx::players_t rbx::players_t::get_localpl()
{
	std::uint64_t local_address = memory->read<std::uint64_t>(this->address + Offsets::Player::LocalPlayer);
	return rbx::players_t{ local_address };
}
