#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iostream>
#include "../drawing/imgui/settings/functions.h"
#include "../../globals/globals.h"
std::string get_current_time_date()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	char buf[64];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
	return std::string(buf);
}

auto checkbox_with_color = [&](const char* label, bool* toggle, float* color, const char* color_id) {
	ImVec2 pos = ImGui::GetCursorPos();
	gui->checkbox(label, toggle);
	if (color) {
		ImGui::SameLine(140.0f);
		ImGui::SetCursorPosY(pos.y);
		gui->color_edit(color_id, color);

	}
	};
auto checkbox_with_colordouble = [&](const char* label, bool* toggle, float* color, float* color2, const char* color_id, const char* color_id2) {
	ImVec2 pos = ImGui::GetCursorPos();
	gui->checkbox(label, toggle);
	if (color) {
		ImGui::SameLine(140.0f);
		ImGui::SetCursorPosY(pos.y);
		gui->color_edit(color_id, color);
		ImGui::SameLine(140.0f);
		ImGui::SetCursorPosY(pos.y);
		gui->color_edit(color_id, color2);
	}
	};

void c_gui::render()
{


}