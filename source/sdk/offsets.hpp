#pragma once
// yn-ext dynamic offsets — powered by https://offsets.imtheo.lol
// No static offsets. Everything loads at runtime from offsets.json.
//
// Startup flow (call Offsets::Init() first in main, before initialise::setup):
//   1. GET /roblox/version -> live version
//   2. compare to offsets.version on disk
//   3. if differ (or no cache), GET /offsets.json -> save offsets.json + offsets.version
//   4. parse JSON, fill all Offsets::Class::Field vars below
//
// JSON shape: { "Roblox Version": "...", "Offsets": { "Humanoid": { "Health": 404, ... }, ... } }
// Values are decimal. Missing keys stay 0 (logged, not fatal).
// Offsets::IsReady() tells you if load succeeded. Fallback: uses disk cache if network fails.

#include <cstdint>
#include <string>
#include <fstream>
#include <filesystem>
#include <cstdio>

#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

#include "../utils/json/json.hpp"
#include "../utils/output/output.hpp"

namespace Offsets {
namespace fs = std::filesystem;
using json = nlohmann::json;

inline std::string ClientVersion = {};
inline bool Loaded = false;

// ---- offset storage (was constexpr, now runtime-loaded) ----
namespace BasePart {
    inline uintptr_t AssemblyAngularVelocity = 0;
    inline uintptr_t AssemblyLinearVelocity = 0;
    inline uintptr_t Color3 = 0;
    inline uintptr_t Position = 0;
    inline uintptr_t Primitive = 0;
    inline uintptr_t Rotation = 0;
    inline uintptr_t Size = 0;
    inline uintptr_t Transparency = 0;
}
namespace Camera {
    inline uintptr_t CameraSubject = 0;
    inline uintptr_t Position = 0;
    inline uintptr_t Rotation = 0;
}
namespace DataModel {
    inline uintptr_t CreatorId = 0;
    inline uintptr_t GameId = 0;
    inline uintptr_t PlaceId = 0;
    inline uintptr_t ServerIP = 0;
}
namespace FakeDataModel {
    inline uintptr_t Pointer = 0;
    inline uintptr_t RealDataModel = 0;
}
namespace GuiObject {
    inline uintptr_t Text = 0;
}
namespace Humanoid {
    inline uintptr_t Health = 0;
    inline uintptr_t HipHeight = 0;
    inline uintptr_t HumanoidState = 0;
    inline uintptr_t HumanoidStateID = 0;
    inline uintptr_t JumpPower = 0;
    inline uintptr_t MaxHealth = 0;
    inline uintptr_t RigType = 0;
    inline uintptr_t Walkspeed = 0;
    inline uintptr_t WalkspeedCheck = 0;
}
namespace Instance {
    inline uintptr_t ChildrenEnd = 0;
    inline uintptr_t ChildrenStart = 0;
    inline uintptr_t ClassDescriptor = 0;
    inline uintptr_t ClassName = 0;
    inline uintptr_t Name = 0;
    inline uintptr_t Parent = 0;
}
namespace Lighting {
    inline uintptr_t Ambient = 0;
    inline uintptr_t Brightness = 0;
    inline uintptr_t ClockTime = 0;
    inline uintptr_t ColorShift_Bottom = 0;
    inline uintptr_t ColorShift_Top = 0;
    inline uintptr_t ExposureCompensation = 0;
    inline uintptr_t FogColor = 0;
    inline uintptr_t FogEnd = 0;
    inline uintptr_t FogStart = 0;
    inline uintptr_t GeographicLatitude = 0;
    inline uintptr_t OutdoorAmbient = 0;
}
namespace Misc {
    inline uintptr_t Adornee = 0;
    inline uintptr_t Value = 0;
}
namespace MouseService {
    inline uintptr_t InputObject = 0;
}
namespace Player {
    inline uintptr_t LocalPlayer = 0;
    inline uintptr_t ModelInstance = 0;
    inline uintptr_t Team = 0;
    inline uintptr_t UserId = 0;
}
namespace PrimitiveFlags {
    inline uintptr_t Anchored = 0;
}
namespace TaskScheduler {
    inline uintptr_t JobEnd = 0;
    inline uintptr_t JobName = 0;
    inline uintptr_t JobStart = 0;
    inline uintptr_t MaxFPS = 0;
    inline uintptr_t Pointer = 0;
}
namespace VisualEngine {
    inline uintptr_t Dimensions = 0;
    inline uintptr_t Pointer = 0;
    inline uintptr_t ViewMatrix = 0;
}
namespace Workspace {
    inline uintptr_t ForceNewAFKDuration = 0;
    inline uintptr_t ReadOnlyGravity = 0;
}
namespace silent {
    inline uintptr_t FramePositionX = 0;
    inline uintptr_t FramePositionY = 0;
}

// ---- internals ----
inline std::string HttpGet(const wchar_t* host, const wchar_t* path) {
    std::string out;
    HINTERNET hSession = WinHttpOpen(L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) yn-ext/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) throw std::runtime_error("WinHttpOpen failed");
    HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); throw std::runtime_error("WinHttpConnect failed"); }
    HINTERNET hReq = WinHttpOpenRequest(hConnect, L"GET", path, NULL,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hReq) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); throw std::runtime_error("WinHttpOpenRequest failed"); }
    if (!WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        throw std::runtime_error("WinHttpSendRequest failed");
    }
    if (!WinHttpReceiveResponse(hReq, NULL)) {
        WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        throw std::runtime_error("WinHttpReceiveResponse failed");
    }
    DWORD status = 0, statusLen = sizeof(status);
    WinHttpQueryHeaders(hReq, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusLen, WINHTTP_NO_HEADER_INDEX);
    if (status != 200) {
        WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        throw std::runtime_error("HTTP status " + std::to_string(status));
    }
    DWORD dwSize = 0;
    do {
        DWORD dwDownloaded = 0;
        if (!WinHttpQueryDataAvailable(hReq, &dwSize)) break;
        if (dwSize == 0) break;
        std::string buf(dwSize, '\0');
        if (!WinHttpReadData(hReq, buf.data(), dwSize, &dwDownloaded)) break;
        out.append(buf.data(), dwDownloaded);
    } while (dwSize > 0);
    WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
    return out;
}

inline std::string TrimStr(std::string v) {
    while (!v.empty() && (v.back() == '\n' || v.back() == '\r' || v.back() == ' ' || v.back() == '\t' || v.back() == '"')) v.pop_back();
    while (!v.empty() && (v.front() == ' ' || v.front() == '\t' || v.front() == '\n' || v.front() == '\r' || v.front() == '"')) v.erase(v.begin());
    return v;
}

inline uintptr_t Lookup(const json& off, const char* cls, const char* field) {
    auto it = off.find(cls);
    if (it == off.end() || !it->is_object()) return 0;
    auto jt = it->find(field);
    if (jt == it->end()) return 0;
    try {
        if (jt->is_string()) return static_cast<uintptr_t>(std::stoull(jt->get<std::string>(), nullptr, 0));
        return jt->get<uintptr_t>();
    } catch (...) { return 0; }
}

inline void FillFromJson(const json& data) {
    if (data.contains("Roblox Version") && data["Roblox Version"].is_string())
        ClientVersion = data["Roblox Version"].get<std::string>();
    else if (data.contains("Roblox Version"))
        ClientVersion = TrimStr(data["Roblox Version"].dump());

    if (!data.contains("Offsets") || !data["Offsets"].is_object())
        throw std::runtime_error("offsets.json missing 'Offsets' object");
    const auto& off = data["Offsets"];
    int missing = 0;
#define SET(CLS, FIELD) do { \
    uintptr_t v = Lookup(off, #CLS, #FIELD); \
    CLS::FIELD = v; \
    if (v == 0) { missing++; logger::print<logger::level::warn>("offset missing: %s::%s", #CLS, #FIELD); } \
} while (0)
#define SET2(CLS, FIELD, ALT_CLS, ALT_FIELD) do { \
    uintptr_t v = Lookup(off, #CLS, #FIELD); \
    if (v == 0) v = Lookup(off, #ALT_CLS, #ALT_FIELD); \
    CLS::FIELD = v; \
    if (v == 0) { missing++; logger::print<logger::level::warn>("offset missing: %s::%s (alt %s::%s)", #CLS, #FIELD, #ALT_CLS, #ALT_FIELD); } \
} while (0)

    // imtheo moved part motion fields under Primitive — try BasePart first, fall back to Primitive.
    SET2(BasePart, AssemblyAngularVelocity, Primitive, AssemblyAngularVelocity);
    SET2(BasePart, AssemblyLinearVelocity, Primitive, AssemblyLinearVelocity);
    SET(BasePart, Color3);
    SET2(BasePart, Position, Primitive, Position);
    SET(BasePart, Primitive);
    SET2(BasePart, Rotation, Primitive, Rotation);
    SET2(BasePart, Size, Primitive, Size);
    SET(BasePart, Transparency);
    SET(Camera, CameraSubject);
    SET(Camera, Position);
    SET(Camera, Rotation);
    SET(DataModel, CreatorId);
    SET(DataModel, GameId);
    SET(DataModel, PlaceId);
    SET(DataModel, ServerIP);
    SET(FakeDataModel, Pointer);
    SET(FakeDataModel, RealDataModel);
    SET(GuiObject, Text);
    SET(Humanoid, Health);
    SET(Humanoid, HipHeight);
    SET(Humanoid, HumanoidState);
    SET(Humanoid, HumanoidStateID);
    SET(Humanoid, JumpPower);
    SET(Humanoid, MaxHealth);
    SET(Humanoid, RigType);
    SET(Humanoid, Walkspeed);
    SET(Humanoid, WalkspeedCheck);
    SET(Instance, ChildrenEnd);
    SET(Instance, ChildrenStart);
    SET(Instance, ClassDescriptor);
    SET(Instance, ClassName);
    SET(Instance, Name);
    SET(Instance, Parent);
    SET(Lighting, Ambient);
    SET(Lighting, Brightness);
    SET(Lighting, ClockTime);
    SET(Lighting, ColorShift_Bottom);
    SET(Lighting, ColorShift_Top);
    SET(Lighting, ExposureCompensation);
    SET(Lighting, FogColor);
    SET(Lighting, FogEnd);
    SET(Lighting, FogStart);
    SET(Lighting, GeographicLatitude);
    SET(Lighting, OutdoorAmbient);
    SET(Misc, Adornee);
    SET(Misc, Value);
    SET(MouseService, InputObject);
    SET(Player, LocalPlayer);
    SET(Player, ModelInstance);
    SET(Player, Team);
    SET(Player, UserId);
    SET(PrimitiveFlags, Anchored);
    SET(TaskScheduler, JobEnd);
    SET(TaskScheduler, JobName);
    SET(TaskScheduler, JobStart);
    // imtheo 2.2.4 dropped TaskScheduler::MaxFPS — keep last known RVA so FPS unlock keeps working.
    TaskScheduler::MaxFPS = Lookup(off, "TaskScheduler", "MaxFPS");
    if (TaskScheduler::MaxFPS == 0) TaskScheduler::MaxFPS = 0xB0;
    SET(TaskScheduler, Pointer);
    SET(VisualEngine, Dimensions);
    SET(VisualEngine, Pointer);
    SET(VisualEngine, ViewMatrix);
    // imtheo has no ForceNewAFKDuration — keep last known RVA (anti-afk write).
    Workspace::ForceNewAFKDuration = Lookup(off, "Workspace", "ForceNewAFKDuration");
    if (Workspace::ForceNewAFKDuration == 0) Workspace::ForceNewAFKDuration = 0x1F8;
    SET(Workspace, ReadOnlyGravity);
    // silent aim mouse offsets live under different keys depending on dumper version;
    // try canonical names first, then common alternates.
    silent::FramePositionX = Lookup(off, "silent", "FramePositionX");
    if (silent::FramePositionX == 0) silent::FramePositionX = Lookup(off, "MouseService", "FramePositionX");
    if (silent::FramePositionX == 0) silent::FramePositionX = 0x4D0;
    silent::FramePositionY = Lookup(off, "silent", "FramePositionY");
    if (silent::FramePositionY == 0) silent::FramePositionY = Lookup(off, "MouseService", "FramePositionY");
    if (silent::FramePositionY == 0) silent::FramePositionY = 0x4D8;
#undef SET
#undef SET2
    if (missing > 0)
        logger::print<logger::level::warn>("%d offsets missing from imtheo payload (left as 0, check names)", missing);
}

// Call once at startup. Returns true on success (fresh or disk cache).
// Throws only if no cache exists AND network fails.
inline bool Init() {
    fs::path versionFile = "offsets.version";
    fs::path offsetsFile = "offsets.json";
    try {
        std::string liveVersion;
        try { liveVersion = TrimStr(HttpGet(L"offsets.imtheo.lol", L"/roblox/version")); }
        catch (const std::exception& e) {
            logger::print<logger::level::warn>("version check failed (%s), trying disk cache", e.what());
        }
        std::string cachedVersion;
        { std::ifstream f(versionFile); if (f) std::getline(f, cachedVersion); cachedVersion = TrimStr(cachedVersion); }

        bool needDownload = !fs::exists(offsetsFile) || (!liveVersion.empty() && liveVersion != cachedVersion);
        if (needDownload && !liveVersion.empty()) {
            logger::print<logger::level::info>("fetching fresh offsets for %s", liveVersion.c_str());
            std::string body = HttpGet(L"offsets.imtheo.lol", L"/offsets.json");
            { std::ofstream f(offsetsFile, std::ios::trunc | std::ios::binary); f << body; }
            { std::ofstream f(versionFile, std::ios::trunc); f << liveVersion; }
        } else if (needDownload && liveVersion.empty()) {
            // no network + no usable cache handled below
        }
        std::ifstream f(offsetsFile);
        if (!f) throw std::runtime_error("offsets.json missing and download failed");
        json data = json::parse(f);
        FillFromJson(data);
        Loaded = true;
        logger::print<logger::level::info>("offsets ready (%s)", ClientVersion.c_str());
        return true;
    } catch (const std::exception& e) {
        logger::print<logger::level::error>("Offsets::Init failed: %s", e.what());
        Loaded = false;
        return false;
    }
}

inline bool IsReady() { return Loaded; }
} // namespace Offsets
