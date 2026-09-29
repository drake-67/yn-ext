#include "../../../cheat/drawing/imgui/imgui.h"
#include "../../../cheat/drawing/imgui/imgui_internal.h"
#include "../cache/cache.hpp"
#include "../../../globals/globals.h"
#include "../../../sdk/datamodel/part_t.hpp"
#include "visuals.hpp"
#include "../../../sdk/world/world_handler.hpp"
#include "../visuals/renderer/renderer.hpp"
#include "../../../sdk/datamodel/humanoid.hpp"
#include <clipper2/clipper.core.h>
#include <clipper2/clipper.h>
#include "../../drawing/imgui/settings/variables.h"

#define _max(a,b) (((a) > (b)) ? (a) : (b))
#define _min(a,b) (((a) < (b)) ? (a) : (b))

static const math::vector3_t local_corners[8] = {
    {-1,-1,-1}, {1,-1,-1}, {-1,1,-1}, {1,1,-1},
    {-1,-1, 1}, {1,-1, 1}, {-1,1, 1}, {1,1, 1}
};

std::vector<const rbx::instance_t*> visuals_t::parts::get_parts(const rbx::player_t& player) {
    std::vector<const rbx::instance_t*> parts;
    bool has_r15_core = player.uppertorso.address && player.lowertorso.address;
    bool has_r6_core = player.torso.address;

    if (has_r15_core) {
        if (player.head.address) parts.push_back(&player.head);
        if (player.uppertorso.address) parts.push_back(&player.uppertorso);
        if (player.lowertorso.address) parts.push_back(&player.lowertorso);
        if (player.leftupperleg.address) parts.push_back(&player.leftupperleg);
        if (player.leftlowerleg.address) parts.push_back(&player.leftlowerleg);
        if (player.leftfoot.address) parts.push_back(&player.leftfoot);
        if (player.rightupperleg.address) parts.push_back(&player.rightupperleg);
        if (player.rightlowerleg.address) parts.push_back(&player.rightlowerleg);
        if (player.rightfoot.address) parts.push_back(&player.rightfoot);
        if (player.leftupperarm.address) parts.push_back(&player.leftupperarm);
        if (player.leftlowerarm.address) parts.push_back(&player.leftlowerarm);
        if (player.lefthand.address) parts.push_back(&player.lefthand);
        if (player.rightupperarm.address) parts.push_back(&player.rightupperarm);
        if (player.rightlowerarm.address) parts.push_back(&player.rightlowerarm);
        if (player.righthand.address) parts.push_back(&player.righthand);
    }
    else if (has_r6_core) {
        if (player.head.address) parts.push_back(&player.head);
        if (player.torso.address) parts.push_back(&player.torso);
        if (player.leftarm.address) parts.push_back(&player.leftarm);
        if (player.rightarm.address) parts.push_back(&player.rightarm);
        if (player.leftleg.address) parts.push_back(&player.leftleg);
        if (player.rightleg.address) parts.push_back(&player.rightleg);
    }
    else {
        for (const auto& bone : player.bones) {
            if (bone.address) {
                parts.push_back(&bone);
            }
        }

        if (parts.empty()) {
            if (player.hrp.address) parts.push_back(&player.hrp);
            if (player.head.address) parts.push_back(&player.head);
            if (player.torso.address) parts.push_back(&player.torso);
            if (player.uppertorso.address) parts.push_back(&player.uppertorso);
            if (player.lowertorso.address) parts.push_back(&player.lowertorso);
        }
    }
    return parts;
}

namespace elements {

    static const math::vector3_t local_corners[8] = {
    {-1,-1,-1}, {1,-1,-1}, {-1,1,-1}, {1,1,-1},
    {-1,-1, 1}, {1,-1, 1}, {-1,1, 1}, {1,1, 1}
    };

    auto calculateDistance = [](const math::vector3_t& pos1, const math::vector3_t& pos2) -> float {
        return std::sqrt((pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y) + (pos1.z - pos2.z) * (pos1.z - pos2.z)); };

    void headdot(const rbx::player_t& player, ImDrawList* draw) {
        if (!globals::visuals::headdot) return;
        if (!player.head.address) return;

        rbx::part_t head(player.head.address);
        auto prim = head.get_primitive();
        auto head_pos = prim.get_position();

        rbx::visualengine_t visualengine = globals::game::visual_engine;
        math::vector2_t dimensions = visualengine.get_dimensions();
        math::matrix4_t viewMatrix = visualengine.get_viewmatrix();

        auto screen = visualengine.world_to_screen(head_pos, dimensions, viewMatrix);
        if (screen.x < 0.f || screen.y < 0.f) return;

        ImVec2 center(screen.x, screen.y);
        float radius = globals::visuals::headdot_size;

        if (globals::visuals::headdot_outline) {
            draw->AddCircleFilled(center, radius + 1.0f, globals::visuals::colors::headdot_outline);
        }
        draw->AddCircleFilled(center, radius, globals::visuals::colors::headdot);
    }

    void tracers(const rbx::player_t& player, ImDrawList* draw) {
        if (!globals::visuals::tracers) return;
        if (!player.hrp.address && !player.head.address) return;

        rbx::visualengine_t visualengine = globals::game::visual_engine;
        math::vector2_t dimensions = visualengine.get_dimensions();
        math::matrix4_t viewMatrix = visualengine.get_viewmatrix();

        rbx::part_t target_part(player.hrp.address ? player.hrp.address : player.head.address);
        auto prim = target_part.get_primitive();
        auto target_pos = prim.get_position();

        auto screen = visualengine.world_to_screen(target_pos, dimensions, viewMatrix);
        if (screen.x < 0.f || screen.y < 0.f) return;

        ImVec2 start_pos;
        switch (globals::visuals::tracer_position) {
        case 0: // Top Left
            start_pos = ImVec2(0.0f, 0.0f);
            break;
        case 1: // Top Center
            start_pos = ImVec2(dimensions.x * 0.5f, 0.0f);
            break;
        case 2: // Bottom Center
            start_pos = ImVec2(dimensions.x * 0.5f, dimensions.y);
            break;
        case 3: // Center Screen
            start_pos = ImVec2(dimensions.x * 0.5f, dimensions.y * 0.5f);
            break;
        default:
            start_pos = ImVec2(dimensions.x * 0.5f, dimensions.y);
            break;
        }

        ImVec2 end_pos(screen.x, screen.y);

        draw->AddLine(start_pos, end_pos, globals::visuals::colors::skeleton_outline, globals::visuals::tracer_thickness + 2.0f);
        draw->AddLine(start_pos, end_pos, globals::visuals::colors::tracer, globals::visuals::tracer_thickness);
    }

 
    void skeleton(const rbx::player_t& player, ImDrawList* draw) {
        if (!globals::visuals::skeleton) return;

        rbx::visualengine_t visualengine = globals::game::visual_engine;
        math::vector2_t dimensions = visualengine.get_dimensions();
        math::matrix4_t viewMatrix = visualengine.get_viewmatrix();

        std::vector<std::pair<rbx::instance_t, rbx::instance_t>> boneConnections;

        bool has_r15_core = player.uppertorso.address && player.lowertorso.address;
        bool has_r6_core = player.torso.address;

        if (has_r15_core) {
            boneConnections = {
                {player.head, player.uppertorso},
                {player.uppertorso, player.lowertorso},
                {player.uppertorso, player.leftupperarm},
                {player.leftupperarm, player.leftlowerarm},
                {player.leftlowerarm, player.lefthand},
                {player.uppertorso, player.rightupperarm},
                {player.rightupperarm, player.rightlowerarm},
                {player.rightlowerarm, player.righthand},
                {player.lowertorso, player.leftupperleg},
                {player.leftupperleg, player.leftlowerleg},
                {player.leftlowerleg, player.leftfoot},
                {player.lowertorso, player.rightupperleg},
                {player.rightupperleg, player.rightlowerleg},
                {player.rightlowerleg, player.rightfoot}
            };
        }
        else if (has_r6_core) {
            boneConnections = {
                {player.head, player.torso},
                {player.torso, player.leftarm},
                {player.torso, player.rightarm},
                {player.torso, player.leftleg},
                {player.torso, player.rightleg}
            };
        }

        const math::vector3_t upperTorsoOffset(0.f, 0.5f, 0.f);
        const math::vector3_t upperArmOffset(0.f, 0.4f, 0.f);

        ImDrawListFlags backup_flags = draw->Flags;
        draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;

        for (const auto& connection : boneConnections) {
            const auto& start = connection.first;
            const auto& end = connection.second;

            if (!start.address || !end.address)
                continue;

            rbx::part_t startPart(start.address);
            rbx::part_t endPart(end.address);

            auto startPrim = startPart.get_primitive();
            auto endPrim = endPart.get_primitive();

            math::vector3_t startPos = startPrim.get_position();
            math::vector3_t endPos = endPrim.get_position();

            if (has_r15_core) {
                if (start.address == player.uppertorso.address) {
                    startPos = startPos + upperTorsoOffset;
                }
                if (end.address == player.uppertorso.address) {
                    endPos = endPos + upperTorsoOffset;
                }
                if (start.address == player.leftupperarm.address || start.address == player.rightupperarm.address) {
                    startPos = startPos + upperArmOffset;
                }
                if (end.address == player.leftupperarm.address || end.address == player.rightupperarm.address) {
                    endPos = endPos + upperArmOffset;
                }
            }

            auto startScreen = visualengine.world_to_screen(startPos, dimensions, viewMatrix);
            auto endScreen = visualengine.world_to_screen(endPos, dimensions, viewMatrix);

            if (startScreen.x < 0.f || startScreen.y < 0.f || endScreen.x < 0.f || endScreen.y < 0.f)
                continue;

            ImVec2 startPoint = ImVec2(roundf(startScreen.x), roundf(startScreen.y));
            ImVec2 endPoint = ImVec2(roundf(endScreen.x), roundf(endScreen.y));

            if (globals::visuals::skeleton_outline) {
                draw->AddLine(startPoint, endPoint, globals::visuals::colors::skeleton_outline, globals::visuals::skeleton_thickness + 2.0f);
            }

            draw->AddLine(startPoint, endPoint, globals::visuals::colors::skeleton, globals::visuals::skeleton_thickness);
        }

        draw->Flags = backup_flags;
    }

    void name_esp(const rbx::player_t& player, ImDrawList* draw, const ImVec2& box_pos, const ImVec2& box_size) {
        if (!globals::visuals::name) return;
        draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;
        const char* name = player.Name.c_str();
        const ImVec2 text_size = ImGui::CalcTextSize(name);
        globals::visuals::ESP_SIDE side = globals::visuals::ESP_SIDE::TOP;
        bool is_vertical = (side == globals::visuals::ESP_SIDE::LEFT || side == globals::visuals::ESP_SIDE::RIGHT);
        float gap = 3.0f;
        ImVec2 final_pos;
        if (is_vertical) {
            if (side == globals::visuals::ESP_SIDE::LEFT) {
                final_pos = ImVec2(
                    std::round(box_pos.x - gap - text_size.x),
                    std::round(box_pos.y)
                );
            }
            else {
                final_pos = ImVec2(
                    std::round(box_pos.x + box_size.x + gap),
                    std::round(box_pos.y)
                );
            }
        }
        else {
            if (side == globals::visuals::ESP_SIDE::TOP) {
                final_pos = ImVec2(
                    std::round(box_pos.x + (box_size.x * 0.5f) - (text_size.x * 0.5f)),
                    std::round(box_pos.y - text_size.y - gap)
                );
            }
            else {
                final_pos = ImVec2(
                    std::round(box_pos.x + (box_size.x * 0.5f) - (text_size.x * 0.5f)),
                    std::round(box_pos.y + box_size.y + gap)
                );
            }
        }
        renderer_e::text_Voided(
            final_pos,
            name,
            globals::visuals::colors::name,
            nullptr
        );
    }


    void healthbar(const rbx::player_t& player, ImDrawList* draw, const ImVec2& box_pos, const ImVec2& box_size, globals::visuals::ESP_SIDE side) {
        if (!player.humanoid.address) return;

        rbx::humanoid_t humanoid(player.humanoid.address);
        float health = humanoid.get_health();
        float max_health = humanoid.get_max_health();

        if (max_health <= 0.f) return;

        float health_percentage = std::fmax(0.f, std::fmin(health / max_health, 1.f));

        ImU32 health_color = (health_percentage > 0.5f)
            ? IM_COL32(static_cast<int>(255 * (1.f - health_percentage) * 2.f), 255, 0, 255)
            : IM_COL32(255, static_cast<int>(255 * health_percentage * 2.f), 0, 255);

        bool is_vertical = (side == globals::visuals::ESP_SIDE::LEFT || side == globals::visuals::ESP_SIDE::RIGHT);

        renderer_e::healthbar_positioned(box_pos, box_size, health, max_health, health_color, side, is_vertical, globals::visuals::healthbar_padding, globals::visuals::healthbar_size, true, IM_COL32(0, 0, 0, 255));
    }

    void state_esp(const rbx::player_t& player, ImDrawList* draw, const ImVec2& box_pos, const ImVec2& box_size) {
        if (!player.humanoid.address) return;
        rbx::humanoid_t humanoid(player.humanoid.address);
        int humanoid_state = humanoid.get_humanoid_state();
        const char* sz_state = "Unknown";
        if (humanoid_state == 8 && player.head.address) {
            rbx::part_t head(player.head.address);
            if (head.get_primitive().get_velocity().length() < 0.1f) {
                sz_state = "Idle";
            }
        }
        if (humanoid_state != 8 || strcmp(sz_state, "Unknown") == 0) {
            static const char* states[] =
            {
                "FallingDown",      // 0
                "Ragdoll",          // 1
                "GettingUp",        // 2
                "Jumping",          // 3
                "Swimming",         // 4
                "Freefall",         // 5
                "Flying",           // 6
                "Landed",           // 7
                "Running",          // 8
                "Unknown",          // 9
                "RunningNoPhysics", // 10
                "StrafingNoPhysics",// 11
                "Climbing",         // 12
                "Seated",           // 13
                "PlatformStanding", // 14
                "Dead",             // 15
                "Physics",          // 16
                "Unknown",          // 17
                "None"              // 18
            };

            if (humanoid_state >= 0 && humanoid_state < 19) {
                sz_state = states[humanoid_state];
            }
        }

        draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;

        ImVec2 text_size = ImGui::CalcTextSize(sz_state);
        globals::visuals::ESP_SIDE side = globals::visuals::ESP_SIDE::RIGHT;
        bool is_vertical = (side == globals::visuals::ESP_SIDE::LEFT || side == globals::visuals::ESP_SIDE::RIGHT);
        float gap = 3.0f;

        ImVec2 final_pos;
        if (is_vertical) {
            if (side == globals::visuals::ESP_SIDE::LEFT) {
                final_pos = ImVec2(
                    std::round(box_pos.x - gap - text_size.x),
                    std::round(box_pos.y + (box_size.y * 0.5f) - (text_size.y * 0.5f))
                );
            }
            else { // RIGHT
                final_pos = ImVec2(
                    std::round(box_pos.x + box_size.x + gap),
                    std::round(box_pos.y)
                );
            }
        }
        else {
            if (side == globals::visuals::ESP_SIDE::TOP) {
                final_pos = ImVec2(
                    std::round(box_pos.x + (box_size.x * 0.5f) - (text_size.x * 0.5f)),
                    std::round(box_pos.y - text_size.y - gap)
                );
            }
            else { // BOTTOM
                final_pos = ImVec2(
                    std::round(box_pos.x + (box_size.x * 0.5f) - (text_size.x * 0.5f)),
                    std::round(box_pos.y + box_size.y + gap)
                );
            }
        }

        renderer_e::text_Voided(
            final_pos,
            sz_state,
            globals::visuals::colors::name,
            nullptr
        );
    }
    void tool_esp(const rbx::player_t& player, ImDrawList* draw, const ImVec2& box_pos, const ImVec2& box_size, float offset = 0.0f) {
        const char* tool_name = "None";
        if (player.tool.address && !player.ToolName.empty()) {
            tool_name = player.ToolName.c_str();
        }

        draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;
        ImVec2 text_size = ImGui::CalcTextSize(tool_name);

        globals::visuals::ESP_SIDE side = globals::visuals::ESP_SIDE::BOTTOM;
        float gap = 3.0f;
        ImVec2 final_pos;

        if (side == globals::visuals::ESP_SIDE::BOTTOM) {
            final_pos = ImVec2(
                std::round(box_pos.x + (box_size.x * 0.5f) - (text_size.x * 0.5f)),
                std::round(box_pos.y + box_size.y + gap + offset)
            );
        }
        else {
            return;
        }

        renderer_e::text_Voided(
            final_pos,
            tool_name,
            globals::visuals::colors::name,
            nullptr
        );
    }


    void distance_esp(const rbx::player_t& player, ImDrawList* draw, const ImVec2& box_pos, const ImVec2& box_size, float offset = 24.0f) {
        char distance_text[32];
        snprintf(distance_text, sizeof(distance_text), "%.0fm", player.distance);

        ImVec2 text_size = ImGui::CalcTextSize(distance_text);
        ImVec2 text_pos = ImVec2(
            std::round(box_pos.x + (box_size.x * 0.5f) - (text_size.x * 0.5f)),
            std::round(box_pos.y + box_size.y + 1.0f + offset - 7.0f)
        );

        renderer_e::text_Voided(
            text_pos,
            distance_text,
            globals::visuals::colors::name,
            nullptr
        );
    }

    void armorbar(const rbx::player_t& player, ImDrawList* draw, const ImVec2& box_pos, const ImVec2& box_size, globals::visuals::ESP_SIDE side) {
        if (!player.humanoid.address) return;

        rbx::humanoid_t humanoid(player.humanoid.address);
        float armor = 100.0f;
        float max_armor = 100.0f;

        if (max_armor <= 0.f) return;

        float armor_percentage = std::fmax(0.f, std::fmin(armor / max_armor, 1.f));
        ImU32 armor_color = IM_COL32(
            0,
            static_cast<int>(100 + 155 * armor_percentage),
            255,
            255
        );

        bool is_vertical = (side == globals::visuals::ESP_SIDE::LEFT || side == globals::visuals::ESP_SIDE::RIGHT);
        float bar_spacing = 4.0f;
        ImVec2 adjusted_pos = box_pos;

        if (is_vertical) {
            if (side == globals::visuals::ESP_SIDE::LEFT) {
                adjusted_pos.x -= (globals::visuals::healthbar_size + bar_spacing);
            }
            else {
                adjusted_pos.x += (globals::visuals::healthbar_size + bar_spacing);
            }
        }
        else {
            if (side == globals::visuals::ESP_SIDE::TOP) {
                adjusted_pos.y -= (globals::visuals::healthbar_size + bar_spacing);
            }
            else {
                adjusted_pos.y += (globals::visuals::healthbar_size + bar_spacing);
            }
        }

        renderer_e::armorbar_positioned(adjusted_pos, box_size, armor, max_armor, armor_color, side, is_vertical, globals::visuals::healthbar_padding, globals::visuals::healthbar_size, true, IM_COL32(0, 0, 0, 255));
    }


    void rig_type(const rbx::player_t& player, ImDrawList* draw, const ImVec2& box_pos, const ImVec2& box_size, float offset = 16.0f) {
        if (!player.humanoid.address) return;
        rbx::humanoid_t humanoid(player.humanoid.address);
        int rig_type = humanoid.get_rig_type();
        const char* sz_rig = "Unknown";
        if (rig_type == 1) {
            sz_rig = "R15";
        }
        else if (rig_type == 0) {
            sz_rig = "R6";
        }
        draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;
        ImVec2 rig_size = ImGui::CalcTextSize(sz_rig);
        globals::visuals::ESP_SIDE side = globals::visuals::ESP_SIDE::RIGHT;
        bool is_vertical = (side == globals::visuals::ESP_SIDE::LEFT || side == globals::visuals::ESP_SIDE::RIGHT);
        float gap = 3.0f;
        ImVec2 rig_pos;

        if (is_vertical) {
            if (side == globals::visuals::ESP_SIDE::LEFT) {
                rig_pos = ImVec2(
                    std::round(box_pos.x - gap - rig_size.x),
                    std::round(box_pos.y + offset)
                );
            }
            else { // RIGHT
                rig_pos = ImVec2(
                    std::round(box_pos.x + box_size.x + gap),
                    std::round(box_pos.y + offset)
                );
            }
        }
        else {
            float center_x = box_pos.x + (box_size.x * 0.5f);
            if (side == globals::visuals::ESP_SIDE::TOP) {
                rig_pos = ImVec2(
                    std::round(center_x - rig_size.x * 0.5f),
                    std::round(box_pos.y - offset - rig_size.y)
                );
            }
            else { // BOTTOM
                rig_pos = ImVec2(
                    std::round(center_x - rig_size.x * 0.5f),
                    std::round(box_pos.y + box_size.y + offset)
                );
            }
        }

        renderer_e::text_Voided(
            rig_pos,
            sz_rig,
            globals::visuals::colors::name,
            nullptr
        );
    }

    // box call
    void box(const rbx::player_t& player, ImDrawList* draw) {

        std::vector<const rbx::instance_t*> parts = visuals_t::parts::get_parts(player);
        if (parts.empty()) return;

        rbx::visualengine_t visualengine = globals::game::visual_engine;
        math::vector2_t dimensions = visualengine.get_dimensions();
        math::matrix4_t viewMatrix = visualengine.get_viewmatrix();

        float left = FLT_MAX, top = FLT_MAX, right = -FLT_MAX, bottom = -FLT_MAX;
        bool valid = false;

        for (const auto* part_inst : parts) {
            if (!part_inst->address) continue;
            rbx::part_t part(part_inst->address);
            auto prim = part.get_primitive();
            auto size = prim.get_size();
            auto pos = prim.get_position();
            auto rot = prim.get_rotation();

            for (const auto& lc : local_corners) {
                math::vector3_t world = pos + rot * math::vector3_t{ lc.x * size.x * 0.5f, lc.y * size.y * 0.5f, lc.z * size.z * 0.5f };
                auto screen = visualengine.world_to_screen(world, dimensions, viewMatrix);
                if (screen.x < 0.f || screen.y < 0.f) continue;

                valid = true;
                left = _min(left, screen.x);
                top = _min(top, screen.y);
                right = _max(right, screen.x);
                bottom = _max(bottom, screen.y);
            }
        }

        if (!valid || left >= right || top >= bottom) return;

        float box_width = (right - left) + 2.f;
        float box_height = (bottom - top) + 2.f;

        const float MAX_BOX_WIDTH = dimensions.x * 0.4f;
        const float MAX_BOX_HEIGHT = dimensions.y * 0.6f;

        if (box_width > MAX_BOX_WIDTH || box_height > MAX_BOX_HEIGHT) return;

        ImVec2 box_pos(left - 1.f, top - 1.f);
        ImVec2 box_size(box_width, box_height);

        ImVec2 glow_offset = { 0, 0 };
        auto draw3 = ImGui::GetBackgroundDrawList();

        if (globals::visuals::box) {
            renderer_e::outlined_rect(box_pos, box_size, IM_COL32(255, 255, 255, 255), 0.f);
        }



        if (globals::visuals::name) {
            name_esp(player, draw, box_pos, box_size);
        }

        if (globals::visuals::flags) {
            state_esp(player, draw, box_pos, box_size);
        }

        if (globals::visuals::healthbar) {
            healthbar(player, draw, box_pos, box_size, globals::visuals::element_sides[4]);
        }

        if (globals::visuals::rigtype) {
            rig_type(player, draw, box_pos, box_size);
        }

        if (globals::visuals::tool) {
            tool_esp(player, draw, box_pos, box_size);
        }

        if (globals::visuals::distance) {
            distance_esp(player, draw, box_pos, box_size);
        }

        if (globals::visuals::armorbar) {
            armorbar(player, draw, box_pos, box_size, globals::visuals::element_sides[4]);
        }

    }


    void chams(const rbx::player_t& player, ImDrawList* draw) {
        draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;

        std::vector<const rbx::instance_t*> parts = visuals_t::parts::get_parts(player);
        if (parts.empty()) return;

        rbx::visualengine_t visualengine = globals::game::visual_engine;
        math::vector2_t dimensions = visualengine.get_dimensions();
        math::matrix4_t viewMatrix = visualengine.get_viewmatrix();

        auto project_part = [&](const rbx::instance_t* part_inst) -> std::vector<ImVec2> {
            std::vector<ImVec2> projected;
            if (!part_inst->address) return projected;

            rbx::part_t part(part_inst->address);
            auto prim = part.get_primitive();
            auto size = prim.get_size();
            auto pos = prim.get_position();
            auto rot = prim.get_rotation();

            for (const auto& lc : local_corners) {
                math::vector3_t world = pos + rot * math::vector3_t{
                    lc.x * size.x * 0.5f,
                    lc.y * size.y * 0.5f,
                    lc.z * size.z * 0.5f
                };
                auto screen = visualengine.world_to_screen(world, dimensions, viewMatrix);
                if (screen.x >= 0.f && screen.y >= 0.f)
                    projected.push_back(ImVec2(screen.x, screen.y));
            }

            if (projected.size() < 3) return {};

            std::sort(projected.begin(), projected.end(), [](const ImVec2& a, const ImVec2& b) {
                return a.x < b.x || (a.x == b.x && a.y < b.y);
                });

            std::vector<ImVec2> hull;
            auto cross = [](const ImVec2& O, const ImVec2& A, const ImVec2& B) {
                return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
                };

            for (auto& p : projected) {
                while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0)
                    hull.pop_back();
                hull.push_back(p);
            }

            size_t t = hull.size() + 1;
            for (int i = (int)projected.size() - 1; i >= 0; --i) {
                auto& p = projected[i];
                while (hull.size() >= t && cross(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0)
                    hull.pop_back();
                hull.push_back(p);
            }

            hull.pop_back();
            return hull;
            };

        if (globals::visuals::chams_type == 0) {
            for (auto& part : parts) {
                auto hull = project_part(part);
                if (hull.empty()) continue;
                draw->AddConvexPolyFilled(hull.data(), hull.size(), globals::visuals::colors::cham_fill);
                draw->AddPolyline(hull.data(), hull.size(), globals::visuals::colors::cham_outline, true, 2.f);
            }
        }
        else if (globals::visuals::chams_type == 1) {
            Clipper2Lib::Paths64 all_parts;
            for (auto& part : parts) {
                auto hull = project_part(part);
                if (hull.size() < 3) continue;

                Clipper2Lib::Path64 path;
                for (auto& pt : hull)
                    path.push_back({
                        static_cast<int64_t>(pt.x * 1000.0),
                        static_cast<int64_t>(pt.y * 1000.0)
                        });
                all_parts.push_back(path);
            }

            if (all_parts.empty()) return;

            auto unified_solution = Clipper2Lib::Union(all_parts, Clipper2Lib::FillRule::NonZero);
            for (auto& sp : unified_solution) {
                std::vector<ImVec2> poly;
                for (auto& pt : sp)
                    poly.push_back(ImVec2(pt.x / 1000.0f, pt.y / 1000.0f));
                if (poly.size() < 3) continue;

                 draw->AddConcavePolyFilled(poly.data(), poly.size(), globals::visuals::colors::cham_fill);
                draw->AddPolyline(poly.data(), poly.size(), IM_COL32(255, 255, 255, 255), true, 2.f);
            }
        }
    }

}





void visuals_t::renderr() {
    if (!globals::visuals::enabled) return;

    ImDrawList* draw = ImGui::GetForegroundDrawList();
    draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;



    for (const auto& player : player_cache::get_snapshot()) {
        try {
        if (!player.character.address) continue;
        if (player.localplayer) continue;
        if (!player.head.address) continue;

        if (globals::visuals::skeleton) {
            elements::skeleton(player, draw);
        }

        elements::box(player, draw);

        if (globals::visuals::chams) {
            elements::chams(player, draw);
        }

        if (globals::visuals::tracers) {
            elements::tracers(player, draw);
        }

        if (globals::visuals::headdot) {
            elements::headdot(player, draw);
        }
        } catch (...) { continue; } // one stale player must not kill the render thread
    }

}