#include "Windows.h"
#include <memory.h>
#include "utils/output/output.hpp"
#include <thread>
#include <atomic>
#include "cheat/initialise.hpp"
#include "sdk/offsets.hpp"
#include "globals/globals.h"
#include "sdk/datamodel/datamodel.hpp"
#include "sdk/datamodel/instance_t.hpp"
#include "cheat/overlay/overlay.hpp"
#include "cheat/features/wallcheck/wallcheck.hpp"
#include "sdk/world/world_handler.hpp"
#include "cheat/features/aimbot/aimbot.hpp"
#include "cheat/features/rage/rage.hpp"
#include "cheat/features/wallcheck/wallcheck.hpp"
#include "cheat/features/silentaim/silentaim.hpp"

static std::atomic<bool> process_active{ false };
static std::atomic<bool> should_reinitialize{ false };



static void wait_for_process(const char* process_name) {
    while (memory->find_process_id(process_name) == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    memory->attach_to_process(process_name);
    memory->find_module_address(process_name);
}

static bool is_process_running(const char* process_name) {
    return memory->find_process_id(process_name) != 0;
}

static void process_monitor_thread(const char* process_name) {
    while (true) {
        bool running = is_process_running(process_name);
        if (!running && process_active.load()) {
            process_active.store(false);
        }
        else if (running && !process_active.load()) {
            process_active.store(true);
            should_reinitialize.store(true);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

static void initialize_game() {
    try {
        initialise::setup();

        if (globals::game::data_model.address != 0) {
            std::uint64_t players_addr = globals::game::data_model.find_first_child_by_class("Players");
            if (players_addr != 0) {
                globals::game::players = rbx::players_t(players_addr);
            }

            std::uint64_t workspace_addr = globals::game::data_model.find_first_child_by_class("Workspace");
            if (workspace_addr != 0) {
                globals::game::workspace = rbx::datamodel_t(workspace_addr);
            }
        }
    }
    catch (...) {}
}

auto main() -> std::int32_t {
    logger::setup();
    SetConsoleTitle("");

    const char* process_name = "RobloxPlayerBeta.exe";

    wait_for_process(process_name);
    process_active.store(true);

    std::thread(process_monitor_thread, process_name).detach();

    initialize_game();

    logger::print<logger::level::debug>("DataModel: 0x%llx", globals::game::data_model.address);
    logger::print<logger::level::debug>("Players: 0x%llx", globals::game::players.address);
    logger::print<logger::level::debug>("Workspace: 0x%llx", globals::game::workspace.address);
    logger::print<logger::level::debug>("VisualEngine: 0x%llx", globals::game::visual_engine.address);
    logger::print<logger::level::debug>("Lightning: 0x%llx", globals::game::lightning.address);

    std::thread(player_cache::hook_cache).detach();
     std::thread(rage::start_rage_thread).detach();
    std::thread(aimbot::initialize).detach();
    std::thread(silent::start_silent_thread).detach();

    renderer_t::start();

    while (true) {
        if (should_reinitialize.exchange(false)) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            memory->attach_to_process(process_name);
            memory->find_module_address(process_name);
            initialize_game();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return EXIT_SUCCESS;
}