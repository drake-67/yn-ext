#pragma once
#include <cstdint>
#include <string>
#include "../../../utils/math/math.hpp"
#include "../../../sdk/datamodel/instance_t.hpp"

namespace silent {
    struct silent_target_t {
       math::vector3_t world_position;
        math::vector2_t screen_position;
        float distance;
        std::string player_name;
        bool valid;

        silent_target_t()
            : world_position(0, 0, 0)
            , screen_position(0, 0)
            , distance(0.0f)
            , player_name("")
            , valid(false)
        {
        }
    };

    class silent_t {
    public:
        silent_t();
        void initialize();
        void update();

    private:
        bool enabled_;
        bool key_pressed_last_frame_;

        // Helper functions
        bool is_key_pressed(int virtual_key);
        float calculate_distance_2d(const math::vector2_t& pos1, const math::vector2_t& pos2);
        bool world_to_screen(const math::vector3_t& world_pos, math::vector2_t& screen_pos);

        // Target selection
        silent_target_t find_best_target();
        math::vector3_t get_hitpart_position(const auto& player);
        rbx::instance_t get_closest_part_to_cursor(const auto& player, const math::vector2_t& cursor_pos);

        // Execution
        void execute_silent_aim(const silent_target_t& target, const math::vector2_t& cursor_pos);
        bool is_within_fov(const math::vector2_t& screen_pos, const math::vector2_t& cursor_pos);
        bool passes_checks(const auto& player);
    };

    extern silent_t instance;
    void start_silent_thread();
}