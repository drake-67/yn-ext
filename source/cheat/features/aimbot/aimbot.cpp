#include "aimbot.hpp"
#include "../../../globals/globals.h"
#include "../../../sdk/datamodel/part_t.hpp"
#include <Windows.h>
#include <thread>
#include <cmath>
#include "../../initialise.hpp"
#include "../../../sdk/offsets.hpp"

namespace aimbot {
    std::string current_lock = "";
    math::vector3_t world_target = { 0, 0, 0 };
    math::vector2_t screen_target = { 0, 0 };
    bool found = false;

    std::string sticky_target = "";
    bool is_sticky_locked = false;

    bool is_spectating = false;
    uintptr_t original_subject = 0;

    static math::matrix3_t LerpMatrix3(const math::matrix3_t& a, const math::matrix3_t& b, float t) {
        math::matrix3_t out;
        for (int i = 0; i < 9; ++i) out.data[i] = a.data[i] + (b.data[i] - a.data[i]) * t;
        return out;
    }

    static math::vector3_t Cross(const math::vector3_t& A, const math::vector3_t& B) {
        return { A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x };
    }

    static math::matrix3_t LookAtToMatrix(const math::vector3_t& camPos, const math::vector3_t& targetPos) {
        math::vector3_t forward = (targetPos - camPos).normalize();
        math::vector3_t right = Cross({ 0.f,1.f,0.f }, forward).normalize();
        math::vector3_t up = Cross(forward, right);

        math::matrix3_t m;
        m.data[0] = -right.x; m.data[1] = up.x; m.data[2] = -forward.x;
        m.data[3] = right.y;  m.data[4] = up.y; m.data[5] = -forward.y;
        m.data[6] = -right.z; m.data[7] = up.z; m.data[8] = -forward.z;
        return m;
    }

    void spectate_target() {
        if (!globals::aim::specate_target || !globals::aim::aimbot_target.address) {
            if (is_spectating) {
                rbx::instance_t workspace = globals::game::data_model.find_first_child_by_class("Workspace");
                rbx::instance_t camera = workspace.find_first_child("Camera");

                if (camera.address && original_subject) {
                    memory->write<std::uint64_t>(camera.address + Offsets::Camera::CameraSubject, original_subject);
                }

                is_spectating = false;
                original_subject = 0;
            }
            return;
        }

        rbx::instance_t workspace = globals::game::data_model.find_first_child_by_class("Workspace");
        rbx::instance_t camera = workspace.find_first_child("Camera");
        if (!camera.address) return;

        if (!is_spectating) {
            original_subject = memory->read<std::uint64_t>(camera.address + Offsets::Camera::CameraSubject);
            is_spectating = true;
        }

        rbx::instance_t target_model(globals::aim::aimbot_target.address);
        rbx::instance_t target_head = target_model.find_first_child("Head");

        if (target_head.address) {
            memory->write<std::uint64_t>(camera.address + Offsets::Camera::CameraSubject, target_head.address);
        }
    }






    void get_player_closest() {
        found = false;
        if (!globals::game::visual_engine.address) return;

        POINT p;
        if (!GetCursorPos(&p)) return;
        ScreenToClient(FindWindowA(0, "Roblox"), &p);

        float best = 999999.f;
        std::string name = "";
        uintptr_t target_character = 0;

        rbx::datamodel_t ws(globals::game::workspace.address);
        rbx::camera_t cam = ws.find_first_child("Camera");
        if (!cam.address) return;
        auto cam_pos = cam.get_camera_position();

        if (globals::aim::aimbot_sticky && is_sticky_locked && !sticky_target.empty()) {
            for (auto& plr : globals::game::player_cache) {
                if (plr.Name != sticky_target) continue;
                if (plr.localplayer || !plr.character.address || !plr.head.address) {
                    is_sticky_locked = false;
                    sticky_target = "";
                    globals::aim::aimbot_target = rbx::instance_t(0);
                    return;
                }

                std::vector<math::vector3_t> parts;
                if (plr.head.address) parts.push_back(rbx::part_t(plr.head.address).get_part_pos());
                if (plr.torso.address) parts.push_back(rbx::part_t(plr.torso.address).get_part_pos());
                if (plr.hrp.address) parts.push_back(rbx::part_t(plr.hrp.address).get_part_pos());
                if (parts.empty()) {
                    is_sticky_locked = false;
                    sticky_target = "";
                    globals::aim::aimbot_target = rbx::instance_t(0);
                    return;
                }

                int idx = globals::aim::hitpart_mode;
                if (idx >= parts.size()) idx = 0;
                if (idx == 4) idx = rand() % parts.size();

                math::vector3_t pos = parts[idx];
                if (isnan(pos.x)) {
                    is_sticky_locked = false;
                    sticky_target = "";
                    globals::aim::aimbot_target = rbx::instance_t(0);
                    return;
                }

                rbx::visualengine_t ve(globals::game::visual_engine.address);
                auto dims = ve.get_dimensions();
                auto vm = ve.get_viewmatrix();
                auto sp = ve.world_to_screen(pos, dims, vm);

                world_target = pos;
                screen_target = sp;
                current_lock = plr.Name;
                globals::aim::aimbot_target = rbx::instance_t(plr.character.address);
                found = true;
                return;
            }

            is_sticky_locked = false;
            sticky_target = "";
            globals::aim::aimbot_target = rbx::instance_t(0);
            return;
        }

        for (auto& plr : globals::game::player_cache) {
            if (plr.localplayer || !plr.character.address || !plr.head.address) continue;

            std::vector<math::vector3_t> parts;
            if (plr.head.address) parts.push_back(rbx::part_t(plr.head.address).get_part_pos());
            if (plr.torso.address) parts.push_back(rbx::part_t(plr.torso.address).get_part_pos());
            if (plr.hrp.address) parts.push_back(rbx::part_t(plr.hrp.address).get_part_pos());
            if (parts.empty()) continue;

            int idx = globals::aim::hitpart_mode;
            if (idx >= parts.size()) idx = 0;
            if (idx == 4) idx = rand() % parts.size();

            math::vector3_t pos = parts[idx];
            if (isnan(pos.x)) continue;

            float dist3d = (cam_pos - pos).magnitude();
            if (dist3d > 700.f) continue;

            rbx::visualengine_t ve(globals::game::visual_engine.address);
            auto dims = ve.get_dimensions();
            auto vm = ve.get_viewmatrix();
            auto sp = ve.world_to_screen(pos, dims, vm);
            if (sp.x < 0 || sp.y < 0 || sp.x > dims.x || sp.y > dims.y) continue;

            float dist2d = sqrt(pow(sp.x - p.x, 2) + pow(sp.y - p.y, 2));
            if (dist2d < best) {
                best = dist2d;
                world_target = pos;
                screen_target = sp;
                name = plr.Name;
                target_character = plr.character.address;
                found = true;
            }
        }

        if (found) {
            current_lock = name;
            globals::aim::aimbot_target = rbx::instance_t(target_character);
            if (globals::aim::aimbot_sticky && !is_sticky_locked) {
                sticky_target = name;
                is_sticky_locked = true;
            }
        }
    }

    void get_target() {
        if (!found) return;

        if (globals::aim::use_camera_mode) {
            rbx::datamodel_t ws(globals::game::workspace.address);
            rbx::camera_t cam = ws.find_first_child("Camera");
            if (!cam.address) return;

            auto cpos = cam.get_camera_position();
            auto tgt = world_target;

            tgt.x += globals::aim::offset_x;
            tgt.y += globals::aim::offset_y;
            tgt.z += globals::aim::offset_z;

            if (globals::aim::shake) {
                tgt.x += ((float)rand() / RAND_MAX * 2 - 1) * globals::aim::shake_x;
                tgt.y += ((float)rand() / RAND_MAX * 2 - 1) * globals::aim::shake_y;
                tgt.z += ((float)rand() / RAND_MAX * 2 - 1) * globals::aim::shake_z;
            }

            float smooth = globals::aim::camera_smoothness / 100.f;
            smooth = std::pow(smooth, 1.2f);

            math::matrix3_t targetMatrix = LookAtToMatrix(cpos, tgt);
            math::matrix3_t camMatrix = cam.get_camera_rotation();
            math::matrix3_t out = LerpMatrix3(camMatrix, targetMatrix, 1.f - smooth);

            cam.set_camera_rotation(out);
        }
        else {
            POINT c;
            if (!GetCursorPos(&c)) return;
            ScreenToClient(FindWindowA(0, "Roblox"), &c);

            float sens = globals::aim::mouse_sensitivity;
            if (globals::aim::mouse_smoothness > 0) {
                float sm = globals::aim::mouse_smoothness;
                if (sm < 1.f) sm = 1.f;
                if (sm > 100.f) sm = 100.f;
                sens /= sm;
            }

            float mx = (screen_target.x - c.x) * sens;
            float my = (screen_target.y - c.y) * sens;

            if (globals::aim::shake) {
                mx += ((float)rand() / RAND_MAX * 2 - 1) * globals::aim::shake_x;
                my += ((float)rand() / RAND_MAX * 2 - 1) * globals::aim::shake_y;
            }

            if (mx < -100.f) mx = -100.f;
            if (mx > 100.f) mx = 100.f;
            if (my < -100.f) my = -100.f;
            if (my > 100.f) my = 100.f;

            if (abs(mx) >= 1.f || abs(my) >= 1.f) {
                INPUT i = {};
                i.type = INPUT_MOUSE;
                i.mi.dx = (LONG)mx;
                i.mi.dy = (LONG)my;
                i.mi.dwFlags = MOUSEEVENTF_MOVE;
                SendInput(1, &i, sizeof(INPUT));
            }
        }
    }

    void initialize() {
        std::thread([]() {
            while (true) {
                if (globals::aim::aimbot) {
                    globals::aim::aimbot_bind.update();

                    if (globals::aim::aimbot_bind.enabled) {
                        get_player_closest();
                        get_target();
                        spectate_target();
                    }
                    else {
                        current_lock = "";
                        is_sticky_locked = false;
                        sticky_target = "";
                        globals::aim::aimbot_target = rbx::instance_t(0);
                        spectate_target();
                    }
                }
                else {
                    spectate_target();
                }
                Sleep(1);
            }
            }).detach();
    }
}