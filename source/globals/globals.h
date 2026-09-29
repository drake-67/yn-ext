#pragma once
#include "../sdk/datamodel/datamodel.hpp"
#include "../sdk/datamodel/players.hpp"
#include "../cheat/features/cache/cache.hpp"
#include "../sdk/graphics/visualengine.hpp"
#include "../cheat/drawing/imgui/imgui.h"
#include "../sdk/environment/environment.hpp"
#include "../sdk/datamodel/camera.hpp"
#include "../utils/ckeybind/ckeybind.h"
namespace globals {
    namespace visuals {
        inline bool enabled = true;
        inline bool box = true;
        inline bool chams = false;
        inline bool flags = true;
        inline bool name = true;
        inline bool tool = true;
        inline bool distance = true;
        inline bool armorbar = false;
        inline bool localplayercheck = false;
        inline bool rigtype = true;
        inline bool healthbar = false;
        inline bool skeleton = false;
        inline bool skeleton_outline = false;
        inline bool tracers = false;
        inline bool headdot = true;
        inline int chams_type = 1;
        inline int tracer_position = 2; // 0 = top left, 1 = top center, 2 = bottom center, 3 = center screen
        inline bool box_fill = true;
        inline bool box_gradient = true;
        inline bool box_outline = true;
        inline bool box_gradient_autorotate = false;
        inline float box_gradient_rotation_speed = 1.0f;
        inline float skeleton_thickness = 1.5f;
        inline float tracer_thickness = 1.5f;
        inline float headdot_size = 4.0f;
        inline bool headdot_outline = true;
        inline ImU32 outline_color = IM_COL32(0, 0, 0, 255);
        inline std::vector<bool> outline_elements = { true, true, true, true, true, true };
        enum class ESP_SIDE { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3 };
        inline std::vector<ESP_SIDE> element_sides = { ESP_SIDE::LEFT, ESP_SIDE::RIGHT, ESP_SIDE::TOP, ESP_SIDE::BOTTOM, ESP_SIDE::LEFT };
        inline float healthbar_size = 2.0f;
        inline float healthbar_padding = 4.0f;
        inline bool health_based_color = true;
        namespace colors {
            inline ImU32 box = IM_COL32(255, 255, 255, 255);
            inline ImU32 flags = IM_COL32(255, 255, 255, 255);
            inline float healthbar[4] = { 1.0f, 1.0f, 1.0f, 1.0f };  // RGBA in 0-1 range
            inline ImU32 name = IM_COL32(255, 255, 255, 255);
            inline ImU32 distance = IM_COL32(255, 255, 255, 255);
            inline ImU32 tracer = IM_COL32(255, 255, 255, 255);
            inline ImU32 skeleton = IM_COL32(255, 255, 255, 255);
            inline ImU32 skeleton_outline = IM_COL32(0, 0, 0, 255);
            inline ImU32 headdot = IM_COL32(255, 255, 255, 255);
            inline ImU32 headdot_outline = IM_COL32(0, 0, 0, 255);
            inline ImU32 glow_color = IM_COL32(255, 255, 255, 255);
            inline ImU32 cham_fill = IM_COL32(255, 255, 255, 80);
            inline ImU32 cham_outline = IM_COL32(255, 255, 255, 255);
            inline float box_color[4] = { 1.f, 1.f, 1.f, 1.f };
            inline float box_outline_color[3] = { 1.0f, 1.0f, 1.0f };
            inline float box_gradient_top[4] = { 1.0f, 1.0f, 1.0f, 0.2f };
            inline float box_gradient_bottom[4] = { 1.0f, 1.0f, 1.0f, 0.2f };
        }
    }
    namespace game {
        inline rbx::datamodel_t data_model;
        inline rbx::visualengine_t visual_engine;
        inline rbx::environment_t lightning;
        inline rbx::players_t players;
        inline rbx::datamodel_t workspace;
        inline rbx::camera_t camera;
        inline std::vector<rbx::player_t> player_cache;
        inline rbx::player_t local_player;
        inline bool anti_afk = false;
    }

    namespace listeners {
        inline bool triggerbot_listener = false;
        inline bool camlock_listener = false;
        inline bool player_select_listener = false;
        inline bool visuals_listener = false;
        inline bool macro_listener = false;
    }


    namespace rage {
        inline CKeybind walkspeed_bind{ ("walkspeed") };
        inline CKeybind jumppower_bind{ ("jumppower") };
        inline CKeybind orbitkeybind{ ("orbitt") };
        inline CKeybind hipheight_bind{ ("hipheightt") };
        inline CKeybind infinite_bind{ ("infinitebind") };
        inline CKeybind fly_bind{ ("flight") };
        inline bool speed_enabled = false;
        inline float third_person_distance = 10.0f;  // Add this line
        inline bool infinite_jump = false;
        inline bool third_person = false;
        inline CKeybind third_personbind{ ("thirdpersonn") };

        inline bool spinbot = false;
        inline bool antiaim = false;
        inline bool gravity_enabled = false;
        inline float gravity_value = 196.0f;

        inline bool orbit = false;
        inline bool antistomp = false;
        inline float orbitspeed = 16.0f;
        inline float infinite_power = 16.0f;
        inline float orbitx = 1.0f;
        inline float orbity = 3.0f;

        inline bool fly = false;
        inline bool hipheight_enabled = false;
        inline float hipheighrslider = 16.0f;

        inline float fly_speed = 16.0f;
        inline float walkspeed_amount = 16.0f;
        inline float jumppowevbalue = 16.0f;
    }
    namespace misc {
        inline rbx::instance_t aim;
        inline rbx::instance_t mouseserivce;
    }

    namespace image {
        inline bool imaged = true;
        inline int image_type = 0;
    }


    namespace silent {
        inline bool enabled = false;
        inline bool sticky = false;
        inline CKeybind bind{ ("silentaimmm") };
        inline float fov = 100.0f;
        inline bool use_fov = true;
        inline int hitpart_mode = 0;
        inline bool prediction = false;
        inline float prediction_x = 1.0f;
        inline float prediction_y = 1.0f;
        inline float prediction_z = 1.0f;
        inline int hit_chance = 100;
        inline bool team_check = true;
        inline bool knocked_check = true;
        inline bool health_check = true;
        inline bool use_mouseservice = true;
        inline bool use_frame_position = true;
    }

    namespace gamesupport {
        inline bool team_check = false;
    }

    namespace fov {

        inline bool enabled = true;
        inline bool filled = false;
        inline bool rainbow = true;

        inline int segments = 36;
        inline float radius = 100.0f;
        inline float thickness = 2.0f;

        inline ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        inline float hue = 0.0f;
        inline float hueSpeed = 0.002f;
    }
    namespace aim {
        inline bool aimbot = true;
        inline bool aimbot_sticky = false;
        inline rbx::instance_t aimbot_target;
        inline CKeybind aimbot_bind{ ("aimbot") };
        inline bool use_camera_mode = false;  // false = mouse mode, true = camera mode
        inline float mouse_smoothness = 2.0f;  // 1-100 (higher = smoother/slower)
        inline float mouse_sensitivity = 1.0f;  // Mouse sensitivity multiplier
        inline float camera_smoothness = 50.0f;  // 0-100 (higher = smoother/slower)
        inline bool shake = false;
		inline bool specate_target = false;
        inline bool wallbang = false;
        static float shake_x = 0.5f;
        inline float shake_y = 0.5f;
        inline float shake_z = 0.5f;
        inline float offset_x = 0.0f;
        inline float offset_y = 0.0f;
        inline float offset_z = 0.0f;
        inline int target_lock_delay = 0;
        inline int target_switch_delay = 500;
        inline int hitpart_mode = 0;
    }
    namespace draw {
    }
}