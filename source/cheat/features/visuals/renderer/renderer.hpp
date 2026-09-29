#pragma once
#define IMGUI_MATH_DEFINE_OPERATORS
#include <string>
#include <cmath>
#include "../../../drawing/imgui/imgui.h"
#include "../../../drawing/imgui/imgui_internal.h"

inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }
inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }


struct renderer_e
{

    static void outlined_rect(const ImVec2& pos, const ImVec2& size, ImU32 col, float rounding = 0.f) {
        ImVec2 rounded_pos = ImVec2(std::round(pos.x), std::round(pos.y));
        ImVec2 rounded_size = ImVec2(std::round(size.x), std::round(size.y));
        auto draw = ImGui::GetBackgroundDrawList();
        ImRect rect_bb(rounded_pos, ImVec2(rounded_pos.x + rounded_size.x, rounded_pos.y + rounded_size.y));

        if (globals::visuals::box_fill) {
            if (globals::visuals::box_gradient) {
                ImU32 top_color = ImGui::ColorConvertFloat4ToU32(ImVec4(globals::visuals::colors::box_gradient_top[0], globals::visuals::colors::box_gradient_top[1], globals::visuals::colors::box_gradient_top[2], globals::visuals::colors::box_gradient_top[3]));
                ImU32 bottom_color = ImGui::ColorConvertFloat4ToU32(ImVec4(globals::visuals::colors::box_gradient_bottom[0], globals::visuals::colors::box_gradient_bottom[1], globals::visuals::colors::box_gradient_bottom[2], globals::visuals::colors::box_gradient_bottom[3]));

                if (globals::visuals::box_gradient_autorotate) {
                    float time = ImGui::GetTime();
                    float rotation_speed = globals::visuals::box_gradient_rotation_speed;
                    float angle = time * rotation_speed;
                    ImVec2 center = ImVec2((rect_bb.Min.x + rect_bb.Max.x) * 0.5f, (rect_bb.Min.y + rect_bb.Max.y) * 0.5f);
                    float half_width = (rect_bb.Max.x - rect_bb.Min.x) * 0.5f;
                    float half_height = (rect_bb.Max.y - rect_bb.Min.y) * 0.5f;
                    float cos_a = cosf(angle);
                    float sin_a = sinf(angle);
                    float corners[4];
                    corners[0] = (cos_a * (-half_width) + sin_a * (-half_height)) / sqrtf(half_width * half_width + half_height * half_height);
                    corners[1] = (cos_a * (half_width)+sin_a * (-half_height)) / sqrtf(half_width * half_width + half_height * half_height);
                    corners[2] = (cos_a * (half_width)+sin_a * (half_height)) / sqrtf(half_width * half_width + half_height * half_height);
                    corners[3] = (cos_a * (-half_width) + sin_a * (half_height)) / sqrtf(half_width * half_width + half_height * half_height);

                    for (int i = 0; i < 4; i++) {
                        corners[i] = (corners[i] + 1.0f) * 0.5f;
                    }

                    ImU32 corner_colors[4];
                    for (int i = 0; i < 4; i++) {
                        float t = corners[i];
                        ImVec4 top_vec = ImGui::ColorConvertU32ToFloat4(top_color);
                        ImVec4 bottom_vec = ImGui::ColorConvertU32ToFloat4(bottom_color);
                        ImVec4 lerped = ImVec4(
                            top_vec.x + t * (bottom_vec.x - top_vec.x),
                            top_vec.y + t * (bottom_vec.y - top_vec.y),
                            top_vec.z + t * (bottom_vec.z - top_vec.z),
                            top_vec.w + t * (bottom_vec.w - top_vec.w)
                        );
                        corner_colors[i] = ImGui::ColorConvertFloat4ToU32(lerped);
                    }

                    draw->AddRectFilledMultiColor(rect_bb.Min, rect_bb.Max,
                        corner_colors[0], corner_colors[1], corner_colors[2], corner_colors[3]);
                }
                else {
                    draw->AddRectFilledMultiColor(rect_bb.Min, rect_bb.Max, top_color, top_color, bottom_color, bottom_color);
                }
            }
            else {
                draw->AddRectFilled(rect_bb.Min, rect_bb.Max, ImGui::ColorConvertFloat4ToU32(ImVec4(globals::visuals::colors::box_color[0], globals::visuals::colors::box_color[1], globals::visuals::colors::box_color[2], globals::visuals::colors::box_color[3])));
            }
        }

        if (globals::visuals::box_outline) {
            draw->AddRect(rect_bb.Min, rect_bb.Max, IM_COL32(0, 0, 0, col >> 24), rounding);
            draw->AddRect(ImVec2(rect_bb.Min.x - 2.f, rect_bb.Min.y - 2.f), ImVec2(rect_bb.Max.x + 2.f, rect_bb.Max.y + 2.f), IM_COL32(0, 0, 0, col >> 24), rounding);
        }

        draw->AddRect(ImVec2(rect_bb.Min.x - 1.f, rect_bb.Min.y - 1.f), ImVec2(rect_bb.Max.x + 1.f, rect_bb.Max.y + 1.f), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::visuals::colors::box_outline_color[0], globals::visuals::colors::box_outline_color[1], globals::visuals::colors::box_outline_color[2], 1)), rounding);
    }


    static void armorbar_positioned(
        const ImVec2& box_pos,
        const ImVec2& box_size,
        float armor,
        float max_armor,
        ImU32 col,
        globals::visuals::ESP_SIDE side,
        bool is_vertical,
        float padding = 1.0f,
        float thickness = 4.0f,
        bool use_outline = true,
        ImU32 outline_col = IM_COL32(0, 0, 0, 255))
    {
        auto draw = ImGui::GetBackgroundDrawList();
        draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;

        float ratio = (max_armor > 0.f) ? armor / max_armor : 0.f;
        ratio = std::clamp(ratio, 0.f, 1.f);

        ImVec2 bar_pos, bar_size;
        const float gap = 5.0f;

        if (is_vertical) {
            bar_size = ImVec2(thickness, box_size.y + 2.0f);
            if (side == globals::visuals::ESP_SIDE::LEFT) {
                bar_pos = ImVec2(box_pos.x - gap - thickness, box_pos.y - 1.0f);
            }
            else {
                bar_pos = ImVec2(box_pos.x + box_size.x + gap, box_pos.y - 1.0f);
            }
        }
        else {
            bar_size = ImVec2(box_size.x + 2.0f, thickness);
            if (side == globals::visuals::ESP_SIDE::TOP) {
                bar_pos = ImVec2(box_pos.x - 1.0f, box_pos.y - gap - thickness);
            }
            else {
                bar_pos = ImVec2(box_pos.x - 1.0f, box_pos.y + box_size.y + gap);
            }
        }

        float bar_fill_length = (is_vertical) ? (bar_size.y * ratio) : (bar_size.x * ratio);
        ImVec2 fill_min, fill_max;

        if (is_vertical) {
            fill_min = ImVec2(bar_pos.x, bar_pos.y + bar_size.y - bar_fill_length);
            fill_max = ImVec2(bar_pos.x + bar_size.x, bar_pos.y + bar_size.y);
        }
        else {
            fill_min = bar_pos;
            fill_max = ImVec2(bar_pos.x + bar_fill_length, bar_pos.y + bar_size.y);
        }

        if (use_outline) {
            ImVec2 outline_min = ImVec2(bar_pos.x - 1.0f, bar_pos.y - 1.0f);
            ImVec2 outline_max = ImVec2(bar_pos.x + bar_size.x + 1.0f, bar_pos.y + bar_size.y + 1.0f);
            draw->AddRectFilled(outline_min, outline_max, outline_col);
        }

        ImU32 top_armor_color = IM_COL32(135, 206, 250, 255);    
        ImU32 bottom_armor_color = IM_COL32(0, 100, 255, 255);    

        if (is_vertical) {
            draw->AddRectFilledMultiColor(
                fill_min, fill_max,
                top_armor_color, top_armor_color,
                bottom_armor_color, bottom_armor_color
            );
        }
        else {
            draw->AddRectFilledMultiColor(
                fill_min, fill_max,
                top_armor_color, bottom_armor_color,
                bottom_armor_color, top_armor_color
            );
        }
    }

static void healthbar_positioned(
    const ImVec2& box_pos,
    const ImVec2& box_size,
    float health,
    float max_health,
    ImU32 col,
    globals::visuals::ESP_SIDE side,
    bool is_vertical,
    float padding = 1.0f,
    float thickness = 4.0f,  
    bool use_outline = true,
    ImU32 outline_col = IM_COL32(0, 0, 0, 255))
{
    auto draw = ImGui::GetBackgroundDrawList();
    draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;
    
    float ratio = (max_health > 0.f) ? health / max_health : 0.f;
    ratio = std::clamp(ratio, 0.f, 1.f);
    
    ImVec2 bar_pos, bar_size;
    const float gap = 5.0f;  
    
    if (is_vertical) {
        bar_size = ImVec2(thickness, box_size.y + 2.0f);
        if (side == globals::visuals::ESP_SIDE::LEFT) {
            bar_pos = ImVec2(box_pos.x - gap - thickness, box_pos.y - 1.0f);
        }
        else {
            bar_pos = ImVec2(box_pos.x + box_size.x + gap, box_pos.y - 1.0f);
        }
    }
    else {
        bar_size = ImVec2(box_size.x + 2.0f, thickness);
        if (side == globals::visuals::ESP_SIDE::TOP) {
            bar_pos = ImVec2(box_pos.x - 1.0f, box_pos.y - gap - thickness);
        }
        else {
            bar_pos = ImVec2(box_pos.x - 1.0f, box_pos.y + box_size.y + gap);
        }
    }
    
    float bar_fill_length = (is_vertical) ? (bar_size.y * ratio) : (bar_size.x * ratio);
    ImVec2 fill_min, fill_max;
    
    if (is_vertical) {
        fill_min = ImVec2(bar_pos.x, bar_pos.y + bar_size.y - bar_fill_length);
        fill_max = ImVec2(bar_pos.x + bar_size.x, bar_pos.y + bar_size.y);
    }
    else {
        fill_min = bar_pos;
        fill_max = ImVec2(bar_pos.x + bar_fill_length, bar_pos.y + bar_size.y);
    }
    
    if (use_outline) {
        ImVec2 outline_min = ImVec2(bar_pos.x - 1.0f, bar_pos.y - 1.0f);
        ImVec2 outline_max = ImVec2(bar_pos.x + bar_size.x + 1.0f, bar_pos.y + bar_size.y + 1.0f);
        draw->AddRectFilled(outline_min, outline_max, outline_col);
    }
    
    ImU32 top_health_color = IM_COL32(250, 246, 248, 255);
    ImU32 bottom_health_color = IM_COL32(182, 114, 133, 255);
    
    if (is_vertical) {
        draw->AddRectFilledMultiColor(
            fill_min, fill_max,
            top_health_color, top_health_color,
            bottom_health_color, bottom_health_color
        );
    }
    else {
        draw->AddRectFilledMultiColor(
            fill_min, fill_max,
            top_health_color, bottom_health_color,
            bottom_health_color, top_health_color
        );
    }
}

#define vec_ref const ImVec2&
#define decl static auto

decl text_Voided(vec_ref pos, const std::string& text, ImU32 col, ImFont* font, float font_size = 13.0f)
{
    font_size = std::clamp(font_size, 10.0f, 18.0f);

    auto alpha = col >> 24;

    if (alpha > 0)
    {
        auto* draw = ImGui::GetBackgroundDrawList();
        ImU32 shadow_col = IM_COL32(0, 0, 0, alpha);

        draw->AddText(font, font_size, pos + ImVec2(-1.f, 1.f), shadow_col, text.c_str());
        draw->AddText(font, font_size, pos + ImVec2(1.f, -1.f), shadow_col, text.c_str());
        draw->AddText(font, font_size, pos + ImVec2(1.f, 1.f), shadow_col, text.c_str());
        draw->AddText(font, font_size, pos + ImVec2(-1.f, -1.f), shadow_col, text.c_str());
        draw->AddText(font, font_size, pos + ImVec2(0.f, 1.f), shadow_col, text.c_str());
        draw->AddText(font, font_size, pos + ImVec2(0.f, -1.f), shadow_col, text.c_str());
        draw->AddText(font, font_size, pos + ImVec2(1.f, 0.f), shadow_col, text.c_str());
        draw->AddText(font, font_size, pos + ImVec2(-1.f, 0.f), shadow_col, text.c_str());
    }

    ImGui::GetBackgroundDrawList()->AddText(font, font_size, pos, col, text.c_str());
}

};