#pragma once
#include <D3DX11.h>
#include "../drawing/imgui/imgui.h"
#include <string>
#include "../drawing/imgui/imgui_internal.h"
#include "../../globals/globals.h"
#include "../drawing/imgui/stbi_image.h"
#include <regex>
#include "../../utils/curl/curl.h"
#include "../../utils/json/json.hpp"

ID3D11ShaderResourceView* bgs = nullptr;

ID3D11ShaderResourceView* rust_model = nullptr;
ID3D11ShaderResourceView* rust_model_shape = nullptr;



bool HotkeyTEMP(CKeybind* bind, float custom_width)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(bind->get_name().c_str());
    char display[64] = "None";
    ImVec2 pos = window->DC.CursorPos + ImVec2(15.0f, 0); // move to the right
    ImVec2 display_size = ImGui::CalcTextSize(display, NULL, true);
    float width = custom_width == 0 ? 80.0f : custom_width; // smaller default width
    float height = ImGui::GetFontSize() + style.FramePadding.y * 2.0f;
    ImVec2 size(width, height);
    ImRect frame_bb(pos, pos + size);
    ImRect total_bb(pos, frame_bb.Max + ImVec2(size.x, 0));

    ImGui::ItemSize(total_bb);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered = ImGui::ItemHoverable(frame_bb, id, false);
    if (hovered)
    {
        ImGui::SetHoveredID(id);
        g.MouseCursor = ImGuiMouseCursor_Hand;
    }

    bool pressed = hovered && io.MouseClicked[0];
    bool right_click = hovered && io.MouseClicked[1];
    bool released_elsewhere = io.MouseClicked[0] && !hovered;

    if (pressed)
    {
        if (g.ActiveId != id)
        {
            memset(io.KeysDown, 0, sizeof(io.KeysDown));
            memset(io.MouseDown, 0, sizeof(io.MouseDown));
            bind->key = 0;
        }
        ImGui::SetActiveID(id, window);
        ImGui::FocusWindow(window);
        bind->waiting_for_input = true;
    }
    else if (released_elsewhere && g.ActiveId == id)
    {
        ImGui::ClearActiveID();
        bind->waiting_for_input = false;
    }
    else if (right_click)
        ImGui::OpenPopup(bind->get_name().c_str());

    bool changed = false;
    int key = bind->key;

    if (bind->waiting_for_input && g.ActiveId == id)
    {
        if (io.MouseClicked[0] && !hovered)
        {
            key = VK_LBUTTON;
            bind->waiting_for_input = false;
            ImGui::ClearActiveID();
            changed = true;
        }
        else
        {
            if (bind->set_key())
            {
                key = bind->key;
                bind->waiting_for_input = false;
                ImGui::ClearActiveID();
                changed = true;
            }
        }
    }

    bind->key = key;

    if (bind->waiting_for_input)
        strcpy_s(display, sizeof display, "...");
    else if (bind->key != 0)
        strcpy_s(display, sizeof display, bind->get_key_name().c_str());
    else
        strcpy_s(display, sizeof display, "None");

    window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_ChildBg), style.FrameRounding); // match child background
    if (style.FrameBorderSize > 0.0f)
    {
        window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, style.FrameBorderSize);
    }

    ImGui::RenderNavHighlight(frame_bb, id);

    ImVec2 text_pos = frame_bb.Min + (frame_bb.Max - frame_bb.Min) * 0.5f - ImGui::CalcTextSize(display, NULL, true) * 0.5f;
    window->DrawList->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), display);

    if (ImGui::BeginPopup(bind->get_name().c_str()))
    {
        if (ImGui::Selectable("Hold", bind->type == CKeybind::HOLD))
            bind->type = CKeybind::HOLD;
        if (ImGui::Selectable("Always", bind->type == CKeybind::ALWAYS))
            bind->type = CKeybind::ALWAYS;
        if (ImGui::Selectable("Toggle", bind->type == CKeybind::TOGGLE))
            bind->type = CKeybind::TOGGLE;
        ImGui::EndPopup();
    }
    return changed;
}

auto p = ImVec2{};
static int tab;
static float m_anim = 0.f;

float rounding = 0.f;
ImVec2 menu_size{ 650, 450 };
bool aimbot_check;
bool aimbot_check1;
bool aimbot_check2;
bool aimbot_check3;
bool aimbot_check4;
float aimbot_check44 = 1.0f;

int combos = 1;
static int current_item = 0;

static bool chams = false;

void RenderTabSwitcher(ImDrawList* draw_list, ImVec2 pos, const char** labels, int label_count, int& selected_index)
{
    float width = 290.0f;
    float height = 40.0f;
    float tab_padding = 10.0f;

    ImVec2 box_min = ImVec2(pos.x, pos.y);
    ImVec2 box_max = ImVec2(pos.x + width, pos.y + height);
    draw_list->AddRectFilled(box_min, box_max, ImColor(14, 14, 14, 255));
    draw_list->AddRect(box_min, box_max, ImColor(34, 34, 34, 255), 0, 0, 1.0f);
    draw_list->AddRect(box_min, box_max, ImColor(34, 34, 34, 255), 0, 0, 1.0f);

    float total_label_width = 0;
    for (int i = 0; i < label_count; ++i)
    {
        total_label_width += ImGui::CalcTextSize(labels[i]).x + tab_padding * 2;
    }

    float start_x = pos.x + (width - total_label_width) / 2.0f;

    ImVec2 cursor_pos = ImVec2(start_x, pos.y);

    for (int i = 0; i < label_count; ++i)
    {
        const char* label = labels[i];
        ImVec2 text_size = ImGui::CalcTextSize(label);
        ImVec2 text_pos = ImVec2(cursor_pos.x + tab_padding, pos.y + (height / 2) - (text_size.y / 2));

        bool is_selected = (i == selected_index);
        bool is_hovered = false;
        ImU32 text_color = is_selected ? ImColor(255, 255, 255, 255) : ImColor(200, 200, 200, 255);
        ImU32 underline_color = ImColor(130, 179, 255, 255);

        ImGui::SetCursorScreenPos(cursor_pos);
        ImGui::InvisibleButton(label, ImVec2(text_size.x + tab_padding * 2, height));
        if (ImGui::IsItemClicked())
        {
            selected_index = i;
        }
        if (ImGui::IsItemHovered())
        {
            is_hovered = true;
            text_color = ImColor(255, 255, 255, 255);
        }

        draw_list->AddText(text_pos, text_color, label);

        if (is_selected)
        {
            ImVec2 underline_min = ImVec2(text_pos.x, text_pos.y + text_size.y + 2);
            ImVec2 underline_max = ImVec2(text_pos.x + text_size.x, underline_min.y + 2);
            draw_list->AddRectFilled(underline_min, underline_max, underline_color);
        }

        cursor_pos.x += text_size.x + tab_padding * 2;
    }
}


void TextCentered(std::string text, ImColor startColor, ImColor endColor, float add_x = 0, float add_y = 0, bool should_anim = false) {
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());

    float horizontalIndentation = (windowSize.x - textSize.x) * 0.5f;
    float verticalIndentation = (windowSize.y - textSize.y) * 0.5f;

    float minHorizontalIndentation = 20.0f;
    if (horizontalIndentation <= minHorizontalIndentation) {
        horizontalIndentation = minHorizontalIndentation;
    }

    float minVerticalIndentation = 20.0f;
    if (verticalIndentation <= minVerticalIndentation) {
        verticalIndentation = minVerticalIndentation;
    }

    ImGui::SetCursorPosY(verticalIndentation + add_y);
    ImGui::SetCursorPosX(horizontalIndentation + add_x);
    ImGui::PushTextWrapPos(windowSize.x - horizontalIndentation);
    if (should_anim) {
        float time = ImGui::GetTime();
        float frequency = -2.0f;

        for (int i = 0; i < text.size(); i++) {
            ImVec4 interpolatedColor;
            interpolatedColor.x = ImLerp(startColor.Value.x, endColor.Value.x, 0.5f + 0.5f * sinf(time * frequency + i * 0.3f));
            interpolatedColor.y = ImLerp(startColor.Value.y, endColor.Value.y, 0.5f + 0.5f * sinf(time * frequency + i * 0.3f));
            interpolatedColor.z = ImLerp(startColor.Value.z, endColor.Value.z, 0.5f + 0.5f * sinf(time * frequency + i * 0.3f));
            interpolatedColor.w = 1.0f;

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(interpolatedColor.x, interpolatedColor.y, interpolatedColor.z, interpolatedColor.w));
            ImGui::TextUnformatted(text.substr(i, 1).c_str());
            ImGui::SameLine(0, 0);
            ImGui::PopStyleColor();
        }
    }
    else {
        ImGui::SetCursorPosX(horizontalIndentation + add_x);
        ImGui::TextWrapped(text.c_str());
    }

    ImGui::PopTextWrapPos();
}

struct ImageCacheEntry {
    std::vector<unsigned char> image_data;
    ID3D11ShaderResourceView* texture;
};

std::unordered_map<std::string, ImageCacheEntry> image_cache;
std::unordered_map<std::string, ImageCacheEntry> image_profile_cache;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    ((std::vector<unsigned char>*)userp)->insert(((std::vector<unsigned char>*)userp)->end(),
        (unsigned char*)contents, (unsigned char*)contents + total_size);
    return total_size;
}

std::vector<unsigned char> HttpGetBinary(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::vector<unsigned char> dataBuffer;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // SSL verification bypass fix
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dataBuffer);
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            dataBuffer.clear();
        }

        curl_easy_cleanup(curl);
    }
    else {
       
    }

    return dataBuffer;
}
std::vector<unsigned char> get_profile_user_image(const std::string& user_id) {
    std::string profile_url = "https://www.roblox.com/users/" + user_id + "/profile";

    std::vector<unsigned char> html_data = HttpGetBinary(profile_url);
    if (html_data.empty()) {
        return {};
    }

    std::string html(html_data.begin(), html_data.end());
    std::regex img_regex(R"(<meta property=\"og:image\" content=\"(https://tr\.rbxcdn\.com/[^\"]+)\")");
    std::smatch match;
    std::string image_url;

    if (std::regex_search(html, match, img_regex) && match.size() > 1) {
        image_url = match[1].str();
        return HttpGetBinary(image_url);
    }

    return {};
}


std::vector<unsigned char> get_user_image(const std::string& user_id) {
    std::string headshot_url = "https://thumbnails.roblox.com/v1/users/avatar-headshot?userIds=" + user_id + "&size=48x48&format=Png&isCircular=false";

    std::vector<unsigned char> json_data = HttpGetBinary(headshot_url);
    if (json_data.empty()) {
        return {};
    }

    std::string json_string(json_data.begin(), json_data.end());

    try {
        auto json_response = nlohmann::json::parse(json_string);
        std::string image_url;

        if (json_response.contains("data") && json_response["data"].is_array() && !json_response["data"].empty()) {
            auto user_data = json_response["data"][0];
            if (user_data.contains("state") && user_data["state"] == "Completed") {
                image_url = user_data["imageUrl"].get<std::string>();
            }
            else {
                return {};
            }
        }
        else {
            return {};
        }

        return HttpGetBinary(image_url);
    }
    catch (...) {
        return {};
    }
}

ID3D11ShaderResourceView* LoadTextureFromMemory(ID3D11Device* device, const std::vector<unsigned char>& image_data) {
    int width, height, channels;
    unsigned char* image_pixels = stbi_load_from_memory(image_data.data(), image_data.size(), &width, &height, &channels, 4);

    if (!image_pixels) {
        return nullptr;
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = image_pixels;
    initData.SysMemPitch = width * 4;

    ID3D11Texture2D* pTexture = nullptr;
    HRESULT hr = device->CreateTexture2D(&desc, &initData, &pTexture);
    if (FAILED(hr)) {
        stbi_image_free(image_pixels);
        return nullptr;
    }

    ID3D11ShaderResourceView* textureView = nullptr;
    hr = device->CreateShaderResourceView(pTexture, nullptr, &textureView);
    pTexture->Release();
    stbi_image_free(image_pixels);

    if (FAILED(hr)) {
        return nullptr;
    }

    return textureView;
}

void RenderUserImage(ID3D11Device* device, const std::string& user_id) {
    if (image_cache.find(user_id) == image_cache.end()) {
        std::vector<unsigned char> image_data = get_user_image(user_id);
        if (!image_data.empty()) {
            ImageCacheEntry entry;
            entry.image_data = image_data;
            entry.texture = LoadTextureFromMemory(device, image_data);
            image_cache[user_id] = entry;
        }
    }

    ImTextureID texture_id = (ImTextureID)image_cache[user_id].texture;
}

void RenderProfileImage(ID3D11Device* device, const std::string& user_id) {
    if (image_profile_cache.find(user_id) == image_profile_cache.end()) {
        std::vector<unsigned char> image_data = get_profile_user_image(user_id);
        if (!image_data.empty()) {
            ImageCacheEntry entry;
            entry.image_data = image_data;
            entry.texture = LoadTextureFromMemory(device, image_data);
            image_profile_cache[user_id] = entry;
        }
    }

    ImTextureID texture_id = (ImTextureID)image_profile_cache[user_id].texture;
}



D3DX11_IMAGE_LOAD_INFO info = {};
ID3DX11ThreadPump* pump = nullptr;
ID3D11ShaderResourceView* Imagine = nullptr;
ID3D11ShaderResourceView* basicbacon = nullptr;
ID3D11ShaderResourceView* esppreview = nullptr;
ID3D11ShaderResourceView* espprevi_skeletona = nullptr;

ID3D11ShaderResourceView* foldera = nullptr;
ID3D11ShaderResourceView* cameraa = nullptr;
ID3D11ShaderResourceView* partaa = nullptr;
ID3D11ShaderResourceView* lightninga = nullptr;
ID3D11ShaderResourceView* humanoida = nullptr;

ID3D11ShaderResourceView* playersa = nullptr;
ID3D11ShaderResourceView* playera = nullptr;

ID3D11ShaderResourceView* meshparta = nullptr;
ID3D11ShaderResourceView* modela = nullptr;
ID3D11ShaderResourceView* terraina = nullptr;
ID3D11ShaderResourceView* localscripta = nullptr;
ID3D11ShaderResourceView* localscriptsa = nullptr;
ID3D11ShaderResourceView* playerguia = nullptr;
ID3D11ShaderResourceView* statsa = nullptr;
ID3D11ShaderResourceView* guiservicea = nullptr;
ID3D11ShaderResourceView* videocapturea = nullptr;
ID3D11ShaderResourceView* runservicea = nullptr;
ID3D11ShaderResourceView* framea = nullptr;
ID3D11ShaderResourceView* csda = nullptr;
ID3D11ShaderResourceView* contentprovidera = nullptr;
ID3D11ShaderResourceView* caractera = nullptr;

ID3D11ShaderResourceView* nonreplicateda = nullptr;
ID3D11ShaderResourceView* startergeara = nullptr;
ID3D11ShaderResourceView* timerdevicea = nullptr;
ID3D11ShaderResourceView* backpacka = nullptr;
ID3D11ShaderResourceView* marketplaceservicea = nullptr;

ID3D11ShaderResourceView* soundservicea = nullptr;
ID3D11ShaderResourceView* logservicea = nullptr;
ID3D11ShaderResourceView* statsitema = nullptr;
ID3D11ShaderResourceView* boolvaluea = nullptr;
ID3D11ShaderResourceView* intvaluea = nullptr;
ID3D11ShaderResourceView* doubletypea = nullptr;
ID3D11ShaderResourceView* anicentlogo = nullptr;