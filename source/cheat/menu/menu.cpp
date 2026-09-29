#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#define _CRT_SECURE_NO_WARNINGS
#include "../drawing/imgui/imgui.h"
#include "../overlay/overlay.hpp"
#include "../drawing/drawing.hpp"
#include <algorithm>
#include <D3DX11tex.h>
#include "../drawing/imgui/imgui_internal.h"
#include <string>
#include "../../globals/globals.h"
#include "../drawing/lua/luau.hpp"





static std::string convert_ip_to_string(uint64_t raw_ip)
{
    uint32_t ip = static_cast<uint32_t>(raw_ip);  // Use lower 32 bits
    //ip = ntohl(ip); // convert from network to host byte order

    std::stringstream ss;
    ss << ((ip >> 24) & 0xFF) << "."
        << ((ip >> 16) & 0xFF) << "."
        << ((ip >> 8) & 0xFF) << "."
        << (ip & 0xFF);
    return ss.str();
}




std::string get_pc_name() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return std::string(buffer);
    }
    return "pc";
}


void gui_t::menu() {

}

void gui_t::render_watermark()
{


}
