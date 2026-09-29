# yn-ext

Roblox external base (ported from local `my_second_external/Project1-master`).

## Status
- [x] RAT/backdoor audit: **CLEAN** — only network calls are Roblox avatar thumbnails (`thumbnails.roblox.com`, `tr.rbxcdn.com`) via libcurl in `cheat/menu/menu.hpp`. `utils/network/*` is placeholder. No persistence, no stealer, no C2. See `AUDIT.md`.
- [x] Structure mapped, entry = `source/main.cpp` → `initialise::setup()` → overlay + aimbot/silent/rage/cache threads.
- [x] Offsets: **fully dynamic via imtheo.lol** — no static `offsets.hpp` values. `source/sdk/offsets.hpp` = runtime loader (`Offsets::Init()` at startup, WinHTTP + browser UA, caches `offsets.json`/`offsets.version`). Zero code changes needed elsewhere (`Offsets::X::Y` names unchanged, now vars not constexpr).
- [ ] CI build (Actions) — best-effort, needs vcxproj cleanup for hardcoded paths.

## Auto offsets (imtheo.lol)
Static offsets removed. `source/sdk/offsets.hpp` IS the loader — same `Offsets::Humanoid::Walkspeed` names, but filled at runtime.

```cpp
// source/main.cpp (already wired):
if (!Offsets::Init()) return EXIT_FAILURE; // needs offsets.json cache or network
```

Endpoints used:
- `GET https://offsets.imtheo.lol/roblox/version`
- `GET https://offsets.imtheo.lol/offsets.json` (cached to `offsets.json` + `offsets.version`)

Missing keys log a warning and stay 0. `silent::FramePositionX/Y` fall back to `0x4D0/0x4D8` if the dumper renames them.
`tools/update_offsets.py` is reference-only (fetch `/offsets.hpp` for name comparison).

## Build (local, VS2022)
1. Open `Project1.sln`, select `Release | x64`
2. Requires: Windows SDK 10, MSVC v143, MASM, D3DX11 (June 2010 SDK, legacy — plan to migrate), `freetype.lib`, `libcurl.lib`, `lua54.lib` (vendored), `library_x64.lib` (KeyAuth stub, currently unreferenced — can drop from link).
3. Output: `build/project/`

Fix needed for CI: `source/Project1.vcxproj` has absolute `C:\Users\moonp\...` include/lib paths. Replace with relative `$(ProjectDir)` / vcpkg.

## Repo workflow
- Every change → commit → push to `main` → Actions builds EXE artifact (`yn-ext-build`).
