#pragma once
// Auto offset updater via https://offsets.imtheo.lol
// Flow (from docs /docs/cpp-auto-update):
// 1. GET /roblox/version -> live version string
// 2. compare to offsets.version on disk
// 3. if differ, GET /offsets.json -> overwrite offsets.json + offsets.version
// 4. load offsets.json, use GetOffset(data,"Humanoid","Walkspeed")
//
// Requires: WinHTTP (winhttp.lib), nlohmann/json.hpp (already vendored in utils/json/json.hpp)
// Call EnsureLatestOffsets() at startup BEFORE initialise::setup().

#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

#include <string>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <cstdio>
#include "../utils/json/json.hpp"

namespace offsets_auto {
namespace fs = std::filesystem;
using json = nlohmann::json;

inline std::string HttpGet(const wchar_t* host, const wchar_t* path) {
    std::string out;
    HINTERNET hSession = WinHttpOpen(L"yn-ext/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
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

inline std::string GetLiveVersion() {
    std::string v = HttpGet(L"offsets.imtheo.lol", L"/roblox/version");
    // trim whitespace/quotes
    while (!v.empty() && (v.back() == '\n' || v.back() == '\r' || v.back() == ' ' || v.back() == '"' )) v.pop_back();
    while (!v.empty() && (v.front() == ' ' || v.front() == '"' )) v.erase(v.begin());
    return v;
}

struct OffsetCache {
    fs::path versionFile = "offsets.version";
    fs::path offsetsFile = "offsets.json";

    std::string ReadCachedVersion() const {
        std::ifstream f(versionFile);
        if (!f) return {};
        std::string v; std::getline(f, v); return v;
    }
    void WriteCachedVersion(const std::string& v) const {
        std::ofstream f(versionFile, std::ios::trunc); f << v;
    }
    void WriteOffsets(const std::string& body) const {
        std::ofstream f(offsetsFile, std::ios::trunc | std::ios::binary); f << body;
    }
    json LoadOffsets() const {
        std::ifstream f(offsetsFile);
        if (!f) throw std::runtime_error("offsets.json missing");
        return json::parse(f);
    }
};

inline json EnsureLatestOffsets() {
    OffsetCache cache;
    std::string liveVersion = GetLiveVersion();
    std::string cachedVersion = cache.ReadCachedVersion();
    if (liveVersion != cachedVersion || !fs::exists(cache.offsetsFile)) {
        std::string body = HttpGet(L"offsets.imtheo.lol", L"/offsets.json");
        cache.WriteOffsets(body);
        cache.WriteCachedVersion(liveVersion);
    }
    return cache.LoadOffsets();
}

inline uintptr_t GetOffset(const json& data, const char* cls, const char* field) {
    if (!data.contains("Offsets")) return 0;
    auto& offsets = data["Offsets"];
    if (!offsets.contains(cls)) return 0;
    const auto& entry = offsets[cls];
    if (!entry.contains(field)) return 0;
    return entry[field].get<uintptr_t>();
}
} // namespace offsets_auto
