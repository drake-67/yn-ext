#!/usr/bin/env python3
"""Fetch https://offsets.imtheo.lol/offsets.hpp and replace source/sdk/offsets.hpp.
Usage: python tools/update_offsets.py [--check]
"""
import sys, urllib.request, pathlib
URL = "https://offsets.imtheo.lol/offsets.hpp"
DST = pathlib.Path(__file__).resolve().parent.parent / "source" / "sdk" / "offsets.hpp"
def main():
    print(f"[+] fetching {URL}")
    with urllib.request.urlopen(URL, timeout=30) as r:
        data = r.read()
    print(f"[+] got {len(data)} bytes")
    if b"namespace Offsets" not in data and b"inline constexpr" not in data:
        print("[-] unexpected content, aborting"); sys.exit(2)
    if "--check" in sys.argv:
        cur = DST.read_bytes() if DST.exists() else b""
        print("up-to-date" if cur == data else "outdated")
        sys.exit(0 if cur == data else 1)
    DST.write_bytes(data)
    print(f"[+] wrote {DST}")
main()
