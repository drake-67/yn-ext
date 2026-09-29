#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <string>
#include <vector>
#include <chrono>
#include "../drawing/imgui/imgui.h"

namespace notifications {

    struct Notification {
        std::string message;
        std::chrono::steady_clock::time_point timestamp;
        float alpha;
        float offset_x;
        bool should_remove;

        Notification(const std::string& msg)
            : message(msg), alpha(0.0f), offset_x(-300.0f), should_remove(false) {
            timestamp = std::chrono::steady_clock::now();
        }
    };

    class NotificationManager {
    private:
        std::vector<Notification> notifications;
        const float display_duration = 3.0f;
        const float fade_duration = 0.3f;
        const float slide_duration = 0.4f;

    public:
        void add(const std::string& message) {
            notifications.emplace_back(message);
        }

        void render() {
            if (notifications.empty()) return;

            ImGuiIO& io = ImGui::GetIO();
            auto current_time = std::chrono::steady_clock::now();
            float dt = io.DeltaTime;

            float y_offset = io.DisplaySize.y - 40.0f;

            for (auto it = notifications.begin(); it != notifications.end();) {
                auto& notif = *it;
                float elapsed = std::chrono::duration<float>(current_time - notif.timestamp).count();

                if (elapsed < slide_duration) {
                    float slide_progress = elapsed / slide_duration;
                    slide_progress = slide_progress * slide_progress * (3.0f - 2.0f * slide_progress);
                    notif.offset_x = -300.0f + (300.0f * slide_progress);
                    notif.alpha = slide_progress;
                }
                else if (elapsed < display_duration) {
                    notif.offset_x = 0.0f;
                    notif.alpha = 1.0f;
                }
                else if (elapsed < display_duration + fade_duration) {
                    notif.offset_x = 0.0f;
                    float fade_progress = (elapsed - display_duration) / fade_duration;
                    notif.alpha = 1.0f - fade_progress;
                }
                else {
                    notif.should_remove = true;
                }

                if (notif.should_remove) {
                    it = notifications.erase(it);
                    continue;
                }

                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
                ImVec2 text_size = ImGui::CalcTextSize(notif.message.c_str());
                ImGui::PopFont();

                float padding_x = 12.0f;
                float padding_y = 6.0f;
                float bar_width = text_size.x + padding_x * 2.0f;
                float bar_height = text_size.y + padding_y * 2.0f;
                float top_line_thickness = 2.0f;

                ImVec2 bar_pos = ImVec2(20.0f + notif.offset_x, y_offset);
                ImVec2 bar_max = ImVec2(bar_pos.x + bar_width, bar_pos.y + bar_height);

                ImDrawList* draw_list = ImGui::GetForegroundDrawList();

                ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.95f * notif.alpha));
                ImU32 border_color = ImGui::ColorConvertFloat4ToU32(ImVec4(0.3f, 0.3f, 0.3f, 0.9f * notif.alpha));
                ImU32 top_line_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.8f * notif.alpha));
                ImU32 text_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, notif.alpha));

                draw_list->AddRectFilled(bar_pos, bar_max, bg_color, 0.0f);
                draw_list->AddRect(bar_pos, bar_max, border_color, 0.0f, 0, 1.0f);

                draw_list->AddLine(
                    ImVec2(bar_pos.x, bar_pos.y),
                    ImVec2(bar_max.x, bar_pos.y),
                    top_line_color,
                    top_line_thickness
                );

                ImVec2 text_pos = ImVec2(bar_pos.x + padding_x, bar_pos.y + padding_y);
                draw_list->AddText(text_pos, text_color, notif.message.c_str());

                y_offset -= (bar_height + 8.0f);

                ++it;
            }
        }

        void clear() {
            notifications.clear();
        }

        size_t count() const {
            return notifications.size();
        }
    };

    inline NotificationManager& get_manager() {
        static NotificationManager manager;
        return manager;
    }

    inline void add(const std::string& message) {
        get_manager().add(message);
    }

    inline void render() {
        get_manager().render();
    }

    inline void clear() {
        get_manager().clear();
    }

    inline size_t count() {
        return get_manager().count();
    }
}

#endif