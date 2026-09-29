#include "rage.hpp"
#include <WinSock2.h>
#include <thread>
#include "../../../globals/globals.h"
#include "../../../sdk/datamodel/humanoid.hpp"
#include "../../initialise.hpp"
#include "../../../sdk/offsets.hpp"
#include "../../../sdk/datamodel/part_t.hpp"

int random_ints(int min, int max) {
    return min + rand() % (max - min + 1);
}

static void write_string(std::uint64_t self, const std::string& value) {
    const auto currentStringLength = memory->read<int64_t>(self + 0x18);
    std::uint64_t stringAddress = currentStringLength >= 16u ? memory->read<std::uint64_t>(self) : self;

    for (size_t i = 0; i < value.size() && i < 200; i++) {
        memory->write<char>(stringAddress + i, value[i]);
    }

    memory->write<char>(stringAddress + value.size(), '\0');
    memory->write<int64_t>(self + 0x10, static_cast<int64_t>(value.size()));
}

void rage::features::walkspeed() {
    DWORD last_check = 0;
    const DWORD check_interval = 1;
    static int initial_speed = -1;

    rbx::humanoid_t humanoid;

    while (true) {
        DWORD current_time = GetTickCount();
        globals::rage::walkspeed_bind.update();

        if (!globals::rage::speed_enabled || !globals::rage::walkspeed_bind.enabled) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        if (globals::rage::speed_enabled) {
            if (current_time - last_check >= check_interval) {
                auto current_speed = humanoid.read_walkspeed();
                if (initial_speed == -1 && current_speed != 0) {
                    initial_speed = current_speed;
                }

                if (globals::rage::walkspeed_amount && globals::rage::walkspeed_bind.enabled) {
                    humanoid.walkspeedloop(globals::rage::walkspeed_amount);
                }
                else if (initial_speed != -1 && current_speed == globals::rage::walkspeed_amount) {
                    humanoid.walkspeedloop(initial_speed);
                }
                last_check = current_time;
            }
            Sleep(1);
        }
        else {
            Sleep(50);
        }
    }
}

void rage::features::spinbot() {
    bool was_enabled = false;

    while (true) {
        if (globals::rage::spinbot == false) {
            if (was_enabled) {
                rbx::part_t local_root(globals::game::local_player.hrp.address);
                rbx::humanoid_t local_humanoid(globals::game::local_player.humanoid.address);

                if (local_root.address && local_humanoid.address) {
                    math::matrix3_t identity(1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f);
                    rbx::part_t prim = local_root.get_primitive();
                    if (prim.address) {
                        prim.set_rotation(identity);
                    }
                    memory->write<std::uint8_t>(local_humanoid.address + 0x1d9, 1);
                }
            }
            was_enabled = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }
        else {
            was_enabled = true;
            static float angle = 0.0f;
            angle += 0.067f;
            if (angle > 6.283185f) {
                angle -= 6.283185f;
            }

            rbx::part_t local_root(globals::game::local_player.hrp.address);
            rbx::humanoid_t local_humanoid(globals::game::local_player.humanoid.address);

            if (local_root.address && local_humanoid.address) {
                memory->write<std::uint8_t>(local_humanoid.address + 0x1d9, 0);
                rbx::part_t prim = local_root.get_primitive();
                if (prim.address) {
                    float c = cosf(angle);
                    float s = sinf(angle);
                    math::matrix3_t rot_y(c, 0.f, -s, 0.f, 1.f, 0.f, s, 0.f, c);
                    prim.set_rotation(rot_y);
                }
            }
			// Sleep(1)  this causes 12+ cpu usage for some reason (whiletrue) when disabled turn it on if u want good cpu
        }
    }
}

void jumppower() {
    while (true) {
        globals::rage::jumppower_bind.update();
        if (globals::rage::jumppower_bind.enabled == false) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }
        rbx::humanoid_t local_humanoid(globals::game::local_player.humanoid.address);
        if (globals::rage::jumppower_bind.enabled) {
            local_humanoid.set_jumppower(globals::rage::jumppowevbalue);
        }
        Sleep(50);
    }
}

void fly() {
    bool air_check = false;

    while (true) {
        globals::rage::fly_bind.update();
        if (!globals::rage::fly || !globals::rage::fly_bind.enabled) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        rbx::instance_t character = globals::game::players.get_localpl().get_model_instance();
        rbx::part_t hrp = character.find_first_child("HumanoidRootPart");

        if (!hrp.address) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        auto workspace = globals::game::workspace;
        globals::game::camera = workspace.find_first_child("Camera");
        math::matrix3_t rotation_matrix = globals::game::camera.get_camera_rotation();
        math::vector3_t look_vector = lookvec(rotation_matrix);
        math::vector3_t right_vector = rightvec(rotation_matrix);
        math::vector3_t direction = { 0.0f, 0.0f, 0.0f };

        if (GetAsyncKeyState('W') & 0x8000) {
            direction = direction - look_vector;
            air_check = true;
        }
        if (GetAsyncKeyState('S') & 0x8000) {
            direction = direction + look_vector;
            air_check = true;
        }
        if (GetAsyncKeyState('A') & 0x8000) {
            direction = direction - right_vector;
            air_check = true;
        }
        if (GetAsyncKeyState('D') & 0x8000) {
            direction = direction + right_vector;
            air_check = true;
        }

        if (direction.magnitude() > 0) {
            direction = direction.normalize();
        }

        if (!air_check) {
            hrp.write_velocity({ 0.0f, 0.0f, 0.0f });
        }
        else {
            hrp.write_velocity(direction * globals::rage::fly_speed * 10);
        }

        air_check = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void orbit() {
    static float angle = 0.0f;
    const float TWO_PI = 6.28318530718f;

    while (true) {
        globals::rage::orbitkeybind.update();

        if (!globals::rage::orbit || !globals::rage::orbitkeybind.enabled ||
            !globals::aim::aimbot_bind.enabled || !globals::aim::aimbot_target.address) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            continue;
        }

        try {
            auto local_character = globals::game::players.get_localpl().get_model_instance();
            if (!local_character) {
                continue;
            }

            auto local_root = rbx::model_instance_t(local_character).find_first_child("HumanoidRootPart");
            uintptr_t local_primitive = memory->read<uintptr_t>(local_root + Offsets::BasePart::Primitive);
            if (!local_primitive) {
                continue;
            }

            auto target_uppertorso = globals::aim::aimbot_target.find_first_child("UpperTorso");
            if (!target_uppertorso) {
                continue;
            }

            uintptr_t target_primitive = memory->read<uintptr_t>(target_uppertorso + Offsets::BasePart::Primitive);
            if (!target_primitive) {
                continue;
            }

            math::vector3_t target_pos = memory->read<math::vector3_t>(target_primitive + Offsets::BasePart::Position);

            angle += globals::rage::orbitspeed * 0.008f;
            if (angle > TWO_PI) {
                angle -= TWO_PI;
            }

            math::vector3_t new_local_pos;
            new_local_pos.x = target_pos.x + (globals::rage::orbitx * std::cos(angle));
            new_local_pos.y = target_pos.y + globals::rage::orbity;
            new_local_pos.z = target_pos.z + (globals::rage::orbitx * std::sin(angle));

            memory->write<math::vector3_t>(local_primitive + Offsets::BasePart::Position, new_local_pos);
            memory->write<math::vector3_t>(local_primitive + Offsets::BasePart::AssemblyAngularVelocity, math::vector3_t(0.0f, 0.0f, 0.0f));

        }
        catch (...) {
        }
        Sleep(1);
    }
}

void inf_jump() {
    while (true) {
        globals::rage::infinite_bind.update();
        if (!globals::rage::infinite_jump && !globals::rage::infinite_bind.enabled) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        try {
            auto lc = globals::game::players.get_localpl().get_model_instance();
            if (!rbx::model_instance_t(lc).address) {
                continue;
            }

            auto hrp = rbx::model_instance_t(lc).find_first_child("HumanoidRootPart");
            if (!rbx::model_instance_t(hrp).address) {
                continue;
            }

            if (globals::rage::infinite_bind.enabled && globals::rage::infinite_jump) {
                if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                    auto vel = rbx::part_t(hrp).get_velocity();
                    vel.y = globals::rage::infinite_power;
                    rbx::part_t(hrp).write_velocity(vel);
                }
            }
        }
        catch (...) {}
        Sleep(1);
    }
}

void anti_stomp() {
    while (true) {
        if (!globals::rage::antistomp) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        try {
            rbx::instance_t lp = globals::game::players.get_localpl().get_model_instance();
            rbx::instance_t bodyeffects = rbx::model_instance_t(lp).find_first_child("BodyEffects");

            bool IsKnocked = static_cast<rbx::value_holder_t<bool>>(bodyeffects.find_first_child("K.O")).get_value();
            if (IsKnocked) {
                rbx::humanoid_t(lp.find_first_child_by_class(("Humanoid"))).set_health(0.0f);
            }
        }
        catch (...) {}

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

void emote_changer() {
}

void freecam() {
}

void void_hide() {
}

void noclip() {
}

void dodge_bullets() {
}

void fakelag() {
}

void autoparry() {
}

void headless() {
}

void korblox() {
}

void no_jump_cooldown() {
}

void vehicle_fly() {
}

void hip_height_changer() {
    while (true) {
        globals::rage::hipheight_bind.update();

        if (!globals::rage::hipheight_enabled) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        rbx::humanoid_t local_humanoid(globals::game::local_player.humanoid.address);
        if (globals::rage::hipheight_bind.enabled) {
            local_humanoid.set_hip_height(globals::rage::hipheighrslider);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void anti_afk3() {
    while (true) {
        if (!globals::game::anti_afk) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        try {
            memory->write<float>(Offsets::Workspace::ForceNewAFKDuration, 999.0f);
        }
        catch (...) {}

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void thirdpersongay() {
    while (true) {
        globals::rage::third_personbind.update();
        if (globals::rage::third_person && globals::rage::third_personbind.enabled) {
            rbx::instance_t workspace = globals::game::workspace;
            rbx::instance_t camera = workspace.find_first_child("Camera");

            if (!camera.address) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

void rage::start_rage_thread()
{
    std::thread walkspeed_thread(rage::features::walkspeed);
    std::thread spinbot_thread(rage::features::spinbot);
    std::thread fly_thread(fly);
    std::thread jumppower_thread(jumppower);
    std::thread orbit_thread(orbit);
    std::thread inf_jump_thread(inf_jump);
    std::thread hip_height_thread(hip_height_changer);
    std::thread anti_stomp_thread(anti_stomp);
    std::thread anti_afk(anti_afk3);
    std::thread thirdperson(thirdpersongay);
    thirdperson.detach();
    anti_afk.detach();
    walkspeed_thread.detach();
    hip_height_thread.detach();
    spinbot_thread.detach();
    fly_thread.detach();
    jumppower_thread.detach();
    orbit_thread.detach();
    inf_jump_thread.detach();
    anti_stomp_thread.detach();
}