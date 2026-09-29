# yn-ext

Roblox external base (ported from local `my_second_external/Project1-master`).

## Status
- [x] RAT/backdoor audit: **CLEAN** — only network calls are Roblox avatar thumbnails (`thumbnails.roblox.com`, `tr.rbxcdn.com`) via libcurl in `cheat/menu/menu.hpp`. `utils/network/*` is placeholder. No persistence, no stealer, no C2. See `AUDIT.md`.
- [x] Structure mapped, entry = `source/main.cpp` → `initialise::setup()` → overlay + aimbot/silent/rage/cache threads.
- [x] Offsets: pinned in `source/sdk/offsets.hpp` (`version-4aeb17bd13994560`). Auto-updater added: `source/sdk/offsets_auto.hpp` (WinHTTP + `offsets.imtheo.lol`).
- [ ] CI build (Actions) — best-effort, needs vcxproj cleanup for hardcoded paths.

## Auto offsets (imtheo.lol)
Integration per https://offsets.imtheo.lol/docs/cpp-auto-update :

```cpp
#include "sdk/offsets_auto.hpp"
// at startup, before initialise::setup():
try {
    auto data = offsets_auto::EnsureLatestOffsets();
    uintptr_t ws = offsets_auto::GetOffset(data, "Humanoid", "Walkspeed");
} catch (const std::exception& e) { /* fallback to compiled offsets.hpp */ }
```

Endpoints used:
- `GET https://offsets.imtheo.lol/roblox/version`
- `GET https://offsets.imtheo.lol/offsets.json` (cached to `offsets.json` + `offsets.version`)

Next step: replace hardcoded `Offsets::X::Y` reads with `GetOffset()` lookups, or run `tools/update_offsets.py` to regenerate `offsets.hpp` at build time.

## Build (local, VS2022)
1. Open `Project1.sln`, select `Release | x64`
2. Requires: Windows SDK 10, MSVC v143, MASM, D3DX11 (June 2010 SDK, legacy — plan to migrate), `freetype.lib`, `libcurl.lib`, `lua54.lib` (vendored), `library_x64.lib` (KeyAuth stub, currently unreferenced — can drop from link).
3. Output: `build/project/`

Fix needed for CI: `source/Project1.vcxproj` has absolute `C:\Users\moonp\...` include/lib paths. Replace with relative `$(ProjectDir)` / vcpkg.

## Repo workflow
- Every change → commit → push to `main` → Actions builds EXE artifact (`yn-ext-build`).
