#pragma once
#include <cstdint>
#include "../../utils/math/math.hpp"

namespace rbx {
	struct instance_t;

	struct addressable_t
	{
		std::uint64_t address;

		addressable_t() : address(0) {}
		addressable_t(std::uint64_t address) : address(address) {}
	};

	struct treeinterface_t
	{
		std::uint64_t get_model_instance();
		math::coordinate_frame_t get_cframe();
		void write_cframe(math::coordinate_frame_t newCFrame);
		void spectate(std::string stringhere);
		rbx::treeinterface_t unspecate();
		std::uint64_t set_frame_position_x(uint64_t position);
		std::uint64_t set_frame_position_y(uint64_t position);
		std::uint64_t get_input_object(std::uint64_t base_address);
		std::uint64_t get_current_input_object(std::uint64_t base_address);
		void initialize_mouse_service(std::uint64_t address);
		void write_mouse_position(std::uint64_t address, float x, float y);
		std::vector<instance_t> get_children();
		std::uint64_t find_first_child(const std::string& child_name);
		double get_double_value();
		std::string text();
		std::uint64_t find_first_child_by_class(const std::string& class_name);
		std::uint64_t get_parent();
		std::uint64_t get_local_player();
		std::uint64_t get_userid();
	};
	


	template <typename T>
	struct value_holder_t : public addressable_t
	{
		using addressable_t::addressable_t;

		T get_value();
	};

	struct model_instance_t final : public addressable_t, public treeinterface_t
	{

	};

	struct nameable_t : addressable_t
	{
		using addressable_t::addressable_t;
		std::string get_name() const;
		std::uint64_t get_adornee();
		std::string get_class_name() const;

	};



	struct instance_t : public nameable_t, public treeinterface_t
	{
		using nameable_t::nameable_t;
		std::uint32_t get_color3() const;
		std::uint32_t get_team();


	};


} // namespace rbx
