#!/usr/bin/env python3
"""Backup helper: fetch https://offsets.imtheo.lol/offsets.hpp for inspection.
NOTE: primary path is runtime auto-update (source/sdk/offsets.hpp :: Offsets::Init()
fetches /roblox/version + /offsets.json with a browser UA). This script is only for
offline inspection — the .hpp is NO LONGER checked into the build.

Usage: python tools/update_offsets.py [--check]
  --check : compare cached offsets.json version vs live (exits 1 if outdated)
"""
import sys, pathlib, json

try:
    import urllib.request
except ImportError:
    urllib = None

URL_HPP = "https://offsets.imtheo.lol/offsets.hpp"
URL_VER = "https://offsets.imtheo.lol/roblox/version"
UA = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) yn-ext/1.0"}

def get(url, timeout=30):
    req = urllib.request.Request(url, headers=UA)
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return r.read()

def main():
    if "--check" in sys.argv:
        try:
            live = get(URL_VER).decode(errors="ignore").strip().strip('"')
            print(f"live: {live}")
            for cand in [pathlib.Path("offsets.version"), pathlib.Path("source/../offsets.version")]:
                if cand.exists():
                    print(f"cached: {cand.read_text().strip()} ({cand})")
            sys.exit(0)
        except Exception as e:  # noqa: BLE001
            print(f"check failed (likely Cloudflare from datacenter IP): {e}")
            print("hint: the cheat itself fetches at runtime from a real Windows PC — that path uses WinHTTP + browser UA and usually passes.")
            sys.exit(2)
    try:
        data = get(URL_HPP)
    except Exception as e:  # noqa: BLE001
        print(f"fetch failed (likely Cloudflare from datacenter IP): {e}")
        print("hint: open https://offsets.imtheo.lol/offsets.hpp in your browser and compare names in source/sdk/offsets.hpp FillFromJson().")
        sys.exit(2)
    print(f"got {len(data)} bytes")
    if b"namespace Offsets" not in data and b"inline constexpr" not in data:
        print("unexpected content, aborting"); sys.exit(2)
    out = pathlib.Path(__file__).resolve().parent / "offsets_latest.hpp"
    out.write_bytes(data)
    print(f"wrote {out} (reference only, NOT used by build)")

main()
