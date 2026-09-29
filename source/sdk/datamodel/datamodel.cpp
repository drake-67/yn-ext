#include "datamodel.hpp"
#include "../../cheat/initialise.hpp"
#include "../offsets.hpp"


std::uint64_t rbx::datamodel_t::get_place_id()
{
	return memory->read<uint64_t>(this->address + Offsets::DataModel::PlaceId);
}

std::uint64_t rbx::datamodel_t::get_game_id()
{
	return memory->read<uint64_t>(this->address + Offsets::DataModel::GameId);
}

std::uint64_t rbx::datamodel_t::get_creator_id()
{
	return memory->read<uint64_t>(this->address + Offsets::DataModel::CreatorId);
}

std::uint64_t rbx::datamodel_t::get_server_ip() const
{
	return memory->read<uint64_t>(this->address + Offsets::DataModel::ServerIP);
}


