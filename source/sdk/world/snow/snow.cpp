#include "snow.hpp"
#include <random>
#include <algorithm>
#include "../../../globals/globals.h"

#define _max(a,b) (((a) > (b)) ? (a) : (b))
#define _min(a,b) (((a) < (b)) ? (a) : (b))

namespace particle {

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    static float random_float(float min, float max) {
        return min + dis(gen) * (max - min);
    }

    SnowParticle::SnowParticle()
        : position(0, 0, 0)
        , velocity(0, 0, 0)
        , size(1.0f)
        , lifetime(0.0f)
        , sway_offset(0.0f)
        , sway_speed(1.0f) {
        reset();
    }

    void SnowParticle::reset() {
        position.x = random_float(-60.0f, 60.0f);
        position.y = random_float(30.0f, 50.0f);
        position.z = random_float(-60.0f, 60.0f);

        velocity.x = random_float(-0.3f, 0.3f);
        velocity.y = random_float(-3.0f, -6.0f);
        velocity.z = random_float(-0.3f, 0.3f);

        size = random_float(1.0f, 3.0f);
        lifetime = 0.0f;
        sway_offset = random_float(0.0f, 6.28318f);
        sway_speed = random_float(0.8f, 1.5f);
    }

    void SnowParticle::update(float dt) {
        lifetime += dt;
        sway_offset += sway_speed * dt;

        position.x += velocity.x * dt + std::sin(sway_offset) * 0.4f * dt;
        position.y += velocity.y * dt;
        position.z += velocity.z * dt + std::cos(sway_offset * 0.7f) * 0.3f * dt;

        if (position.y < -10.0f) {
            reset();
        }
    }

    SnowSystem::SnowSystem(int count) {
        particles.reserve(count);
        for (int i = 0; i < count; ++i) {
            particles.emplace_back();
        }
    }

    void SnowSystem::update(float dt) {
        for (auto& particle : particles) {
            particle.update(dt);
        }
    }

    void SnowSystem::render(ImDrawList* draw_list,
        const rbx::visualengine_t& visualengine,
        const math::vector2_t& dimensions,
        const math::matrix4_t& viewMatrix) {
        if (!draw_list) return;

        for (const auto& particle : particles) {
            rbx::visualengine_t visualengine = globals::game::visual_engine;
            math::vector2_t screen = visualengine.world_to_screen(particle.position, dimensions, viewMatrix);

            if (screen.x < 0.f || screen.y < 0.f ||
                screen.x > dimensions.x || screen.y > dimensions.y) continue;

            float depth = particle.position.y;
            float alpha = _max(0.3f, _min(1.0f, 1.0f - (depth / 100.0f)));
            float radius = _max(1.0f, _min(particle.size, 4.0f));

            ImU32 color = IM_COL32(255, 255, 255, static_cast<int>(alpha * 200));
            draw_list->AddCircleFilled(ImVec2(screen.x, screen.y), radius, color);
        }
    }

    void SnowSystem::set_particle_count(int count) {
        int current = static_cast<int>(particles.size());

        if (count > current) {
            particles.reserve(count);
            for (int i = current; i < count; ++i) {
                particles.emplace_back();
            }
        }
        else if (count < current) {
            particles.resize(count);
        }
    }

    int SnowSystem::get_particle_count() const {
        return static_cast<int>(particles.size());
    }

} // namespace particle