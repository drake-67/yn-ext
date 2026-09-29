// finding stuff via roblox 
#include "initialise.hpp"
#include "../sdk/offsets.hpp"
#include "../globals/globals.h"
#include "overlay/overlay.hpp"


void initialise::setup() {
    auto module = memory->get_module_address();
    auto fake_datamodel = memory->read<std::uint64_t>(module + Offsets::FakeDataModel::Pointer);
    globals::game::data_model = memory->read<std::uint64_t>(fake_datamodel + Offsets::FakeDataModel::RealDataModel);
    globals::game::visual_engine = rbx::visualengine_t(memory->read<std::uint64_t>(module + Offsets::VisualEngine::Pointer));

    globals::game::players = globals::game::data_model.find_first_child("Players");
    globals::game::workspace = globals::game::data_model.find_first_child("Workspace");
    globals::game::lightning = globals::game::data_model.find_first_child("lighting");
    globals::misc::mouseserivce = globals::game::data_model.find_first_child_by_class("MouseService");


}


void renderer_t::start() {
    render_t overlay;
    if (!overlay.create_window()) {
        std::cout << "Failed to create window." << std::endl;
    }
    if (!overlay.create_device()) {
        std::cout << "Failed to create DirectX device." << std::endl;
    }
    if (!overlay.create_imgui()) {
        std::cout << "Failed to initialize ImGui." << std::endl;

    }

    overlay.start_render();
}

