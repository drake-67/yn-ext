#pragma once
#include <vector>
#include "../../../cheat/drawing/imgui/imgui.h"
#include "../../../utils/math/math.hpp"
#include "../../graphics/visualengine.hpp"

namespace particle {

    struct SnowParticle {
        math::vector3_t position;
        math::vector3_t velocity;
        float size;
        float lifetime;
        float sway_offset;
        float sway_speed;

        SnowParticle();
        void reset();
        void update(float dt);
    };

    class SnowSystem {
    public:
        SnowSystem(int count);

        void update(float dt);
        void render(ImDrawList* draw_list,
            const rbx::visualengine_t& visualengine,
            const math::vector2_t& dimensions,
            const math::matrix4_t& viewMatrix);

        void set_particle_count(int count);
        int get_particle_count() const;

    private:
        std::vector<SnowParticle> particles;
    };

} // namespace particle