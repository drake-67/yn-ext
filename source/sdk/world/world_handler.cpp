#include <iostream>
#include "world_handler.hpp"
#include "../world/snow/snow.hpp"
#include "../../../globals/globals.h"
#include <thread>
#include <chrono>

namespace world {

    static particle::SnowSystem* g_snow_system = nullptr;
    static bool g_snow_running = false;
    static std::thread g_snow_thread;

    void initialize() {
        if (!g_snow_system) {
            g_snow_system = new particle::SnowSystem(500);
        }
    }

    void shutdown() {
        g_snow_running = false;
        if (g_snow_thread.joinable()) {
            g_snow_thread.join();
        }
        delete g_snow_system;
        g_snow_system = nullptr;
    }

    void update_snow() {
        auto last_time = std::chrono::high_resolution_clock::now();

        while (g_snow_running) {
            auto current_time = std::chrono::high_resolution_clock::now();
            float delta_time = std::chrono::duration<float>(current_time - last_time).count();
            last_time = current_time;

                g_snow_system->update(delta_time);
            

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    void start_snow_thread() {
        if (g_snow_running) return;

        initialize();
        g_snow_running = true;
        g_snow_thread = std::thread(update_snow);
    }

    void stop_snow_thread() {
        shutdown();
    }

    void render_snow(ImDrawList* draw_list) {

        rbx::visualengine_t visualengine = globals::game::visual_engine;
        if (!visualengine.address) return;

        math::vector2_t dimensions = visualengine.get_dimensions();
        math::matrix4_t viewMatrix = visualengine.get_viewmatrix();

        g_snow_system->render(draw_list, visualengine, dimensions, viewMatrix);
    }

    void set_snow_particle_count(int count) {
        if (g_snow_system) {
            g_snow_system->set_particle_count(count);
        }
    }

} // namespace world