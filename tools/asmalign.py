#!/usr/bin/env python3
"""asmalign.py - check the alignment C may assume for inline-asm data.

The x86-64 ABI promises 16-byte alignment for any array of 16 bytes or more,
and clang takes an extern declaration at its word and uses aligned SSE moves.
The data blocks in endmem.c and the *data.c files pin their own layout, so an
array there has whatever alignment its offset gives it; the declaration must
say so with ASM_ALIGNED(n) or the load faults (SIGSEGV on Linux, SIGBUS on
FreeBSD). Reads the objects of a finished build and every extern declaration
in the tree, and reports the mismatches.

    tools/asmalign.py build
"""
import glob
import re
import subprocess
import sys

ASM_SECTIONS = {".data", ".bss", ".data.sa1state", ".data.spc7110state", ".data.spc7110ptr"}
SIZES = {
    "u1": 1, "s1": 1, "uint8_t": 1, "int8_t": 1, "char": 1,
    "u2": 2, "s2": 2, "uint16_t": 2, "int16_t": 2, "short": 2,
    "u4": 4, "s4": 4, "uint32_t": 4, "int32_t": 4, "int": 4, "float": 4,
    "u8": 8, "s8": 8, "uint64_t": 8, "int64_t": 8, "zreg": 8, "long": 8, "double": 8,
}
POINTER = 8


def asm_symbols(build_dir):
    """Symbol -> alignment its section and offset guarantee."""
    syms = {}
    for obj in glob.glob(build_dir + "/**/*.o", recursive=True):
        if obj.startswith(build_dir + "/test"):
            continue
        sections = {}
        out = subprocess.run(["readelf", "-SW", obj], capture_output=True, text=True).stdout
        for m in re.finditer(r"\[\s*(\d+)\]\s+(\S+)\s+\S+\s+[0-9a-f]+\s+[0-9a-f]+\s+[0-9a-f]+\s+\S+\s+\S+\s+\S+\s+\S+\s+(\d+)", out):
            sections[int(m.group(1))] = (m.group(2), int(m.group(3)))
        out = subprocess.run(["readelf", "-sW", obj], capture_output=True, text=True).stdout
        for line in out.splitlines():
            p = line.split()
            if len(p) < 8 or p[3] not in ("OBJECT", "NOTYPE") or not p[6].isdigit():
                continue
            name, align = sections.get(int(p[6]), ("", 0))
            if name not in ASM_SECTIONS:
                continue
            off = int(p[1], 16)
            syms[p[7]] = align if off == 0 else min(align, off & -off)
    return syms


def declarations():
    """Yield (file, name, bytes, attribute alignment or None) per extern array."""
    for f in glob.glob("**/*.[ch]", recursive=True):
        if f.split("/")[0] in ("build", "win", "test"):
            continue
        src = open(f, errors="ignore").read()
        for m in re.finditer(r"extern\s+([^;{}]*?);", src, re.S):
            stmt = m.group(1)
            if "(" in re.sub(r"ASM_ALIGNED\(\d+\)", "", stmt):
                continue
            tm = re.match(r"\s*(?:const\s+)?(?:unsigned\s+|signed\s+)?(\w+)\s*(\*?)", stmt)
            if not tm:
                continue
            typ, star = tm.groups()
            for dm in re.finditer(r"(\*?)\s*\b(\w+)\s*((?:\[[^\]]*\]\s*)+)(?:ASM_ALIGNED\((\d+)\))?", stmt):
                pstar, name, dims, attr = dm.groups()
                n = 1
                for d in re.findall(r"\[([^\]]*)\]", dims):
                    try:
                        n *= int(eval(d, {}, {})) if d.strip() else 0
                    except Exception:
                        n = 0
                size = POINTER if (star or pstar) else SIZES.get(typ, 16)
                yield f, name, n * size, int(attr) if attr else None


def main():
    build_dir = sys.argv[1] if len(sys.argv) > 1 else "build"
    syms = asm_symbols(build_dir)
    if not syms:
        print("asmalign: no objects under %s; build first" % build_dir, file=sys.stderr)
        return 2
    bad = 0
    for f, name, nbytes, attr in declarations():
        if name not in syms:
            continue
        have = syms[name]
        assumed = attr if attr is not None else (16 if nbytes >= 16 else 1)
        if attr is None and nbytes < 16:
            continue
        if assumed > have:
            print("%s: %s is %d-byte aligned, declaration assumes %d" % (f, name, have, assumed))
            bad += 1
    print("asmalign: %d problem(s)" % bad)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
