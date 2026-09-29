#include "cache.hpp"
#include "../../../globals/globals.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <thread>
#include <chrono>
#include <cmath>
#include <atomic>
#include <mutex>
#include <iostream>
#include "../../../sdk/datamodel/part_t.hpp"
#include "../../initialise.hpp"
#include "../../../sdk/offsets.hpp"

namespace {
    struct part_mapping {
        std::string_view name;
        rbx::instance_t rbx::player_t::* member;
    };

    constexpr part_mapping part_mappings[] = {
        {"Humanoid", &rbx::player_t::humanoid},
        {"HumanoidRootPart", &rbx::player_t::hrp},
        {"Head", &rbx::player_t::head},
        {"Torso", &rbx::player_t::torso},
        {"UpperTorso", &rbx::player_t::uppertorso},
        {"LowerTorso", &rbx::player_t::lowertorso},
        {"Left Arm", &rbx::player_t::leftarm},
        {"Right Arm", &rbx::player_t::rightarm},
        {"Left Leg", &rbx::player_t::leftleg},
        {"Right Leg", &rbx::player_t::rightleg},
        {"LeftUpperLeg", &rbx::player_t::leftupperleg},
        {"RightUpperLeg", &rbx::player_t::rightupperleg},
        {"LeftLowerLeg", &rbx::player_t::leftlowerleg},
        {"RightLowerLeg", &rbx::player_t::rightlowerleg},
        {"LeftFoot", &rbx::player_t::leftfoot},
        {"RightFoot", &rbx::player_t::rightfoot},
        {"LeftHand", &rbx::player_t::lefthand},
        {"RightHand", &rbx::player_t::righthand},
        {"LeftUpperArm", &rbx::player_t::leftupperarm},
        {"RightUpperArm", &rbx::player_t::rightupperarm},
        {"LeftLowerArm", &rbx::player_t::leftlowerarm},
        {"RightLowerArm", &rbx::player_t::rightlowerarm}
    };

    std::unordered_map<std::string, rbx::instance_t rbx::player_t::*> create_part_lookup() {
        std::unordered_map<std::string, rbx::instance_t rbx::player_t::*> map;
        map.reserve(sizeof(part_mappings) / sizeof(part_mappings[0]));
        for (const auto& mapping : part_mappings) {
            map.emplace(mapping.name, mapping.member);
        }
        return map;
    }

    const auto part_lookup = create_part_lookup();
    std::atomic<bool> references_updated{ false };
    std::atomic<std::uint64_t> current_game_id{ 0 };
    std::mutex cache_mutex;

    inline float calculate_distance(const math::vector3_t& p1, const math::vector3_t& p2) {
        float dx = p1.x - p2.x;
        float dy = p1.y - p2.y;
        float dz = p1.z - p2.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    bool is_valid_position(const math::vector3_t& pos) {
        return !std::isnan(pos.x) && !std::isnan(pos.y) && !std::isnan(pos.z);
    }

    bool is_same_team(rbx::instance_t player1, rbx::instance_t player2) {
        if (!globals::gamesupport::team_check) {
            return false;
        }

        std::uint64_t team1_addr = player1.get_team();
        std::uint64_t team2_addr = player2.get_team();

        if (team1_addr == 0 || team2_addr == 0) {
            return false;
        }

        return team1_addr == team2_addr;
    }

    rbx::instance_t get_phantom_forces_team(rbx::instance_t PlayerModel) {
        std::uint64_t parent_addr = PlayerModel.get_parent();
        if (PlayerModel.address && parent_addr != 0) {
            std::uint64_t folder_addr = PlayerModel.find_first_child_by_class("Folder");
            if (folder_addr != 0) {
                rbx::instance_t Folder(folder_addr);
                for (auto& child : Folder.get_children()) {
                    if (child.get_color3() == (std::uint32_t)0x5a4b36) {
                        std::uint64_t teams_addr = globals::game::data_model.find_first_child_by_class("Teams");
                        if (teams_addr != 0) {
                            rbx::instance_t teams(teams_addr);
                            return teams.find_first_child("Phantoms");
                        }
                    }
                }
            }
        }
        std::uint64_t teams_addr = globals::game::data_model.find_first_child_by_class("Teams");
        if (teams_addr != 0) {
            rbx::instance_t teams(teams_addr);
            return teams.find_first_child("Ghosts");
        }
        return rbx::instance_t(0);
    }

    std::vector<rbx::instance_t> get_bb_players() {
        std::vector<rbx::instance_t> target_players;

        std::uint64_t workspace_addr = globals::game::workspace.address;
        if (workspace_addr == 0) return target_players;

        rbx::instance_t workspace_inst(workspace_addr);
        std::uint64_t characters_addr = workspace_inst.find_first_child("Characters");
        if (characters_addr != 0) {
            rbx::instance_t characters(characters_addr);
            return characters.get_children();
        }
        return target_players;
    }

    std::vector<rbx::instance_t> get_pf_players() {
        std::vector<rbx::instance_t> target_players;

        std::uint64_t workspace_addr = globals::game::workspace.address;
        if (workspace_addr == 0) return target_players;

        rbx::instance_t workspace_inst(workspace_addr);
        std::uint64_t teams_folder_addr = workspace_inst.find_first_child("Players");
        if (teams_folder_addr == 0) return target_players;

        rbx::instance_t teams_folder(teams_folder_addr);
        auto teams = teams_folder.get_children();
        if (teams.size() != 2) return target_players;

        if (!globals::gamesupport::team_check) {
            for (auto& team : teams) {
                auto team_players = team.get_children();
                for (auto& player : team_players) {
                    if (player.get_class_name() == "Model") {
                        target_players.push_back(player);
                    }
                }
            }
        }
        else {
            std::uint64_t local_player_addr = globals::game::players.get_local_player();
            if (local_player_addr == 0) return target_players;

            rbx::instance_t local_player(local_player_addr);
            rbx::instance_t local_char = local_player.get_model_instance();
            rbx::instance_t local_team = get_phantom_forces_team(local_char);

            for (auto& team : teams) {
                auto team_players = team.get_children();
                for (auto& player : team_players) {
                    if (player.get_class_name() == "Model") {
                        rbx::instance_t player_team = get_phantom_forces_team(player);
                        if (player_team.address != local_team.address) {
                            target_players.push_back(player);
                        }
                    }
                }
            }
        }

        return target_players;
    }

    void cache_bad_business_players(std::vector<rbx::player_t>& players, const math::vector3_t& local_position, const std::string& local_name) {
        static auto last_debug = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        bool should_debug = std::chrono::duration_cast<std::chrono::seconds>(now - last_debug).count() >= 5;

        auto player_models = get_bb_players();

        if (should_debug) {
            std::cout << "[BB] Found " << player_models.size() << " players" << std::endl;
        }

        std::uint64_t local_player_addr = globals::game::players.get_local_player();
        rbx::instance_t local_player_inst(local_player_addr);

        int total_players = 0;
        for (auto& player_model : player_models) {
            if (player_model.get_class_name() != "Model") continue;

            rbx::player_t player{};
            player.character = player_model;
            player.Name = player_model.get_name();
            player.bones.clear();

            std::uint64_t body_addr = player_model.find_first_child("Body");
            if (body_addr == 0) continue;

            rbx::instance_t body(body_addr);
            auto body_children = body.get_children();

            for (const auto& part : body_children) {
                std::string part_class = part.get_class_name();
                if (part_class == "Part" || part_class == "MeshPart") {
                    player.bones.push_back(part);

                    std::string part_name = part.get_name();
                    if (part_name == "Head") {
                        player.head = part;
                    }
                    else if (part_name == "Abdomen") {
                        player.hrp = part;
                    }
                }
            }

            std::uint64_t health_addr = player_model.find_first_child("Health");
            if (health_addr != 0) {
                rbx::instance_t health_obj(health_addr);
                player.Health = health_obj.get_double_value();

                std::uint64_t max_health_addr = health_obj.find_first_child("MaxHealth");
                if (max_health_addr != 0) {
                    rbx::instance_t max_health(max_health_addr);
                    player.MaxHealth = max_health.get_double_value();
                }
            }

            if (player.hrp.address != 0) {
                rbx::part_t hrp_part(player.hrp.address);
                math::vector3_t player_position = hrp_part.get_part_pos();
                if (is_valid_position(player_position) && is_valid_position(local_position)) {
                    player.distance = calculate_distance(local_position, player_position);
                }
            }

            std::uint64_t clothes_addr = player_model.find_first_child("Clothes");
            if (clothes_addr == 0) {
                globals::game::local_player = player;
            }
            else if (player.Name != local_name && !player.Name.empty()) {
                if (globals::gamesupport::team_check && local_player_addr != 0) {
                    std::uint64_t players_addr = globals::game::players.address;
                    if (players_addr != 0) {
                        rbx::instance_t players_obj(players_addr);
                        rbx::instance_t target_player_inst = players_obj.find_first_child(player.Name);

                        if (target_player_inst.address != 0) {
                            if (is_same_team(local_player_inst, target_player_inst)) {
                                continue; 
                            }
                        }
                    }
                }

                players.push_back(std::move(player));
                total_players++;
            }
        }

        if (should_debug) {
            std::cout << "[BB] Total players cached: " << total_players << std::endl;
            last_debug = now;
        }
    }

    void cache_phantom_forces_players(std::vector<rbx::player_t>& players, const math::vector3_t& local_position, const std::string& local_name) {
        static auto last_debug = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        bool should_debug = std::chrono::duration_cast<std::chrono::seconds>(now - last_debug).count() >= 5;

        auto player_models = get_pf_players();

        if (should_debug) {
            std::cout << "[PF] Found " << player_models.size() << " players" << std::endl;
        }

        int total_players = 0;
        for (auto& player_model : player_models) {
            if (player_model.get_class_name() != "Model") continue;

            rbx::player_t player{};
            player.character = player_model;
            player.team = get_phantom_forces_team(player_model);
            player.bones.clear();

            auto model_children = player_model.get_children();

            for (auto& child : model_children) {
                std::string class_name = child.get_class_name();

                if (class_name == "Folder") {
                    auto folder_children = child.get_children();
                    for (const auto& part : folder_children) {
                        std::string part_class = part.get_class_name();
                        if (part_class == "Part" || part_class == "MeshPart") {
                            player.bones.push_back(part);
                        }
                    }
                }
                else if (class_name == "Part" || class_name == "MeshPart") {
                    player.bones.push_back(child);
                }
            }

            if (!player.bones.empty()) {
                player.hrp = player.bones[0];
            }

            for (auto& part : player.bones) {
                for (auto& child : part.get_children()) {
                    if (child.get_class_name() == "BillboardGui") {
                        player.head = part;
                        std::uint64_t textlabel_addr = child.find_first_child_by_class("TextLabel");
                        if (textlabel_addr != 0) {
                            rbx::instance_t textlabel(textlabel_addr);
                            player.Name = textlabel.text();
                        }
                        break;
                    }
                }
                if (player.head.address != 0) break;
            }

            if (player.hrp.address != 0) {
                rbx::part_t hrp_part(player.hrp.address);
                math::vector3_t player_position = hrp_part.get_part_pos();
                if (is_valid_position(player_position) && is_valid_position(local_position)) {
                    player.distance = calculate_distance(local_position, player_position);
                }
            }

            if (player.Name != local_name && !player.Name.empty()) {
                players.push_back(std::move(player));
                total_players++;
            }
        }

        if (should_debug) {
            std::cout << "[PF] Total players cached: " << total_players << std::endl;
            last_debug = now;
        }
    }

    void cache_player_data(rbx::player_t& player, const math::vector3_t& local_position, bool is_local) {
        if (player.character.address == 0) return;

        auto children = player.character.get_children();
        for (const auto& part : children) {
            auto it = part_lookup.find(part.get_name());
            if (it != part_lookup.end()) {
                player.*(it->second) = part;
            }
        }

        std::uint64_t tool_address = player.character.find_first_child_by_class("Tool");
        if (tool_address != 0) {
            player.tool = rbx::instance_t(tool_address);
            player.ToolName = player.tool.get_name();
        }
        else {
            player.tool = rbx::instance_t(0);
            player.ToolName = "";
        }

        if (!is_local && player.hrp.address != 0) {
            rbx::part_t hrp_part(player.hrp.address);
            math::vector3_t player_position = hrp_part.get_part_pos();

            if (is_valid_position(player_position) && is_valid_position(local_position)) {
                player.distance = calculate_distance(local_position, player_position);
            }
            else {
                player.distance = 0.0f;
            }
        }
        else {
            player.distance = 0.0f;
        }
    }

    void update_aim_gui() {
        rbx::instance_t playerGui = globals::game::players.get_localpl().find_first_child("PlayerGui");
        globals::misc::aim = rbx::instance_t(0);

        if (playerGui.address) {
            for (auto& child : playerGui.get_children()) {
                for (auto& grandchild : child.get_children()) {
                    if (grandchild.get_name() == "Aim") {
                        globals::misc::aim = grandchild;
                        return;
                    }
                }
            }
        }
    }

    void teleport_monitor() {
        static std::uint64_t last_game_id = 0;

        while (true) {
            try {
                auto module = memory->get_module_address();
                auto fake_datamodel = memory->read<std::uint64_t>(module + Offsets::FakeDataModel::Pointer);
                globals::game::data_model = memory->read<std::uint64_t>(fake_datamodel + Offsets::FakeDataModel::RealDataModel);

                if (globals::game::data_model.address != 0) {
                    std::uint64_t game_id = globals::game::data_model.get_game_id();

                    if (game_id != last_game_id) {
                        last_game_id = game_id;
                        current_game_id.store(game_id);
                        globals::game::players = globals::game::data_model.find_first_child_by_class("Players");
                        globals::game::workspace = globals::game::data_model.find_first_child_by_class("Workspace");
                        references_updated.store(true);

                        if (game_id == 113491250) {
                            std::cout << "[PF] Detected Phantom Forces (Game ID: " << game_id << ")" << std::endl;
                        }
                        else if (game_id == 1168263273) {
                            std::cout << "[BB] Detected Bad Business (Game ID: " << game_id << ")" << std::endl;
                        }
                    }
                }
            }
            catch (...) {}

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void update_standard_cache(const math::vector3_t& local_position, const std::string& local_name) {
        if (globals::game::players.address == 0) return;

        std::uint64_t local_player_addr = globals::game::players.get_local_player();
        if (local_player_addr == 0) return;

        rbx::instance_t local_player_inst(local_player_addr);
        auto player_instances = globals::game::players.get_children();
        std::vector<rbx::player_t> players;
        players.reserve(player_instances.size());

        for (const auto& instance : player_instances) {
            rbx::player_t player{};
            player.player = instance;
            player.character = player.player.get_model_instance();
            player.Name = instance.get_name();
            player.userid = player.player.get_userid();
            player.localplayer = false;

            cache_player_data(player, local_position, false);

            if (globals::gamesupport::team_check) {
                if (is_same_team(local_player_inst, instance)) {
                    continue; 
                }
            }

            if (!globals::visuals::localplayercheck || player.hrp.address != globals::game::local_player.hrp.address) {
                players.push_back(std::move(player));
            }
        }

        std::lock_guard<std::mutex> lock(cache_mutex);
        globals::game::player_cache = std::move(players);
    }

    void update_cache() {
        if (globals::game::players.address == 0) return;

        std::uint64_t local_player_addr = globals::game::players.get_local_player();
        if (local_player_addr == 0) return;

        rbx::instance_t player_instance(local_player_addr);
        rbx::player_t local_player{};
        local_player.player = player_instance;
        local_player.character = local_player.player.get_model_instance();
        local_player.Name = player_instance.get_name();
        local_player.localplayer = true;

        math::vector3_t local_position{};
        cache_player_data(local_player, local_position, true);

        if (local_player.hrp.address != 0) {
            rbx::part_t local_hrp(local_player.hrp.address);
            local_position = local_hrp.get_part_pos();
        }

        update_aim_gui();
        globals::game::local_player = local_player;

        std::uint64_t game_id = current_game_id.load();

        if (game_id == 113491250) {
            std::vector<rbx::player_t> players;
            cache_phantom_forces_players(players, local_position, local_player.Name);
            std::lock_guard<std::mutex> lock(cache_mutex);
            globals::game::player_cache = std::move(players);
        }
        else if (game_id == 1168263273) {
            std::vector<rbx::player_t> players;
            cache_bad_business_players(players, local_position, local_player.Name);
            std::lock_guard<std::mutex> lock(cache_mutex);
            globals::game::player_cache = std::move(players);
        }
        else {
            update_standard_cache(local_position, local_player.Name);
        }
    }
}

void player_cache::hook_cache() {
    std::thread(teleport_monitor).detach();

    while (true) {
        try {
            if (references_updated.exchange(false)) {
                std::lock_guard<std::mutex> lock(cache_mutex);
                globals::game::player_cache.clear();
            }

            update_cache();
        }
        catch (...) {}

        std::this_thread::sleep_for(std::chrono::milliseconds(6));
    }
}