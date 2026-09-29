#include "../../../globals/globals.h"
#include "../../../sdk/datamodel/part_t.hpp"
#include <Windows.h>
#include <thread>
#include <chrono>
#include <limits>
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>
#include "silentaim.hpp"

namespace silent {
    silent_t instance;

    std::string current_target_name = "";

    silent_t::silent_t()
        : enabled_(true), key_pressed_last_frame_(false) {
    }

    void silent_t::initialize() {
        std::cout << "[Silent] Initialized." << std::endl;
    }

    bool silent_t::is_key_pressed(int virtual_key) {
        return (GetAsyncKeyState(virtual_key) & 0x8000) != 0;
    }

    float silent_t::calculate_distance_2d(const math::vector2_t& pos1, const math::vector2_t& pos2) {
        float dx = pos1.x - pos2.x;
        float dy = pos1.y - pos2.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    bool silent_t::world_to_screen(const math::vector3_t& world_pos, math::vector2_t& screen_pos) {
        if (globals::game::visual_engine.address == 0) {
            return false;
        }

        rbx::visualengine_t visualengine = globals::game::visual_engine;
        math::vector2_t dimensions = visualengine.get_dimensions();
        math::matrix4_t viewMatrix = visualengine.get_viewmatrix();

        math::vector2_t result = visualengine.world_to_screen(world_pos, dimensions, viewMatrix);

        if (result.x < 0.f || result.y < 0.f || result.x > dimensions.x || result.y > dimensions.y) {
            return false;
        }

        screen_pos = result;
        return true;
    }

    bool silent_t::passes_checks(const auto& player) {
        if (player.localplayer) {
            return false;
        }

        if (!player.character.address) {
            return false;
        }

        if (!player.head.address) {
            return false;
        }

        if (globals::silent::health_check && player.humanoid.address) {
            rbx::instance_t humanoid(player.humanoid.address);
        }

        if (globals::silent::team_check && player.team.address) {
            if (player.team.address == globals::game::local_player.team.address) {
                return false;
            }
        }

        return true;
    }

    bool silent_t::is_within_fov(const math::vector2_t& screen_pos, const math::vector2_t& cursor_pos) {
        if (!globals::silent::use_fov) {
            return true;
        }

        float distance = calculate_distance_2d(screen_pos, cursor_pos);
        return distance <= globals::silent::fov;
    }

    rbx::instance_t silent_t::get_closest_part_to_cursor(const auto& player, const math::vector2_t& cursor_pos) {
        std::vector<rbx::instance_t> available_parts;

        if (player.head.address) available_parts.push_back(player.head);
        if (player.torso.address) available_parts.push_back(player.torso);
        if (player.hrp.address) available_parts.push_back(player.hrp);

        if (available_parts.empty()) {
            return rbx::instance_t(0);
        }

        rbx::visualengine_t visualengine = globals::game::visual_engine;
        math::vector2_t dimensions = visualengine.get_dimensions();
        math::matrix4_t viewMatrix = visualengine.get_viewmatrix();

        float nearest_dist = FLT_MAX;
        rbx::instance_t closest_part(0);

        for (const auto& part : available_parts) {
            rbx::part_t part_obj(part.address);
            math::vector3_t part_pos = part_obj.get_part_pos();
            math::vector2_t screen_pos = visualengine.world_to_screen(part_pos, dimensions, viewMatrix);

            if (screen_pos.x >= 0.f && screen_pos.y >= 0.f && screen_pos.x <= dimensions.x && screen_pos.y <= dimensions.y) {
                float dist = calculate_distance_2d(screen_pos, cursor_pos);
                if (dist < nearest_dist) {
                    nearest_dist = dist;
                    closest_part = part;
                }
            }
        }

        return closest_part;
    }

    math::vector3_t silent_t::get_hitpart_position(const auto& player) {
        static std::random_device rd;
        static std::mt19937 gen(rd());

        std::vector<math::vector3_t> available_parts;

        if (player.head.address) {
            rbx::part_t part(player.head.address);
            available_parts.push_back(part.get_part_pos());
        }
        if (player.torso.address) {
            rbx::part_t part(player.torso.address);
            available_parts.push_back(part.get_part_pos());
        }
        if (player.hrp.address) {
            rbx::part_t part(player.hrp.address);
            available_parts.push_back(part.get_part_pos());
        }

        if (available_parts.empty()) {
            return math::vector3_t{ 0, 0, 0 };
        }

        math::vector3_t target_pos;

        switch (globals::silent::hitpart_mode) {
        case 0:
            target_pos = available_parts[0];
            break;

        case 1:
            target_pos = available_parts.size() > 1 ? available_parts[1] : available_parts[0];
            break;

        case 2:
            target_pos = available_parts.size() > 2 ? available_parts[2] : available_parts[0];
            break;

        case 3:
        {
            POINT cursor_pos;
            if (!GetCursorPos(&cursor_pos)) {
                target_pos = available_parts[0];
                break;
            }
            ScreenToClient(FindWindowA(0, "Roblox"), &cursor_pos);
            math::vector2_t cursor_position(static_cast<float>(cursor_pos.x), static_cast<float>(cursor_pos.y));

            rbx::instance_t closest_part_inst = get_closest_part_to_cursor(player, cursor_position);
            if (closest_part_inst.address) {
                rbx::part_t part(closest_part_inst.address);
                target_pos = part.get_part_pos();
            }
            else {
                target_pos = available_parts[0];
            }
            break;
        }

        case 4:
        {
            std::uniform_int_distribution<> dis(0, available_parts.size() - 1);
            target_pos = available_parts[dis(gen)];
            break;
        }

        default:
            target_pos = available_parts[0];
            break;
        }

        if (globals::silent::prediction) {
            rbx::instance_t hitpart_instance;
            switch (globals::silent::hitpart_mode) {
            case 0:
                hitpart_instance = player.head;
                break;
            case 1:
                hitpart_instance = player.torso;
                break;
            case 2:
                hitpart_instance = player.hrp;
                break;
            default:
                hitpart_instance = player.head;
                break;
            }

            if (hitpart_instance.address) {
                rbx::part_t part(hitpart_instance.address);
                math::vector3_t velocity = part.get_velocity();
                target_pos = target_pos + (velocity / math::vector3_t{
                    globals::silent::prediction_x,
                    globals::silent::prediction_y,
                    globals::silent::prediction_z
                    });
            }
        }

        return target_pos;
    }

    silent_target_t silent_t::find_best_target() {
        silent_target_t best_target;

        if (globals::game::visual_engine.address == 0) {
            return best_target;
        }

        if (globals::silent::sticky && !current_target_name.empty()) {
            for (const auto& player : globals::game::player_cache) {
                if (player.Name == current_target_name && passes_checks(player)) {
                    math::vector3_t target_position = get_hitpart_position(player);

                    if (std::isnan(target_position.x) || std::isnan(target_position.y) || std::isnan(target_position.z)) {
                        continue;
                    }

                    math::vector2_t screen_pos;
                    if (world_to_screen(target_position, screen_pos)) {
                        best_target.world_position = target_position;
                        best_target.screen_position = screen_pos;
                        best_target.distance = player.distance;
                        best_target.player_name = player.Name;
                        best_target.valid = true;
                        return best_target;
                    }
                }
            }
            current_target_name = "";
        }

        POINT cursor_point;
        if (!GetCursorPos(&cursor_point)) {
            return best_target;
        }
        ScreenToClient(FindWindowA(0, "Roblox"), &cursor_point);
        math::vector2_t cursor_position(static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y));

        float closest_distance = FLT_MAX;

        for (const auto& player : globals::game::player_cache) {
            if (!passes_checks(player)) {
                continue;
            }

            math::vector3_t target_position = get_hitpart_position(player);

            if (std::isnan(target_position.x) || std::isnan(target_position.y) || std::isnan(target_position.z)) {
                continue;
            }

            math::vector2_t screen_pos;
            if (!world_to_screen(target_position, screen_pos)) {
                continue;
            }

            if (!is_within_fov(screen_pos, cursor_position)) {
                continue;
            }

            float distance_to_cursor = calculate_distance_2d(screen_pos, cursor_position);

            if (distance_to_cursor < closest_distance) {
                closest_distance = distance_to_cursor;
                best_target.world_position = target_position;
                best_target.screen_position = screen_pos;
                best_target.distance = player.distance;
                best_target.player_name = player.Name;
                best_target.valid = true;
            }
        }

        if (best_target.valid) {
            if (current_target_name.empty() || !globals::silent::sticky) {
                current_target_name = best_target.player_name;
            }
        }

        return best_target;
    }



    void silent_t::execute_silent_aim(const silent_target_t& target, const math::vector2_t& cursor_pos) {
        if (!target.valid) {
            return;
        }

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(1, 100);

        if (dist(gen) > globals::silent::hit_chance) {
            return;
        }

        rbx::visualengine_t visualengine = globals::game::visual_engine;
        math::vector2_t screen_size = visualengine.get_dimensions();

        uint64_t new_position_x = static_cast<uint64_t>(target.screen_position.x);
        uint64_t new_position_y = static_cast<uint64_t>(target.screen_position.y);

        if (globals::misc::aim.address) {
            new_position_y = static_cast<uint64_t>(
                screen_size.y - std::abs(screen_size.y - target.screen_position.y) - 58
                );
        }

        if (globals::silent::use_frame_position && globals::misc::aim.address) {
            globals::misc::aim.set_frame_position_x(new_position_x);
            globals::misc::aim.set_frame_position_y(new_position_y);
        }

        if (globals::silent::use_mouseservice && globals::misc::mouseserivce.address) {
            globals::misc::mouseserivce.initialize_mouse_service(globals::misc::mouseserivce.address);
            globals::misc::mouseserivce.write_mouse_position(
                globals::misc::mouseserivce.address,
                target.screen_position.x,
                target.screen_position.y
            );
        }
    }

    void silent_t::update() {
        if (!enabled_) {
            return;
        }


        if (!globals::aim::aimbot_bind.enabled || !globals::silent::enabled); {
            current_target_name = "";
            return;
        }



        POINT cursor_point;
        if (!GetCursorPos(&cursor_point)) {
            return;
        }
        ScreenToClient(FindWindowA(0, "Roblox"), &cursor_point);
        math::vector2_t cursor_pos(static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y));

        silent_target_t target = find_best_target();

        if (target.valid) {
            execute_silent_aim(target, cursor_pos);
        }
        else {
            current_target_name = "";
        }
    }

    void start_silent_thread() {
        std::thread silent_thread([]() {
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
            instance.initialize();

            HWND rblx = FindWindowA(nullptr, "Roblox");

            while (true) {
                try {
                    if (GetForegroundWindow() != rblx) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        continue;
                    }

                    instance.update();
                }
                catch (const std::exception& e) {
                    std::cout << "[Silent] Exception: " << e.what() << std::endl;
                }
                catch (...) {
                    std::cout << "[Silent] Unknown exception occurred." << std::endl;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            });

        silent_thread.detach();
        std::cout << "[Silent] Thread started." << std::endl;
    }
}