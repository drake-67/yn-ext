#define STB_IMAGE_IMPLEMENTATION
#define IMGUI_DEFINE_MATH_OPERATORS
#define _CRT_SECURE_NO_WARNINGS
#include "../overlay/overlay.hpp"
#include <dwmapi.h>
#include <chrono>
#include <thread>
#include <d3d11.h>
#include <wincodec.h>
#include <vector>
#pragma comment(lib, "windowscodecs.lib")
#include "../drawing/drawing.hpp"
#include "../features/visuals/visuals.hpp"
#include "../drawing/imgui/imgui_internal.h"
#include "../menu/menu.hpp"
#include "../../globals/globals.h"
#include "../drawing/images/esp_preview.h"
#include "../../sdk/datamodel/humanoid.hpp"
#include "../drawing/lua/luau.hpp"
#include "../drawing/images/iconsdex.h"
#include "../../sdk/datamodel/part_t.hpp"
#include "../drawing/fonts/arial.h"
#include "../drawing/imgui/addons/addons.hpp"
#include <lmcons.h>
#include "../drawing/fonts/verdane.h"
#include "../../utils/key/keybind.h"
#include "../drawing/imgui/backends/TextEditor.h"
#include "../drawing/imgui/settings/functions.h"
#include "../drawing/imgui/data/fonts.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

// D3DX11-free texture loader (stb first, WIC fallback for WEBP/PNG).
// Replaces CreateTextureFromMemory so the project builds
// without the legacy June-2010 DirectX SDK.
static HRESULT CreateTextureFromMemory(ID3D11Device* device, const void* data, size_t size, ID3D11ShaderResourceView** out) {
    if (!device || !data || !size || !out) return E_INVALIDARG;
    *out = nullptr;
    int w = 0, h = 0;
    unsigned char* px = stbi_load_from_memory((const stbi_uc*)data, (int)size, &w, &h, nullptr, 4);
    if (px && w > 0 && h > 0) {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = (UINT)w; desc.Height = (UINT)h;
        desc.MipLevels = 1; desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA init = {};
        init.pSysMem = px; init.SysMemPitch = (UINT)(w * 4);
        ID3D11Texture2D* tex = nullptr;
        HRESULT hr = device->CreateTexture2D(&desc, &init, &tex);
        stbi_image_free(px);
        if (FAILED(hr)) return hr;
        HRESULT hr2 = device->CreateShaderResourceView(tex, nullptr, out);
        tex->Release();
        return hr2;
    }
    if (px) stbi_image_free(px);
    // WIC fallback (handles PNG/JPEG/WEBP via OS codecs)
    IWICImagingFactory* factory = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return hr;
    IWICStream* stream = nullptr;
    hr = factory->CreateStream(&stream);
    if (SUCCEEDED(hr)) hr = stream->InitializeFromMemory((WICInProcPointer)(BYTE*)data, (DWORD)size);
    IWICBitmapDecoder* decoder = nullptr;
    if (SUCCEEDED(hr)) hr = factory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnDemand, &decoder);
    IWICBitmapFrameDecode* frame = nullptr;
    if (SUCCEEDED(hr)) hr = decoder->GetFrame(0, &frame);
    IWICFormatConverter* conv = nullptr;
    if (SUCCEEDED(hr)) hr = factory->CreateFormatConverter(&conv);
    if (SUCCEEDED(hr)) hr = conv->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    UINT fw = 0, fh = 0;
    if (SUCCEEDED(hr)) hr = conv->GetSize(&fw, &fh);
    std::vector<BYTE> buf;
    if (SUCCEEDED(hr)) {
        buf.resize((size_t)fw * fh * 4);
        hr = conv->CopyPixels(nullptr, fw * 4, (UINT)buf.size(), buf.data());
    }
    if (SUCCEEDED(hr)) {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = fw; desc.Height = fh;
        desc.MipLevels = 1; desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA init = {};
        init.pSysMem = buf.data(); init.SysMemPitch = fw * 4;
        ID3D11Texture2D* tex = nullptr;
        hr = device->CreateTexture2D(&desc, &init, &tex);
        if (SUCCEEDED(hr)) { hr = device->CreateShaderResourceView(tex, nullptr, out); tex->Release(); }
    }
    if (conv) conv->Release();
    if (frame) frame->Release();
    if (decoder) decoder->Release();
    if (stream) stream->Release();
    factory->Release();
    return hr;
}

void game_explorer()
{
	static std::optional<rbx::instance_t> selected_instance;
	static std::uint64_t selected_instance_address = 0;

	static std::string selectedNodeName = "";
	static std::string selectedNodeClassName = "";
	static std::string selectedNodeHexAddress = "";
	static std::string selectedNodePath = "";
	std::function<void(rbx::instance_t&, int, const std::string&)> drawNode;

	drawNode = [&drawNode](rbx::instance_t& node, int indentLevel, const std::string& currentPath) -> void {
		std::string nodeName = node.get_name();
		std::string nodeClassName = node.get_class_name();
		std::vector<rbx::instance_t> children = node.get_children();
		bool hasChildren = !children.empty();

		std::stringstream uniqueIdStream;
		uniqueIdStream << std::hex << node.address;
		std::string uniqueId = uniqueIdStream.str();

		ImGui::SetCursorPosX(25.0f * indentLevel);

		std::string fullPath = currentPath.empty() ? nodeName : currentPath + "." + nodeName;

		if (nodeClassName == "Workspace")
		{
			ImGui::Image((ImTextureID)Imagine, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel);

		}

		if (nodeClassName == "Folder")
		{
			ImGui::Image((ImTextureID)foldera, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.2);
		}
		if (nodeClassName == "Camera")
		{
			ImGui::Image((ImTextureID)cameraa, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.2);
		}
		if (nodeClassName == "ReplicatedStorage")
		{
			ImGui::Image((ImTextureID)cameraa, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.2);
		}
		if (nodeClassName == "Lighting")
		{
			ImGui::Image((ImTextureID)lightninga, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "Humanoid")
		{
			ImGui::Image((ImTextureID)humanoida, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.2);
		}
		if (nodeClassName == "Part")
		{
			ImGui::Image((ImTextureID)partaa, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.2);
		}
		if (nodeClassName == "Players")
		{
			ImGui::Image((ImTextureID)playersa, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "Frame")
		{
			ImGui::Image((ImTextureID)framea, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "Backpack")
		{
			ImGui::Image((ImTextureID)backpacka, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}

		if (nodeClassName == "Backpack")
		{
			ImGui::Image((ImTextureID)backpacka, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}

		if (nodeClassName == "BoolValue")
		{
			ImGui::Image((ImTextureID)boolvaluea, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "ContentProvider")
		{
			ImGui::Image((ImTextureID)contentprovidera, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "CSGDictionaryService")
		{
			ImGui::Image((ImTextureID)csda, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "GuiService")
		{
			ImGui::Image((ImTextureID)guiservicea, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "IntValue")
		{
			ImGui::Image((ImTextureID)intvaluea, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "LocalScript")
		{
			ImGui::Image((ImTextureID)localscripta, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "LogService")
		{
			ImGui::Image((ImTextureID)logservicea, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "MarketplaceService")
		{
			ImGui::Image((ImTextureID)marketplaceservicea, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "MeshPart")
		{
			ImGui::Image((ImTextureID)meshparta, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "Model")
		{
			ImGui::Image((ImTextureID)modela, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "NonReplicatedCSGDictionaryService")
		{
			ImGui::Image((ImTextureID)nonreplicateda, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "Player")
		{
			ImGui::Image((ImTextureID)playera, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "PlayerGui")
		{
			ImGui::Image((ImTextureID)playerguia, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "RunService")
		{
			ImGui::Image((ImTextureID)runservicea, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}

		if (nodeClassName == "SoundService")
		{
			ImGui::Image((ImTextureID)soundservicea, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "StarterGear")
		{
			ImGui::Image((ImTextureID)startergeara, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "Stats")
		{
			ImGui::Image((ImTextureID)statsa, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "StatsItem")
		{
			ImGui::Image((ImTextureID)statsitema, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "Terrain")
		{
			ImGui::Image((ImTextureID)terraina, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "TimerService")
		{
			ImGui::Image((ImTextureID)timerdevicea, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}
		if (nodeClassName == "VideoCaptureService")
		{
			ImGui::Image((ImTextureID)videocapturea, ImVec2(12, 12));

			ImGui::SameLine();
			ImGui::SetCursorPosX(35.0f * indentLevel / 1.1);
		}

		bool isExpanded = ImGui::TreeNodeEx(uniqueId.c_str(),
			(hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet),
			"%s [%s]", nodeName.c_str(), nodeClassName.c_str());

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			selectedNodeName = nodeName;
			selectedNodeClassName = nodeClassName;
			selectedNodeHexAddress = uniqueId;
			selectedNodePath = fullPath;
			selected_instance = node;
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup(("NodeContextMenu##" + uniqueId).c_str());
		}

		if (ImGui::BeginPopup(("NodeContextMenu##" + uniqueId).c_str())) {
			if (ImGui::MenuItem("Copy Path")) {
				ImGui::SetClipboardText(fullPath.c_str());
			}
			if (ImGui::MenuItem("Copy Hex Address")) {
				ImGui::SetClipboardText(uniqueId.c_str());
			}
			if (selectedNodeClassName == "Part") {
				if (ImGui::MenuItem("Add To Esp")) {
					// esp logic
				}
			}
			ImGui::EndPopup();
		}

		if (isExpanded) {
			for (auto& child : children) {
				drawNode(child, indentLevel + 1, fullPath);
			}
			ImGui::TreePop();
		}
		};

	ImVec2 screenSize = ImGui::GetContentRegionAvail();
	float propertiesHeight = screenSize.y / 3.0f;

	ImGui::BeginChild("NodeTree", ImVec2(screenSize.x, screenSize.y - propertiesHeight), true);

	try {
		auto& datamodel = globals::game::data_model;
		if (datamodel.address != 0) {
			rbx::instance_t root_instance(datamodel.address);
			drawNode(root_instance, 0, "");
		}
	}
	catch (const std::exception& e) {
		ImGui::Text("Error: %s", e.what());
	}
	ImGui::EndChild();

	ImGui::BeginChild("NodeProperties", ImVec2(screenSize.x, propertiesHeight), true);

	if (selected_instance.has_value()) {
		rbx::instance_t& instance = selected_instance.value();

		ImGui::Text("Name: %s", selectedNodeName.c_str());
		ImGui::Text("Class: %s", selectedNodeClassName.c_str());
		ImGui::Text("Hex Address: %s", selectedNodeHexAddress.c_str());
		ImGui::Text("Path: %s", selectedNodePath.c_str());

		// Display children count if any
		auto children = instance.get_children();
		ImGui::Text("Children: %d", static_cast<int>(children.size()));

		if (selectedNodeClassName == "Part" || selectedNodeClassName == "MeshPart") {
			rbx::part_t* part = reinterpret_cast<rbx::part_t*>(&instance);
			if (part) {
				const math::vector3_t& position = part->get_position();
				const math::vector3_t& size = part->get_part_size();

				ImGui::Separator();
				ImGui::Text("Position:");
				ImGui::Text("  X: %.5f", position.x);
				ImGui::Text("  Y: %.5f", position.y);
				ImGui::Text("  Z: %.5f", position.z);

				ImGui::Text("Size:");
				ImGui::Text("  X: %.5f", size.x);
				ImGui::Text("  Y: %.5f", size.y);
				ImGui::Text("  Z: %.5f", size.z);

				ImGui::Text("Transparency: %.2f", part->get_transparency());
				ImGui::Text("Anchored: %s", part->get_anchored() ? "True" : "False");
			}
		}

	}


	if ((selectedNodeClassName == "Part" || selectedNodeClassName == "MeshPart") && selected_instance.has_value()) {
		rbx::instance_t& instance = selected_instance.value();
		rbx::part_t* part = reinterpret_cast<rbx::part_t*>(&instance);
		if (part) {
			const math::vector3_t& position = part->get_position();
			ImGui::Text("Position: X: %.5f, Y: %.5f, Z: %.5f", position.x, position.y, position.z);
		}


	}

	ImGui::EndChild();
}

std::string GetUserNameString() {
	wchar_t username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	if (GetUserNameW(username, &username_len)) {
		char username_c[UNLEN + 1];
		wcstombs(username_c, username, UNLEN + 1);
		return std::string(username_c);
	}
	else {
		return "User";
	}
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	render_t* instance = reinterpret_cast<render_t*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

	switch (msg) {
	case WM_SIZE:
		if (instance && instance->detail && instance->detail->swap_chain && wParam != SIZE_MINIMIZED) {
			if (instance->detail->render_target_view)
				instance->detail->render_target_view->Release();

			instance->detail->swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);

			ID3D11Texture2D* back_buffer = nullptr;
			instance->detail->swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

			if (back_buffer) {
				instance->detail->device->CreateRenderTargetView(back_buffer, NULL, &instance->detail->render_target_view);
				back_buffer->Release();
			}
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		return 0;
	}
	return DefWindowProcA(hwnd, msg, wParam, lParam);
}

render_t::render_t() {
	this->detail = std::make_unique<detail_t>();
}

render_t::~render_t() {
	destroy_imgui();
	destroy_window();
	destroy_device();
}








bool render_t::create_window() {
	WNDCLASSEXA wc{};
	wc.cbSize = sizeof(wc);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = wnd_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandleA(nullptr);
	wc.hIcon = nullptr;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = "Classname";

	if (!RegisterClassExA(&wc))
		return false;

	this->detail->window = CreateWindowExA(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		wc.lpszClassName,
		"main",
		WS_POPUP,
		0, 0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	if (!this->detail->window) {
		UnregisterClassA(wc.lpszClassName, wc.hInstance);
		return false;
	}

	SetWindowLongPtrA(this->detail->window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	if (!SetLayeredWindowAttributes(this->detail->window, RGB(0, 0, 0), 255, LWA_ALPHA))
		return false;

	MARGINS margins = { -1 };
	if (DwmExtendFrameIntoClientArea(this->detail->window, &margins) != S_OK)
		return false;

	ShowWindow(this->detail->window, SW_SHOW);
	UpdateWindow(this->detail->window);

	return true;
}

bool render_t::create_device() {
	if (!this->detail->window)
		return false;

	DXGI_SWAP_CHAIN_DESC swap_desc{};
	swap_desc.BufferCount = 1;
	swap_desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_desc.OutputWindow = this->detail->window;
	swap_desc.SampleDesc.Count = 1;
	swap_desc.SampleDesc.Quality = 0;
	swap_desc.Windowed = TRUE;
	swap_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
	D3D_FEATURE_LEVEL feature_level;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		levels,
		2,
		D3D11_SDK_VERSION,
		&swap_desc,
		&this->detail->swap_chain,
		&this->detail->device,
		&feature_level,
		&this->detail->device_context
	);

	if (FAILED(hr)) {
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_WARP,
			nullptr,
			0,
			levels,
			2,
			D3D11_SDK_VERSION,
			&swap_desc,
			&this->detail->swap_chain,
			&this->detail->device,
			&feature_level,
			&this->detail->device_context
		);
	}

	if (FAILED(hr) || !this->detail->device || !this->detail->device_context)
		return false;

	ID3D11Texture2D* back_buffer = nullptr;
	hr = this->detail->swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

	if (FAILED(hr) || !back_buffer)
		return false;

	hr = this->detail->device->CreateRenderTargetView(back_buffer, nullptr, &this->detail->render_target_view);
	back_buffer->Release();

	return !FAILED(hr);
}

bool render_t::create_imgui() {
	using namespace ImGui;
	CreateContext();
	StyleColorsDark();

	if (!ImGui_ImplWin32_Init(this->detail->window))
		return false;

	if (!this->detail->device || !this->detail->device_context)
		return false;

	if (!ImGui_ImplDX11_Init(this->detail->device, this->detail->device_context))
		return false;

	return true;
}

void render_t::destroy_device() {
	if (this->detail->render_target_view) this->detail->render_target_view->Release();
	if (this->detail->swap_chain) this->detail->swap_chain->Release();
	if (this->detail->device_context) this->detail->device_context->Release();
	if (this->detail->device) this->detail->device->Release();
}

void render_t::destroy_window() {
	if (this->detail->window)
		DestroyWindow(this->detail->window);
}

void render_t::destroy_imgui() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void render_t::start_render() {
	MSG msg{};
	static bool last_running_state = false;

	const std::chrono::milliseconds frame_time(1000 / 60);
	auto last_frame_time = std::chrono::high_resolution_clock::now();
	// <<<<<<<<<<< ///////////
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImFontConfig config;
	config.SizePixels = 12;
	ImFontConfig fa_config; fa_config.MergeMode = true; fa_config.PixelSnapH = true;

	ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->AddFontDefault();


	if (rust_model == nullptr)
		CreateTextureFromMemory(this->detail->device, raw_esp, sizeof(raw_esp), &rust_model);

	HRESULT basicbacona = CreateTextureFromMemory(this->detail->device, typeshit, sizeof(typeshit), &basicbacon); HRESULT ha = CreateTextureFromMemory(this->detail->device, folder, sizeof(folder), &foldera); HRESULT hb = CreateTextureFromMemory(this->detail->device, camera, sizeof(camera), &cameraa); HRESULT hc = CreateTextureFromMemory(this->detail->device, lightning, sizeof(lightning), &lightninga); HRESULT hd = CreateTextureFromMemory(this->detail->device, humanoid, sizeof(humanoid), &humanoida); HRESULT he = CreateTextureFromMemory(this->detail->device, part, sizeof(part), &partaa); HRESULT hf = CreateTextureFromMemory(this->detail->device, players, sizeof(players), &playersa); HRESULT hg = CreateTextureFromMemory(this->detail->device, meshpart, sizeof(meshpart), &meshparta);  HRESULT hag = CreateTextureFromMemory(this->detail->device, model, sizeof(model), &modela); HRESULT hga = CreateTextureFromMemory(this->detail->device, player, sizeof(player), &playera); HRESULT hgaa = CreateTextureFromMemory(this->detail->device, terrain, sizeof(terrain), &terraina); HRESULT hgba = CreateTextureFromMemory(this->detail->device, localscript, sizeof(localscript), &localscripta); HRESULT hcga = CreateTextureFromMemory(this->detail->device, localscripts, sizeof(localscripts), &localscriptsa); HRESULT hdga = CreateTextureFromMemory(this->detail->device, playergui, sizeof(playergui), &playerguia); HRESULT hfga = CreateTextureFromMemory(this->detail->device, stats, sizeof(stats), &statsa); HRESULT hgea = CreateTextureFromMemory(this->detail->device, guiservice, sizeof(guiservice), &guiservicea); HRESULT hgga = CreateTextureFromMemory(this->detail->device, videocapture, sizeof(videocapture), &videocapturea); HRESULT hhga = CreateTextureFromMemory(this->detail->device, runservice, sizeof(runservice), &runservicea); HRESULT hjga = CreateTextureFromMemory(this->detail->device, frame, sizeof(frame), &framea); HRESULT hsga = CreateTextureFromMemory(this->detail->device, csd, sizeof(csd), &csda); HRESULT h2ga = CreateTextureFromMemory(this->detail->device, contentprovider, sizeof(contentprovider), &contentprovidera); HRESULT h3ga = CreateTextureFromMemory(this->detail->device, nonreplicated, sizeof(nonreplicated), &nonreplicateda); HRESULT h4ga = CreateTextureFromMemory(this->detail->device, startergear, sizeof(startergear), &startergeara); HRESULT hg5a = CreateTextureFromMemory(this->detail->device, timerdevice, sizeof(timerdevice), &timerdevicea); HRESULT hg6a = CreateTextureFromMemory(this->detail->device, backpack, sizeof(backpack), &backpacka); HRESULT hg7a = CreateTextureFromMemory(this->detail->device, marketplaceservice, sizeof(marketplaceservice), &marketplaceservicea); HRESULT h8ga = CreateTextureFromMemory(this->detail->device, soundservice, sizeof(soundservice), &soundservicea); HRESULT h9ga = CreateTextureFromMemory(this->detail->device, logservice, sizeof(logservice), &logservicea); HRESULT h11ga = CreateTextureFromMemory(this->detail->device, statsitem, sizeof(statsitem), &statsitema); HRESULT h111ga = CreateTextureFromMemory(this->detail->device, boolvalue, sizeof(boolvalue), &boolvaluea); HRESULT h1111ga = CreateTextureFromMemory(this->detail->device, intvalue, sizeof(intvalue), &intvaluea); HRESULT h12ga = CreateTextureFromMemory(this->detail->device, doubletype, sizeof(doubletype), &doubletypea);
	HRESULT hr = CreateTextureFromMemory(this->detail->device, workspace, sizeof(workspace), &Imagine);

	HRESULT typah = CreateTextureFromMemory(this->detail->device, ancientlogo, sizeof(ancientlogo), &anicentlogo);

	var->font.icons[0] = io.Fonts->AddFontFromMemoryTTF(section_icons_hex, sizeof section_icons_hex, 15.f, &config, io.Fonts->GetGlyphRangesCyrillic());
	var->font.icons[1] = io.Fonts->AddFontFromMemoryTTF(icons_hex, sizeof icons_hex, 5.f, &config, io.Fonts->GetGlyphRangesCyrillic());

	var->font.tahoma = io.Fonts->AddFontFromMemoryTTF(tahoma_hex, sizeof tahoma_hex, 13.f, &config, io.Fonts->GetGlyphRangesCyrillic());
	// << <<<<<<<<< ///////////
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		if (this->limit_fps) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}


		if (GetAsyncKeyState(var->gui.menu_key) & 1)
			this->running = !this->running;




		HWND target = FindWindowA("Chrome_WidgetWin_1", "Discord Overlay");
		if (!target || IsIconic(target)) {
			MoveWindow(this->detail->window, 0, 0, 0, 0, true);
		}
		else {
			RECT client_rect;
			if (GetClientRect(target, &client_rect)) {
				POINT client_to_screen_pos = { client_rect.left, client_rect.top };
				ClientToScreen(target, &client_to_screen_pos);
				MoveWindow(this->detail->window, client_to_screen_pos.x, client_to_screen_pos.y,
					client_rect.right - client_rect.left,
					client_rect.bottom - client_rect.top, true);
			}
		}



		if (last_running_state != this->running) {
			if (this->running) {
				SetWindowLong(this->detail->window, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
			}
			else {
				SetWindowLong(this->detail->window, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
			}
			last_running_state = this->running;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		// gui->device = this->detail->device;

			render_menu();
		
		visuals_t::renderr();
		end_render();
	}
}

void render_t::end_render() {
	using namespace ImGui;
	Render();
	const float clear_color[4] = { 0.f, 0.f, 0.f, 0.f };
	this->detail->device_context->OMSetRenderTargets(1, &this->detail->render_target_view, nullptr);
	this->detail->device_context->ClearRenderTargetView(this->detail->render_target_view, clear_color);
	ImGui_ImplDX11_RenderDrawData(GetDrawData());
	this->detail->swap_chain->Present(0, 0);
}

ID3D11Device* render_t::GetDevice()
{
	return this->detail->device;
}



void render_t::render_menu() {


	static auto last_time = std::chrono::high_resolution_clock::now();
	auto current_time = std::chrono::high_resolution_clock::now();
	float delta_time = std::chrono::duration<float>(current_time - last_time).count();
	last_time = current_time;

	float fade_speed = 8.0f * delta_time;
	var->gui.menu_alpha = ImClamp(
		var->gui.menu_alpha + (fade_speed * (this->running ? 1.f : -1.f)),
		0.f,
		1.f
	);

	if (var->gui.menu_alpha <= 0.01f)
		return;


	gui->set_next_window_pos(ImVec2(GetIO().DisplaySize.x / 2 - var->window.width / 2, 20));
	gui->set_next_window_size(ImVec2(var->window.width, elements->section.size.y + var->window.spacing.y * 2 - 1));
	gui->push_style_var(ImGuiStyleVar_Alpha, var->gui.menu_alpha);
	gui->begin("Atlanta", nullptr, var->window.main_flags);
	{
		const ImVec2 pos = GetWindowPos();
		const ImVec2 size = GetWindowSize();
		ImDrawList* draw_list = GetWindowDrawList();
		ImGuiStyle* style = &GetStyle();

		{
			style->WindowPadding = var->window.padding;
			style->PopupBorderSize = var->window.border_size;
			style->WindowBorderSize = var->window.border_size;
			style->ItemSpacing = var->window.spacing;
			style->WindowShadowSize = var->window.shadow_size;
			style->ScrollbarSize = var->window.scrollbar_size;
			style->Colors[ImGuiCol_WindowShadow] = { clr->accent.Value.x, clr->accent.Value.y, clr->accent.Value.z, var->window.shadow_alpha };
		}

		{
			draw->rect(GetBackgroundDrawList(), pos - ImVec2(1, 1), pos + size + ImVec2(1, 1), draw->get_clr({ 0, 0, 0, 0.5f }));
			draw->rect_filled(draw_list, pos, pos + size, draw->get_clr(clr->window.background_one));
			draw->line(draw_list, pos + ImVec2(1, 1), pos + ImVec2(size.x - 1, 1), draw->get_clr(clr->accent), 1);
			draw->line(draw_list, pos + ImVec2(1, 2), pos + ImVec2(size.x - 1, 2), draw->get_clr(clr->accent, 0.4f), 1);
			draw->rect(draw_list, pos, pos + size, draw->get_clr(clr->window.stroke));
		}

		{
			gui->set_cursor_pos(style->ItemSpacing);
			gui->begin_group();
			{
				for (int i = 0; i < IM_ARRAYSIZE(var->gui.current_section); i++)
					gui->section(var->gui.section_icons[i], &var->gui.current_section[i]);
			}
			gui->end_group();
		}

		/*

		for (int i = 0; i < ImGui::GetWindowSize().x; i += 16)
						{
							for (int j = 0; j < ImGui::GetWindowSize().x; j += 16)
							{
								auto jew = ImGui::GetWindowPos();
								ImGui::GetWindowDrawList()->AddCircleFilled({ jew.x + i, jew.y + j }, 1.f, IM_COL32_WHITE);
							}
						}*/



		{
			if (var->gui.current_section[0])
			{
				gui->set_next_window_size_constraints(ImVec2(554, 627), GetIO().DisplaySize);
				gui->begin("1()", nullptr, var->window.flags);
				{
					draw->window_decorations();



					{
						static int subtabs;
						gui->set_cursor_pos(elements->content.window_padding + ImVec2(0, var->window.titlebar));
						gui->begin_group();
						{
							gui->sub_section("Legit", 0, subtabs, 7);
							gui->sub_section("Rage", 1, subtabs, 7);
							gui->sub_section("Players", 2, subtabs, 7);
							gui->sub_section("Visuals", 3, subtabs, 7);
							gui->sub_section("Misc", 4, subtabs, 7);
							gui->sub_section("Settings", 5, subtabs, 7);
							gui->sub_section("Configs", 6, subtabs, 7);


						}
						gui->end_group();

						gui->set_cursor_pos(elements->content.window_padding + ImVec2(0, var->window.titlebar + elements->section.height - 1));
						if (subtabs == 0) {
							gui->begin_content();
							{
								gui->begin_group();
								{
									gui->begin_child("Aim Settings", 2, 2);
									{
										gui->push_font(var->font.tahoma);

										gui->checkbox("Enable Aimbot", &globals::aim::aimbot);
										gui->sameline();
										gui->label_keybind("##AimKey", &globals::aim::aimbot_bind);

										const char* mode_items[] = { "Mouse Control", "Camera Control" };
										int current_mode = globals::aim::use_camera_mode ? 1 : 0;
										if (gui->dropdown("Aim Mode", &current_mode, mode_items, IM_ARRAYSIZE(mode_items))) {
											globals::aim::use_camera_mode = (current_mode == 1);
										}

										if (globals::aim::use_camera_mode) {
											gui->slider_float("Camera Smoothness", &globals::aim::camera_smoothness, 0.0f, 100.0f);
										}
										else {
											gui->slider_float("Mouse Smoothness", &globals::aim::mouse_smoothness, 0.0f, 100.0f);
											gui->slider_float("Mouse Sensitivity", &globals::aim::mouse_sensitivity, 0.1f, 5.0f);
										}

										const char* hitpart_items[] = { "Head", "Torso", "Root", "Nearest", "Random" };
										gui->dropdown("Target Hitbox", &globals::aim::hitpart_mode, hitpart_items, IM_ARRAYSIZE(hitpart_items));

										gui->checkbox("Sticky Target", &globals::aim::aimbot_sticky);

										gui->slider_int("Lock Delay", &globals::aim::target_lock_delay, 0, 100);

										if (globals::aim::aimbot_sticky) {
											gui->slider_int("Switch Delay", &globals::aim::target_switch_delay, 0, 1000);
										}

										gui->pop_font();
									}
									gui->end_child();

									gui->begin_child("Humanization", 2, 2);
									{
										gui->push_font(var->font.tahoma);

										gui->checkbox("Enable Shake", &globals::aim::shake);

										gui->slider_float("Shake X", &globals::aim::shake_x, 0.0f, 5.0f);
										gui->slider_float("Shake Y", &globals::aim::shake_y, 0.0f, 5.0f);

										if (globals::aim::use_camera_mode) {
											gui->slider_float("Shake Z", &globals::aim::shake_z, 0.0f, 5.0f);
										}

										gui->slider_float("Offset X", &globals::aim::offset_x, -5.0f, 5.0f);
										gui->slider_float("Offset Y", &globals::aim::offset_y, -5.0f, 5.0f);
										gui->slider_float("Offset Z", &globals::aim::offset_z, -5.0f, 5.0f);

										gui->pop_font();
									}
									gui->end_child();
								}
								gui->end_group();

								gui->sameline();

								gui->begin_group();
								{
									gui->begin_child("Aim Essentials", 2, 1);
									{
										gui->push_font(var->font.tahoma);
										gui->checkbox("Enable specate target", &globals::aim::specate_target);
									


										gui->pop_font();
									}
									gui->end_child();
								}
								gui->end_group();
							}
							gui->end_content();
						}

						if (subtabs == 2) {
							ImVec2 checkbox_pos;
							gui->begin_content();
							{
								gui->begin_group();
								{
									gui->begin_child("Enemies", 2, 2);
									{
										gui->checkbox("Masterswitch", &globals::visuals::enabled);
										gui->checkbox("Box ESP", &globals::visuals::box);
										gui->sameline();
										gui->color_edit("Box Outline Color", globals::visuals::colors::box_outline_color);

										gui->checkbox("Chams", &globals::visuals::chams);
										gui->checkbox("Skeleton", &globals::visuals::skeleton);
										if (globals::visuals::skeleton) {
											gui->checkbox("Skeleton outline", &globals::visuals::skeleton_outline);

										}
										gui->checkbox("Head Dot", &globals::visuals::headdot);
										gui->checkbox("Tracers", &globals::visuals::tracers);
										gui->checkbox("HealthBar", &globals::visuals::healthbar);
										gui->checkbox("ArmorBar", &globals::visuals::armorbar);
									}
									gui->end_child();

									gui->begin_child("Other", 2, 2);
									{
										gui->checkbox("Name", &globals::visuals::name);
										gui->checkbox("Tool", &globals::visuals::tool);
										gui->checkbox("Distance", &globals::visuals::distance);
										gui->checkbox("RigType", &globals::visuals::rigtype);
										gui->checkbox("State", &globals::visuals::flags);
									}
									gui->end_child();
								}
								gui->end_group();

								gui->sameline();

								gui->begin_group();
								{
									gui->begin_child("Options", 2, 1);
									{
										static const char* tracer_positions[] = { "Top Left", "Top Center", "Bottom Center", "Center" };
										static const char* chams_types[] = { "Convex", "Union" };

										gui->checkbox("Box Fill", &globals::visuals::box_fill);
										gui->sameline();
										gui->color_edit("Gradient Top", globals::visuals::colors::box_gradient_top);
										gui->sameline();
										gui->color_edit("Gradient Bottom", globals::visuals::colors::box_gradient_bottom);

										gui->checkbox("Box Outline", &globals::visuals::box_outline);

										if (globals::visuals::box_fill) {
											gui->checkbox("Box Gradient", &globals::visuals::box_gradient);

											if (globals::visuals::box_gradient) {
												gui->checkbox("Gradient Auto-Rotate", &globals::visuals::box_gradient_autorotate);

												if (globals::visuals::box_gradient_autorotate) {
													gui->slider_float("Rotation Speed", &globals::visuals::box_gradient_rotation_speed, 0.1f, 5.0f);
												}
											}
											else {

											}
										}

										gui->checkbox("LocalPlayer Check", &globals::visuals::localplayercheck);
										gui->dropdown("Tracer Position", &globals::visuals::tracer_position, tracer_positions, IM_ARRAYSIZE(tracer_positions), false, 4);
										gui->slider_float("Tracer Thickness", &globals::visuals::tracer_thickness, 0.5f, 5.f);
										gui->slider_float("Head Dot Size", &globals::visuals::headdot_size, 1.f, 15.f);
										gui->dropdown("Chams Type", &globals::visuals::chams_type, chams_types, IM_ARRAYSIZE(chams_types), false, 2);
									}
									gui->end_child();
								}
								gui->end_group();
							}
							gui->end_content();
						}

						if (subtabs == 1) {
							gui->begin_content();
							{
								gui->begin_group();
								{
									gui->begin_child("Movement", 2, 2);
									{
										gui->push_font(var->font.tahoma);

										gui->checkbox("Fly", &globals::rage::fly);
										gui->sameline();
										gui->label_keybind("##fly_bind", &globals::rage::fly_bind);
										gui->slider_float("Fly Speed", &globals::rage::fly_speed, 1.0f, 100.0f);


										gui->checkbox("Walk Speed", &globals::rage::speed_enabled);
										gui->sameline();

										gui->label_keybind("##walkspeed_bind", &globals::rage::walkspeed_bind);
										gui->slider_float("Speed Amount", &globals::rage::walkspeed_amount, 16, 200);


										gui->checkbox("Jump Power", &globals::rage::jumppower_bind.enabled);
										gui->sameline();

										gui->label_keybind("##jumppower_bind", &globals::rage::jumppower_bind);
										gui->slider_float("Power", &globals::rage::jumppowevbalue, 50.0f, 500.0f);


										gui->checkbox("Infinite Jump", &globals::rage::infinite_jump);
										gui->sameline();

										gui->label_keybind("##infinite_bind", &globals::rage::infinite_bind);
										gui->slider_float("Jump Power2", &globals::rage::infinite_power, 50.0f, 200.0f);


										gui->checkbox("Hip Height", &globals::rage::hipheight_enabled);
										gui->sameline();

										gui->label_keybind("##hipheight_bind", &globals::rage::hipheight_bind);
										gui->slider_float("Height", &globals::rage::hipheighrslider, -10.0f, 10.0f);


										gui->pop_font();
									}
									gui->end_child();

									gui->begin_child("Angles", 2, 2);
									{
										gui->push_font(var->font.tahoma);

										gui->checkbox("Spin Bot", &globals::rage::spinbot);

										gui->checkbox("third person", &globals::rage::third_person);
										gui->sameline();
										gui->label_keybind("##third", &globals::rage::third_personbind);

										gui->checkbox("Orbit", &globals::rage::orbit);
										gui->sameline();

										gui->label_keybind("##orbit_bind", &globals::rage::orbitkeybind);
										gui->slider_float("Orbit Speed", &globals::rage::orbitspeed, 0.1f, 10.0f);
										gui->slider_float("Orbit X", &globals::rage::orbitx, 1.0f, 20.0f);
										gui->slider_float("Orbit Y", &globals::rage::orbity, -10.0f, 10.0f);


										gui->pop_font();
									}
									gui->end_child();
								}
								gui->end_group();

								gui->sameline();

								gui->begin_group();
								{
									gui->begin_child("Manipulation", 2, 1);
									{
										gui->push_font(var->font.tahoma);

										gui->checkbox("Anti Stomp", &globals::rage::antistomp);
										gui->checkbox("Anti AFK", &globals::game::anti_afk);



										gui->pop_font();
									}
									gui->end_child();
								}
								gui->end_group();
							}
							gui->end_content();
						}


						if (subtabs == 6) {
							gui->begin_content();
							{
								gui->begin_group();
								{
									gui->begin_child("Menu", 2, 2);
									{
										//gui->KeybindTest("Menu Hotkey", &var->gui.menu_key, 0);

									}
									gui->end_child();

									gui->begin_child("Theme", 2, 2);
									{

										static float menu_accent[4] = { clr->accent.Value.x, clr->accent.Value.y, clr->accent.Value.z, 1.f };
										static float contrast_one[4] = { clr->window.background_one.Value.x, clr->window.background_one.Value.y, clr->window.background_one.Value.z, 1.f };
										static float contrast_two[4] = { clr->window.background_two.Value.x, clr->window.background_two.Value.y, clr->window.background_two.Value.z, 1.f };
										static float inline_c[4] = { clr->window.stroke.Value.x, clr->window.stroke.Value.y, clr->window.stroke.Value.z, 1.f };
										static float outline_c[4] = { clr->widgets.stroke_two.Value.x, clr->widgets.stroke_two.Value.y, clr->widgets.stroke_two.Value.z, 1.f };
										static float text_active[4] = { clr->widgets.text.Value.x, clr->widgets.text.Value.y, clr->widgets.text.Value.z, 1.f };
										static float text_inactive[4] = { clr->widgets.text_inactive.Value.x, clr->widgets.text_inactive.Value.y, clr->widgets.text_inactive.Value.z, 1.f };
										static int theme_index = 0;
										static int previous_theme = -1;
										const char* themes[] = {
									"Midnight Blue", "Forest Green", "Purple", "Hot Pink", "Titanium White", "Mint", "Hazard",
									"Ocean Cyan", "Canada", "Steel", "Skyline Blue", "Cherry Blossom", "Atlanta"
										};



										gui->dropdown("Theme", &theme_index, themes, IM_ARRAYSIZE(themes));

										if (theme_index != previous_theme)
										{
											if (theme_index == 0)  // Midnight Blue
											{
												clr->accent = ImColor(0.0348633f, 0.313692f, 0.865772f);
												clr->window.background_one = ImColor(0.0563038f, 0.0563038f, 0.0671141f);
											}
											if (theme_index == 1)  // Forest Green
											{
												clr->accent = ImColor(0.165398f, 0.456376f, 0.274759f);
												clr->window.background_one = ImColor(0.0563038f, 0.0563038f, 0.0671141f);
												clr->window.background_two = ImColor(0.0794559f, 0.0794559f, 0.0939597f);
												clr->window.stroke = ImColor(0.158101f, 0.158101f, 0.181208f);
												clr->window.outline = ImColor(0.141176f, 0.141176f, 0.188235f);
												clr->widgets.text = ImColor(0.784314f, 0.784314f, 0.784314f);
												clr->widgets.text_inactive = ImColor(0.533333f, 0.533333f, 0.533333f);
											}
											if (theme_index == 2)  // Purple
											{
												clr->accent = ImColor(0.443671f, 0.129994f, 0.744966f);
												clr->window.background_one = ImColor(0.0563038f, 0.0563038f, 0.0671141f);
											}
											if (theme_index == 3)  // Hot Pink
											{
												clr->accent = ImColor(0.744966f, 0.129994f, 0.501454f);
												clr->window.background_one = ImColor(0.0563038f, 0.0563038f, 0.0671141f);
											}
											if (theme_index == 4)  // Titanium White
											{
												clr->accent = ImColor(1.0f, 1.0f, 1.0f);
												clr->window.background_one = ImColor(0.0563038f, 0.0563038f, 0.0671141f);
											}
											if (theme_index == 5)  // Mint
											{
												clr->accent = ImColor(0.630872f, 1.0f, 0.814198f);
												clr->window.background_one = ImColor(0.0563038f, 0.0563038f, 0.0671141f);
											}
											if (theme_index == 6)  // Hazard
											{
												clr->accent = ImColor(0.556454f, 0.718121f, 0.226521f);
												clr->window.background_one = ImColor(0.0563038f, 0.0563038f, 0.0671141f);
											}
											if (theme_index == 7)  // Ocean Cyan
											{
												clr->accent = ImColor(0.137f, 0.745f, 0.854f);
												clr->window.background_one = ImColor(0.045f, 0.055f, 0.065f);
												clr->window.background_two = ImColor(0.065f, 0.075f, 0.085f);
												clr->window.stroke = ImColor(0.110f, 0.130f, 0.150f);
												clr->window.outline = ImColor(0.150f, 0.170f, 0.190f);
												clr->widgets.text = ImColor(0.780f, 0.930f, 1.000f);
												clr->widgets.text_inactive = ImColor(0.530f, 0.630f, 0.730f);
											}
											if (theme_index == 8)  // Canadian Pride
											{
												clr->accent = ImColor(0.850f, 0.050f, 0.050f);
												clr->window.background_one = ImColor(0.060f, 0.060f, 0.060f);
												clr->window.background_two = ImColor(0.090f, 0.090f, 0.090f);
												clr->window.stroke = ImColor(0.160f, 0.160f, 0.160f);
												clr->window.outline = ImColor(0.200f, 0.200f, 0.200f);
												clr->widgets.text = ImColor(1.000f, 1.000f, 1.000f);
												clr->widgets.text_inactive = ImColor(0.700f, 0.700f, 0.700f);
											}

											if (theme_index == 9) // Steel 
											{
												clr->accent = ImColor(0.290f, 0.540f, 0.690f);
												clr->window.background_one = ImColor(0.030f, 0.040f, 0.050f);
												clr->window.background_two = ImColor(0.060f, 0.080f, 0.100f);
												clr->window.stroke = ImColor(0.110f, 0.130f, 0.150f);
												clr->window.outline = ImColor(0.150f, 0.170f, 0.190f);
												clr->widgets.text = ImColor(0.920f, 0.950f, 1.000f);
												clr->widgets.text_inactive = ImColor(0.600f, 0.700f, 0.800f);
											}

											if (theme_index == 10) // Skyline Blue
											{
												clr->accent = ImColor(0.200f, 0.550f, 0.950f);
												clr->window.background_one = ImColor(0.030f, 0.040f, 0.060f);
												clr->window.background_two = ImColor(0.060f, 0.070f, 0.090f);
												clr->window.stroke = ImColor(0.100f, 0.120f, 0.150f);
												clr->window.outline = ImColor(0.140f, 0.160f, 0.190f);
												clr->widgets.text = ImColor(0.850f, 0.900f, 1.000f);
												clr->widgets.text_inactive = ImColor(0.550f, 0.600f, 0.700f);
											}

											if (theme_index == 11) // Cherry Blossom
											{
												clr->accent = ImColor(1.000f, 0.670f, 0.830f);
												clr->window.background_one = ImColor(0.060f, 0.050f, 0.060f);
												clr->window.background_two = ImColor(0.090f, 0.070f, 0.090f);
												clr->window.stroke = ImColor(0.130f, 0.110f, 0.130f);
												clr->window.outline = ImColor(0.170f, 0.150f, 0.170f);
												clr->widgets.text = ImColor(1.000f, 0.800f, 0.900f);
												clr->widgets.text_inactive = ImColor(0.700f, 0.500f, 0.600f);
											}

											if (theme_index == 12) // Atlanta
											{
												clr->accent = ImColor(154, 127, 172);
												clr->window.background_one = ImColor(36, 36, 47);
												clr->window.background_two = ImColor(42, 42, 56);
												clr->window.stroke = ImColor(53, 53, 67);
												clr->window.outline = ImColor(36, 36, 48);
												clr->widgets.text = ImColor(200, 200, 200);
												clr->widgets.text_inactive = ImColor(136, 136, 136);
											}

											menu_accent[0] = clr->accent.Value.x;
											menu_accent[1] = clr->accent.Value.y;
											menu_accent[2] = clr->accent.Value.z;

											contrast_one[0] = clr->window.background_one.Value.x;
											contrast_one[1] = clr->window.background_one.Value.y;
											contrast_one[2] = clr->window.background_one.Value.z;

											contrast_two[0] = clr->window.background_two.Value.x;
											contrast_two[1] = clr->window.background_two.Value.y;
											contrast_two[2] = clr->window.background_two.Value.z;

											inline_c[0] = clr->window.stroke.Value.x;
											inline_c[1] = clr->window.stroke.Value.y;
											inline_c[2] = clr->window.stroke.Value.z;

											outline_c[0] = clr->widgets.stroke_two.Value.x;
											outline_c[1] = clr->widgets.stroke_two.Value.y;
											outline_c[2] = clr->widgets.stroke_two.Value.z;

											text_active[0] = clr->widgets.text.Value.x;
											text_active[1] = clr->widgets.text.Value.y;
											text_active[2] = clr->widgets.text.Value.z;

											text_inactive[0] = clr->widgets.text_inactive.Value.x;
											text_inactive[1] = clr->widgets.text_inactive.Value.y;
											text_inactive[2] = clr->widgets.text_inactive.Value.z;

										}






										static bool window_glow = false;
										gui->checkbox("Window Glow", &window_glow);
										if (window_glow)
										{
											gui->slider_float("Glow Thickness", &var->window.shadow_size, 1, 100);
											gui->slider_float("Glow Alpha", &var->window.shadow_alpha, 0, 1);
										}
										else
											var->window.shadow_size = 0;



									}
									gui->end_child();
								}
								gui->end_group();

								gui->sameline();

								gui->begin_group();
								{
									gui->begin_child("Configs", 2, 2);
									{

									}
									gui->end_child();

									gui->begin_child("Settings", 2, 2);
									{
										gui->checkbox("Team Check", &globals::gamesupport::team_check);
									}
									gui->end_child();
								}
								gui->end_group();
							}
							gui->end_content();
						}
					}

				}
				gui->end();
			}

			if (var->gui.current_section[1])
			{
				gui->set_next_window_size_constraints(ImVec2(377, 415), GetIO().DisplaySize);
				gui->begin("preview", nullptr, var->window.flags);
				{
					draw->window_decorations();

				}
				gui->end();
			}
			if (var->gui.current_section[1])
			{
				static int selected_row = -1;
				static std::optional<rbx::player_t> selected_player;

				gui->set_next_window_size_constraints(ImVec2(509, 620), GetIO().DisplaySize);
				gui->begin("Players", nullptr, var->window.flags);
				{
					draw->window_decorations();
					gui->set_cursor_pos(elements->content.window_padding + ImVec2(0, var->window.titlebar + 1));
					gui->push_style_var(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
					gui->push_style_var(ImGuiStyleVar_ItemSpacing, elements->content.spacing);
					gui->begin_def_child("table test", ImVec2(GetWindowWidth() - elements->content.window_padding.x * 2, GetContentRegionAvail().y - elements->content.window_padding.y * 6), 0, ImGuiWindowFlags_NoMove);
					{
						gui->push_font(var->font.tahoma);
						gui->push_style_color(ImGuiCol_TableBorderLight, draw->get_clr(clr->window.stroke));
						gui->push_style_color(ImGuiCol_TableBorderStrong, draw->get_clr(clr->window.stroke));
						gui->push_style_color(ImGuiCol_TableRowBg, draw->get_clr(clr->window.background_one));
						gui->push_style_color(ImGuiCol_TableRowBgAlt, draw->get_clr(clr->window.background_one));
						gui->push_style_color(ImGuiCol_Text, draw->get_clr(clr->widgets.text_inactive));

						if (gui->begin_table("Table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg, ImVec2(GetContentRegionAvail().x - 1, 0)))
						{
							int row = 0;
							for (rbx::player_t& player : globals::game::player_cache)
							{
								gui->table_next_row();
								gui->table_set_column_index(0);
								{
									if (selected_row == row)
										gui->push_style_color(ImGuiCol_Text, draw->get_clr(clr->accent));
									draw->text_outline(player.Name.c_str());
									if (selected_row == row)
										gui->pop_style_color();
									if (IsItemClicked()) {
										selected_row = row;
										selected_player = player;
									}
								}
								gui->table_set_column_index(1);
								{
									std::string team_name = player.team.get_name();
									if (team_name.empty()) {
										draw->text_outline("None");
									}
									else {
										draw->text_outline(team_name.c_str());
									}
								}
								gui->table_set_column_index(2);
								{
									std::string local_player_name = globals::game::local_player.Name;
									if (player.Name == local_player_name) {
										draw->text_outline("LP");
									}
									else {
										draw->text_outline("Player");
									}
								}
								row++;
							}
							gui->end_table();

							Text((std::stringstream{} << "selected row - " << std::to_string(selected_row)).str().c_str());
							ImGui::Separator();

							if (selected_player) {
								auto draw_list = ImGui::GetWindowDrawList();

								std::string user_id = std::to_string(selected_player->userid);

								RenderUserImage(this->detail->device, user_id);

								ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
								ImVec2 image_size(100.0f, 100.0f);

								if (image_cache.find(user_id) != image_cache.end()) {
									ImTextureID texture_id = (ImTextureID)image_cache[user_id].texture;
									if (texture_id) {
										ImGui::Image(texture_id, image_size);
										draw_list->AddRect(cursor_pos,
											ImVec2(cursor_pos.x + image_size.x, cursor_pos.y + image_size.y),
											IM_COL32(62, 52, 62, 200),
											0.0f,
											ImDrawFlags_None,
											1.0f);
									}
								}
								else {
									draw_list->AddRectFilled(cursor_pos,
										ImVec2(cursor_pos.x + image_size.x, cursor_pos.y + image_size.y),
										IM_COL32(80, 80, 80, 255));
									draw_list->AddRect(cursor_pos,
										ImVec2(cursor_pos.x + image_size.x, cursor_pos.y + image_size.y),
										IM_COL32(62, 52, 62, 200),
										0.0f,
										ImDrawFlags_None,
										1.0f);
									ImGui::Dummy(image_size);
								}

								ImGui::SameLine(0.0f, 10.0f);

								ImGui::BeginGroup();
								std::string text1 = "Name: " + selected_player->Name;
								gui->text(draw_list, text1.c_str());
								std::string text2 = "User ID: " + std::to_string(selected_player->userid);
								gui->text(draw_list, text2.c_str());
								std::string text3 = "Team: " + selected_player->team.get_name();
								gui->text(draw_list, text3.c_str());
								ImGui::EndGroup();
							}
							else {
								auto draw_list = ImGui::GetWindowDrawList();
								gui->text(draw_list, "No player selected");
							}

							gui->pop_font();
						}
						gui->pop_style_color(5);
					}
					gui->end_def_child();
					gui->pop_style_var(2);
				}
				gui->end();
			}




			//if (var->gui.current_section[3])
			//{
			//	gui->set_next_window_size_constraints(ImVec2(400, 400), GetIO().DisplaySize);
			//	gui->begin("menu", nullptr, var->window.flags);
			//	{
			//		draw->window_decorations();

			//	}
			//	gui->end();
			//}

			//if (var->gui.current_section[4])
			//{
			//	gui->set_next_window_size_constraints(ImVec2(400, 400), GetIO().DisplaySize);
			//	gui->begin("code", nullptr, var->window.flags);
			//	{
			//		draw->window_decorations();

			//	}
			//	gui->end();
			//}

			if (var->gui.current_section[2])
			{

			}

			//if (var->gui.current_section[6])
			//{
			//	gui->set_next_window_size_constraints(ImVec2(400, 400), GetIO().DisplaySize);
			//	gui->begin("cloud", nullptr, var->window.flags);
			//	{
			//		draw->window_decorations();

			//	}
			//	gui->end();
			//}
		}

		var->window.width = GetCurrentWindow()->ContentSize.x + style->ItemSpacing.x;

		if (IsMouseHoveringRect(pos, pos + size))
			SetWindowFocus();
	}
	gui->end();
	gui->pop_style_var();


	gui_t::render_watermark();
}



void render_t::render_visuals() {


}

