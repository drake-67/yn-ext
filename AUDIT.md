# Security audit — 2026-09-29

Scope: `source/` full read + grep for net/persist/inject/stealer/obfuscation + strings on vendored `.lib`s.

## Network endpoints (all found)
- `cheat/menu/menu.hpp:267-280` `HttpGetBinary(url)` via curl (generic helper, no hardcoded C2)
- `cheat/menu/menu.hpp:295` `https://www.roblox.com/users/<id>/profile`
- regex `https://tr.rbxcdn.com/...` (og:image scrape)
- `cheat/menu/menu.hpp:317` `https://thumbnails.roblox.com/v1/users/avatar-headshot?...` → dynamic `imageUrl` (rbxcdn)
- Caller: `cheat/overlay/overlay.cpp:1379` `RenderUserImage()` — avatar ESP only
- `library_x64.lib` strings: `https://curl.se/docs/...` (curl internals, benign)
- No discord webhooks, telegram, pastebin, IPs, WinINet/WinHTTP/sockets in cheat code. `utils/network/*` = placeholder.

## Verdict: CLEAN (not ratted)
- No persistence (Run keys, Startup, schtasks, services), no `ShellExecute/system/popen` in cheat (only bundled Lua stdlib, not opened — `luaL_openlibs` commented out).
- No injection (`CreateRemoteThread`, `WriteProcessMemory`, etc). Only `OpenProcess(PROCESS_ALL_ACCESS)` + Toolhelp snapshot in `utils/memory/memory.cpp`.
- No stealer (`.ROBLOSECURITY`, browser paths, tokens). `GetComputerNameA` / `GetUserNameW` defined but never called, no sink.
- `luck.asm` = direct syscalls `NtReadVirtualMemory (63)` / `NtWriteVirtualMemory (58)` — standard external AV-bypass, not driver.
- Caveat: `library_x64.lib` (19.8MB) = KeyAuth seller lib (`init/login/license/.../ban/log/webhook`), currently **unreferenced** in source (linked in vcxproj only, leftover from `cursed.wtf` base). Dormant in this snapshot, but closed binary — rebuild from source, consider dropping from link line.
- `CURLOPT_SSL_VERIFYPEER/VERIFYHOST=0` — MITM-able, fix later.

Re-audit after every third-party import.
