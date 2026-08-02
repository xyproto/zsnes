#!/usr/bin/env python3
"""asmport.py - scaffolding for porting a NASM routine to C11.

Two things are mechanical and were being done by hand, badly: working out which
registers a routine takes in and hands back, and writing the spill/restore
thunk that replaces it. Both are easy to get wrong in ways the emulator does
not show - a register that is live across the seam and gets dropped renders
identically until it does not.

    tools/asmport.py regs   video/mv16tms.asm draw16x1616tms
    tools/asmport.py thunk  video/mv16tms.asm draw16x1616tms --prefix MV16
    tools/asmport.py body   video/mv16tms.asm draw16x1616tms

`regs` reports, per register: whether the routine reads it before writing it
(so it is an input), and whether it writes it at all (so it may be an output).
The analysis is deliberately flow-insensitive and conservative - it unions over
every path - because a seam that carries too much is merely wasteful, while one
that carries too little is a bug.
"""
import argparse
import re
import sys

R32 = ["eax", "ebx", "ecx", "edx", "esi", "edi", "ebp"]
# Every spelling of each register, so `mov al,..` counts as touching eax.
ALIAS = {
    "eax": ["eax", "ax", "ah", "al"],
    "ebx": ["ebx", "bx", "bh", "bl"],
    "ecx": ["ecx", "cx", "ch", "cl"],
    "edx": ["edx", "dx", "dh", "dl"],
    "esi": ["esi", "si"],
    "edi": ["edi", "di"],
    "ebp": ["ebp", "bp"],
}
OF = {a: r for r, al in ALIAS.items() for a in al}

# Instructions whose first operand is written without being read. Everything
# else that names a register is treated as reading it, which is the safe way
# round for this purpose.
WRITE_ONLY_DEST = {"mov", "movzx", "movsx", "lea", "pop", "xor", "sub", "set"}
# ... except xor/sub with identical operands, the idiomatic zeroing, which do
# not read. Handled below.
NO_OPERAND_WRITES = {
    "cdq": ["edx"], "cwd": ["edx"], "cbw": ["eax"], "cwde": ["eax"],
    "lodsb": ["eax", "esi"], "lodsw": ["eax", "esi"], "lodsd": ["eax", "esi"],
    "stosb": ["edi"], "stosw": ["edi"], "stosd": ["edi"],
    "movsb": ["esi", "edi"], "movsw": ["esi", "edi"], "movsd": ["esi", "edi"],
}


def strip(line):
    line = re.sub(r";.*$", "", line)
    return line.rstrip()


def collect_macros(path, seen=None):
    """Every %macro in `path` and anything it %includes, by name."""
    if seen is None:
        seen = set()
    if path in seen:
        return {}
    seen.add(path)
    macros, cur, body = {}, None, []
    try:
        lines = open(path).read().split("\n")
    except OSError:
        return {}
    for line in lines:
        m = re.match(r'%include\s+"([^"]+)"', line.strip())
        if m:
            macros.update(collect_macros(m.group(1), seen))
            continue
        m = re.match(r"%i?macro\s+(\S+)\s+(\S+)", line.strip(), re.I)
        if m:
            cur, body = m.group(1).lower(), []
            continue
        if cur is not None:
            if line.strip().lower() == "%endmacro":
                macros[cur] = body
                cur = None
            else:
                body.append(line)
    return macros


def expand(lines, macros, depth=0):
    """Inline macro invocations so their register use is visible. Without this
       a routine whose whole body is `drawtilegrpfull draw8x816tcms` looks like
       it touches nothing."""
    if depth > 8:
        return lines
    out = []
    for line in lines:
        text = strip(line).strip()
        parts = text.split(None, 1)
        if parts and parts[0].lower() in macros:
            args = [a.strip() for a in parts[1].split(",")] if len(parts) > 1 else []
            body = []
            for b in macros[parts[0].lower()]:
                for i, arg in enumerate(args, 1):
                    b = b.replace("%%%d" % i, arg)
                body.append(b)
            out.extend(expand(body, macros, depth + 1))
        else:
            out.append(line)
    return out


def routine_body(path, name):
    """The lines of `name`, from its NEWSYM to the next global label."""
    out, on = [], False
    for raw in open(path):
        line = raw.rstrip("\n")
        m = re.match(r"NEWSYM\s+(\w+)", line)
        if m:
            if on:
                break
            on = m.group(1) == name
            continue
        if re.match(r"[A-Za-z_]\w*\s*:?\s*$", line) and on and not line.startswith("."):
            break
        if on:
            out.append(line)
    if not out:
        sys.exit("asmport: routine %s not found in %s" % (name, path))
    return out


def regs_used(lines):
    """(inputs, written) - inputs are read before being written on some path."""
    written, inputs = set(), set()

    def note_read(tok):
        for a in re.findall(r"\b([a-z]{2,3})\b", tok):
            r = OF.get(a)
            if r and r not in written:
                inputs.add(r)

    for line in lines:
        line = strip(line).strip()
        if not line or line.startswith((".", "%")) or line.endswith(":"):
            continue
        parts = line.split(None, 1)
        op = parts[0].lower()
        args = parts[1] if len(parts) > 1 else ""
        if op in NO_OPERAND_WRITES:
            written.update(NO_OPERAND_WRITES[op])
            continue
        ops = [a.strip() for a in args.split(",")] if args else []
        if not ops:
            continue
        dest = ops[0]
        # Zeroing idioms read nothing.
        zeroing = op in ("xor", "sub") and len(ops) == 2 and ops[0] == ops[1]
        for o in ops[1:]:
            note_read(o)
        if not zeroing and (op not in WRITE_ONLY_DEST or "[" in dest):
            note_read(dest)
        # A memory destination writes memory, not a register.
        if "[" not in dest:
            r = OF.get(dest.lower())
            if r:
                written.add(r)
    return inputs, written


def emit_thunk(name, prefix, carried, save):
    slots = [(r, "%s%s" % (prefix, r[1:].upper())) for r in carried]
    w = max(len(s) for _, s in slots)
    lines = ["NEWSYM %s" % name]
    for r in save:
        lines.append("    push %s   ; untouched by the original; cdecl would clobber it" % r)
    for r, s in slots:
        lines.append("    mov [%-*s], %s" % (w, s, r))
    lines.append("    call c_%s" % name)
    for r, s in slots:
        lines.append("    mov %s, [%s]" % (r, s))
    for r in reversed(save):
        lines.append("    pop %s" % r)
    lines.append("    ret")
    return "\n".join(lines), [s for _, s in slots]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["regs", "thunk", "body"])
    ap.add_argument("file")
    ap.add_argument("routine")
    ap.add_argument("--prefix", default="SEAM")
    a = ap.parse_args()

    lines = routine_body(a.file, a.routine)
    lines = expand(lines, collect_macros(a.file))
    inputs, written = regs_used(lines)

    if a.mode == "body":
        print("\n".join(lines))
        return
    if a.mode == "regs":
        print("%s: %d lines" % (a.routine, len(lines)))
        for r in R32:
            tag = []
            if r in inputs:
                tag.append("in")
            if r in written:
                tag.append("out?")
            print("  %-4s %s" % (r, "/".join(tag) if tag else "-"))
        carried = [r for r in R32 if r in inputs or r in written]
        print("\ncarry across the seam: %s" % " ".join(carried))
        # cdecl lets the callee clobber eax/ecx/edx. Any of those the assembly
        # never touched must be pushed around the call, or the port silently
        # destroys something the caller was keeping there.
        save = [r for r in ("eax", "ecx", "edx") if r not in carried]
        if save:
            print("push/pop around the call: %s" % " ".join(save))
            print("  (untouched by the assembly, so callers may rely on them;")
            print("   cdecl would let the C half clobber them)")
        return

    carried = [r for r in R32 if r in inputs or r in written]
    save = [r for r in ("eax", "ecx", "edx") if r not in carried]
    thunk, slots = emit_thunk(a.routine, a.prefix, carried, save)
    print(thunk)
    print()
    print("; EXTSYM %s,c_%s" % (",".join(slots), a.routine))
    print()
    print("/* C side */")
    for s in slots:
        print("u4 %s;" % s)
    print("void c_%s(void)\n{\n}" % a.routine)


main()
