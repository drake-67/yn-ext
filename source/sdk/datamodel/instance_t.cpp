#include "instance_t.hpp"
#include "../../cheat/initialise.hpp"
#include "../offsets.hpp"
#include "players.hpp"
#include "../../globals/globals.h"

std::string rbx::nameable_t::get_name() const {
	std::uint64_t ptr = memory->read<std::uint64_t>(this->address + Offsets::Instance::Name);
	if (ptr != 0) {
		return memory->read_string(ptr);
	}
	return "unknown";
}

std::uint64_t rbx::treeinterface_t::get_model_instance()
{
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);
	rbx::instance_t modelInstance = memory->read<rbx::instance_t>(self->address + Offsets::Player::ModelInstance);
	return std::uint64_t(modelInstance.address);
}

math::coordinate_frame_t rbx::treeinterface_t::get_cframe() {
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);
	return memory->read<math::coordinate_frame_t>(self->address + 0x130);
}

void rbx::treeinterface_t::write_cframe(math::coordinate_frame_t newCFrame)
{
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);
	auto primitive = memory->read< std::uint64_t >(self->address + Offsets::BasePart::Primitive);

	if (primitive) {
		int count = 0;
		while (true) {
			count += 1;
			memory->write<math::coordinate_frame_t>(primitive + Offsets::BasePart::Rotation, newCFrame);

			if (count == 250) {
				break;
			}
		}

		memory->write<math::coordinate_frame_t>(primitive + Offsets::BasePart::Rotation, newCFrame);
	}
}


void rbx::treeinterface_t::spectate(std::string stringhere) {
	rbx::instance_t workspace = globals::game::data_model.find_first_child_by_class("Workspace");
	rbx::instance_t camera = workspace.find_first_child("Camera");
	rbx::instance_t player = globals::game::players.find_first_child(stringhere);
	rbx::instance_t model = rbx::model_instance_t(player).get_model_instance();
	rbx::instance_t head = rbx::model_instance_t(model).find_first_child("Head");

	memory->write<std::uint64_t>(camera.address + Offsets::Camera::CameraSubject, head.address);
}

rbx::treeinterface_t rbx::treeinterface_t::unspecate() {
	rbx::treeinterface_t yes;

	rbx::instance_t workspace = globals::game::data_model.find_first_child_by_class("Workspace");
	rbx::instance_t camera = workspace.find_first_child("Camera");
	rbx::instance_t localplayer = globals::game::players.get_localpl();
	rbx::instance_t model = rbx::model_instance_t(localplayer).get_model_instance();
	rbx::instance_t humanoid = rbx::model_instance_t(model).find_first_child("Humanoid");

	memory->write<std::uint64_t>(camera.address + Offsets::Camera::CameraSubject, humanoid.address);
	return yes;
}

std::uint64_t rbx::nameable_t::get_adornee()
{
	return memory->read<std::uint64_t>(this->address + Offsets::Misc::Adornee);
}

std::string rbx::nameable_t::get_class_name() const
{
	std::uint64_t classAddress = memory->read<std::uint64_t>(this->address + Offsets::Instance::ClassDescriptor);
	std::uint64_t sizeAddress = classAddress + Offsets::Instance::ClassName;
	std::uint64_t classNameSize = memory->read<std::uint64_t>(sizeAddress);
	return memory->read_string(classNameSize);
}

std::uint64_t rbx::treeinterface_t::set_frame_position_x(uint64_t position) {
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);
	memory->write<uint64_t>(self->address + Offsets::silent::FramePositionX, position);
	return position;
}

std::uint64_t rbx::treeinterface_t::set_frame_position_y(uint64_t position) {
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);
	memory->write<uint64_t>(self->address + Offsets::silent::FramePositionY, position);
	return position;
}



std::uint64_t rbx::treeinterface_t::get_input_object(std::uint64_t base_address) {
	try {

		std::uint64_t target_address = base_address + Offsets::MouseService::InputObject;

		std::uint64_t input_object = memory->read<std::uint64_t>(target_address);

		if (input_object == 0) {
			std::cerr << "Error: Failed to read valid input object. Address may be invalid." << std::endl;
			return 0;
		}

		return input_object;
	}
	catch (const std::exception& e) {

		std::cerr << "Error: Exception caught in GetInputObject: " << e.what() << std::endl;
		return 0;
	}
	catch (...) {

		std::cerr << "Error: Unknown exception caught in GetInputObject" << std::endl;
		return 0;
	}
}


std::uint64_t rbx::treeinterface_t::get_current_input_object(std::uint64_t base_address) {

	std::uint64_t object_address = memory->read<std::uint64_t>(base_address + Offsets::MouseService::InputObject);

	return object_address;
}

std::uint64_t cached_input_object = 0;
void rbx::treeinterface_t::initialize_mouse_service(std::uint64_t address) {
	cached_input_object = get_current_input_object(address);

	if (cached_input_object && cached_input_object != 0xFFFFFFFFFFFFFFFF) {
		const char* base_pointer = reinterpret_cast<
			const char*> (cached_input_object);

		_mm_prefetch(base_pointer + 0xE4, _MM_HINT_T0);
		_mm_prefetch(base_pointer + 0xE4 + sizeof(math::vector2_t), _MM_HINT_T0);
	}
}

void rbx::treeinterface_t::write_mouse_position(std::uint64_t address, float x, float y) {
	std::uint64_t updated_input_object = get_current_input_object(address);

	if (updated_input_object != 0 && updated_input_object != 0xFFFFFFFFFFFFFFFF) {
		math::vector2_t new_position = { x, y };


		memory->write<math::vector2_t>(updated_input_object + 0xE4, new_position);
	}
}

std::vector<rbx::instance_t> rbx::treeinterface_t::get_children()
{
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);

	std::vector<instance_t> children;

	std::uint64_t start = memory->read<std::uint64_t>(self->address + Offsets::Instance::ChildrenStart);
	std::uint64_t end = memory->read<std::uint64_t>(start + Offsets::Instance::ChildrenEnd);
	for (auto instance = memory->read<std::uint64_t>(start); instance != end; instance += 0x10) {
		children.emplace_back(instance_t(memory->read<std::uint64_t>(instance)));
	}
	return children;
}

std::uint64_t rbx::treeinterface_t::find_first_child(const std::string& child_name)
{
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);

	std::vector<rbx::instance_t> children = this->get_children();
	for (const auto& child : children) {
		if (child.get_name() == child_name) {
			return child.address;
		}
	}

	return 0;
}


double rbx::treeinterface_t::get_double_value()
{
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);
	return memory->read<double>(self->address + Offsets::Misc::Value);
}

std::string rbx::treeinterface_t::text()
{
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);

	if (!self->address)
		return "???";
	return memory->read_string(self->address + Offsets::GuiObject::Text);
}

std::uint64_t rbx::treeinterface_t::find_first_child_by_class(const std::string& class_name)
{
	std::vector<rbx::instance_t> children = this->get_children();

	for (auto& object : children)
	{
		if (object.get_class_name() == class_name)
		{
			return object.address;
		}
	}
	return 0;
}

std::uint64_t rbx::treeinterface_t::get_parent()
{
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);

	instance_t parent = instance_t(memory->read<std::uint64_t>(self->address + Offsets::Instance::Parent));
	return parent.address;
}


std::uint32_t rbx::instance_t::get_color3() const {
	if (!this->address)
		return 0;

	uint32_t rawValue = memory->read<uint32_t>(this->address + Offsets::BasePart::Color3);
	uint32_t maskedValue = rawValue & 0x00FFFFFF;
	return maskedValue;
}

std::uint32_t rbx::instance_t::get_team() {
	if (!this->address)
		return 0;

	return memory->read<uint32_t>(this->address + Offsets::Player::Team);
}

std::uint64_t rbx::treeinterface_t::get_local_player()
{
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);

	instance_t localpalyer = instance_t(memory->read<std::uint64_t>(self->address + Offsets::Player::LocalPlayer));
	return localpalyer.address;
}

std::uint64_t rbx::treeinterface_t::get_userid()
{
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);

	return memory->read<std::uint64_t>(self->address + Offsets::Player::UserId);
}

template <typename T>
T rbx::value_holder_t<T>::get_value()
{
	return memory->read<T>(this->address + Offsets::Misc::Value);
}

template bool rbx::value_holder_t<bool>::get_value();
