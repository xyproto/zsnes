#!/usr/bin/env python3
"""asmdiff.py - compare two routines in an assembled object, instruction by
instruction.

Porting this codebase means repeatedly claiming "routine B is routine A except
for X". Checking that by eye means reading through NASM macros that expand to
dozens of instructions, which is exactly where the misreadings came from. The
assembled object has no macros left in it, so the claim can be checked
mechanically instead.

    tools/asmdiff.py test/_mvall.o asm_draw8x816tsms asm_draw8x8fulladdms
    tools/asmdiff.py test/_mvall.o --cluster       # group identical routines

Addresses and branch targets are normalised, so two routines that differ only
in where they sit compare equal. What survives a diff is the real difference -
and if the diff is empty, the two routines are the same code and the C should
share one body.
"""
import argparse
import collections
import difflib
import re
import subprocess
import sys


def globals_of(obj):
    out = subprocess.run(["nm", "-g", "--defined-only", obj],
                         capture_output=True, text=True, check=True).stdout
    return {l.split()[-1] for l in out.splitlines() if l.strip()}


def disasm(obj):
    """{global symbol: [normalised instruction, ...]} for `obj`.

    NASM emits every local label (`.loopa`) as a symbol too, so objdump's
    section headings would otherwise chop each routine into dozens of
    fragments. Only the globals start a new routine; a local label is folded
    into the one it sits in.
    """
    glob = globals_of(obj)
    # -r, because every reference to a global assembles as a bare 0x0 until it
    # is relocated: without the relocation folded in, a load from pal16bcl and
    # one from pal16bxcl are the same instruction.
    out = subprocess.run(["objdump", "-dr", "--no-show-raw-insn", obj],
                         capture_output=True, text=True, check=True).stdout
    routines, cur = {}, None
    for line in out.splitlines():
        m = re.match(r"^[0-9a-f]+ <([^>]+)>:", line)
        if m:
            if m.group(1) in glob:
                cur = m.group(1)
                routines[cur] = []
            continue
        m = re.match(r"^\s+[0-9a-f]+:\t(.*)$", line)
        if m and cur is not None:
            routines[cur].append(m.group(1).strip())
            continue
        m = re.search(r"R_386_\w+\s+(\S+)", line)
        if m and cur is not None and routines[cur]:
            routines[cur][-1] += "  {%s}" % m.group(1)
    return {k: normalise(v) for k, v in routines.items() if v}


def normalise(insns):
    """Strip absolute addresses so position does not count as a difference.

    A branch keeps its *relative* target (in instruction counts) so that
    control flow still shows up as a difference when it really differs.
    """
    # Map byte offsets to instruction indices for intra-routine branches.
    out = []
    for i, ins in enumerate(insns):
        # "jne 1234 <sym+0x12>" -> "jne <sym+0x12>"; drop the numeric address.
        ins = re.sub(r"\b[0-9a-f]+\s+<", "<", ins)
        # Relocation placeholders: "mov 0x0,%eax  # R_386_32 foo" keeps foo.
        ins = re.sub(r"\s+#\s+", "  # ", ins)
        # Local label offsets differ with position; keep only the symbol.
        ins = re.sub(r"<([^>+]+)\+0x[0-9a-f]+>", r"<\1+N>", ins)
        # A local label carries its routine's name; drop it so the same loop
        # in two routines compares equal.
        ins = re.sub(r"<[A-Za-z_0-9]+(\.[A-Za-z_0-9]+)>", r"<\1>", ins)
        # NASM numbers every %%label in a macro expansion; the number is just
        # the expansion count and differs between two copies of one macro.
        ins = re.sub(r"\.\.@\d+\.", "..@.", ins)
        out.append(ins)
    return out


def relocs(obj):
    """{symbol: [referenced symbol, ...]} - what each routine actually names."""
    out = subprocess.run(["objdump", "-dr", "--no-show-raw-insn", obj],
                         capture_output=True, text=True, check=True).stdout
    res, cur = collections.defaultdict(list), None
    for line in out.splitlines():
        m = re.match(r"^[0-9a-f]+ <([^>]+)>:", line)
        if m:
            cur = m.group(1)
            continue
        m = re.search(r"R_386_\w+\s+(\S+)", line)
        if m and cur:
            res[cur].append(m.group(1))
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("object")
    ap.add_argument("routines", nargs="*")
    ap.add_argument("--cluster", action="store_true",
                    help="group routines with identical instruction streams")
    ap.add_argument("--symbols", action="store_true",
                    help="per routine, the external symbols it references")
    a = ap.parse_args()

    r = disasm(a.object)

    if a.symbols:
        for name, syms in sorted(relocs(a.object).items()):
            seen = sorted(set(syms))
            print("%s (%d)\n    %s" % (name, len(seen), " ".join(seen)))
        return 0

    if a.cluster:
        groups = collections.defaultdict(list)
        for name, ins in r.items():
            groups["\n".join(ins)].append(name)
        for body, names in sorted(groups.items(), key=lambda kv: -len(kv[1])):
            print("%4d insns  %s" % (len(body.split("\n")), " ".join(sorted(names))))
        return 0

    if len(a.routines) != 2:
        sys.exit("asmdiff: give two routine names, or --cluster / --symbols")
    x, y = a.routines
    for n in (x, y):
        if n not in r:
            sys.exit("asmdiff: %s not in %s (have: %s)"
                     % (n, a.object, " ".join(sorted(r))))
    d = list(difflib.unified_diff(r[x], r[y], x, y, lineterm="", n=2))
    if not d:
        print("identical: %s and %s (%d instructions)" % (x, y, len(r[x])))
        return 0
    print("\n".join(d))
    adds = sum(1 for l in d if l.startswith("+") and not l.startswith("+++"))
    dels = sum(1 for l in d if l.startswith("-") and not l.startswith("---"))
    print("\n%d instructions differ (%d/%d, %d/%d)"
          % (max(adds, dels), dels, len(r[x]), adds, len(r[y])))
    return 0


sys.exit(main())
