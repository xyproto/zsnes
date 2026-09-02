#!/usr/bin/env python3
"""mkoracle.py - build a difftest oracle from a whole pre-port assembly file.

The oracle for a port is the original assembly, taken from the last revision
in test/asm-sources.zip that still had it. The per-routine mk*.sh scripts did that by cutting
the routine and the macros it needs out of the file and hand-writing an EXTERN
for every symbol they referenced. That is where two separate classes of bug came
from:

  - a missed EXTERN leaves the symbol undefined, and NASM then reports
    "label changed during code generation" - which reads like an optimiser
    problem and was misdiagnosed as one for a long time;
  - a cut that is too small, or a revision detector keyed off a line the port
    keeps, silently produces an oracle that is the code under test.

Neither can happen if the whole file is assembled unmodified. Every symbol it
defines is renamed with a prefix afterwards, via objcopy, so the oracle and the
C under test can be linked together.

    tools/mkoracle.py video/mv16tms.asm -o test/_mvs.o --prefix asm_

Prints the symbols the oracle leaves undefined: those are exactly what the
difftest has to define, and the list is complete by construction.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASMGIT = os.path.join(ROOT, "test", "asmgit.sh")


def git(*args):
    """The original assembly comes from test/asm-sources.zip, not git history:
    a shallow clone has no history to read."""
    return subprocess.run([ASMGIT, *args], capture_output=True, text=True)


def find_rev(path, ported_marker, requires):
    """Newest revision of `path` from before the port.

    Two ways to say which one that is, and neither is enough alone.

    --ported-marker is a pattern the *port* introduces, and the newest revision
    without it is taken to be pre-port. The trap: once the port finishes it
    deletes its own scaffolding too, so the marker is absent at both ends of
    the history and this picks the *finished* file. The 65816 targets hit
    exactly that - the oracle came back with no opcodes in it at all.

    --requires is a pattern only the original has, usually the %include of the
    code under test. On its own it lands on a half-ported revision, where the
    routines are already thunks into the C.

    Give both. The newest revision that still includes the code and has not
    started thunking it out is the last one where the oracle is really the
    original.
    """
    revs = git("log", "--format=%H", "--", path).stdout.split()
    chosen, marked = None, None
    for i, rev in enumerate(revs):
        blob = git("show", "%s:%s" % (rev, path))
        if blob.returncode != 0:
            continue
        if ported_marker and re.search(ported_marker, blob.stdout):
            if marked is None:
                marked = i
            continue
        if requires and not re.search(requires, blob.stdout):
            continue
        if chosen is None:
            chosen = i
    if chosen is None:
        sys.exit("mkoracle: no revision of %s matching /%s/ without /%s/"
                 % (path, requires or ".", ported_marker or "."))
    # The marker is absent at both ends of a finished port, so "newest without
    # it" can land *after* the port instead of before it. If some revision does
    # carry the marker and the one picked is newer than all of them, that is
    # what happened - refuse rather than hand back an oracle that is the port.
    if marked is not None and chosen < marked:
        sys.exit(
            "mkoracle: %s at %s is newer than every revision carrying /%s/, so "
            "it is the finished port, not the original. Add --requires with a "
            "pattern only the original has (usually the %%include of the code "
            "under test)." % (path, revs[chosen][:8], ported_marker))
    return revs[chosen]


def extract(rev, path, dest, seen):
    """`path` at `rev` into the mirror tree, following %include."""
    if path in seen:
        return
    seen.add(path)
    blob = git("show", "%s:%s" % (rev, path))
    if blob.returncode != 0:
        sys.exit("mkoracle: %s missing at %s" % (path, rev[:8]))
    out = os.path.join(dest, path)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, "w") as f:
        f.write(blob.stdout)
    for inc in re.findall(r'^\s*%include\s+"([^"]+)"', blob.stdout, re.M):
        extract(rev, inc, dest, seen)


def copy_tree(path, dest, seen):
    """Like extract(), but from the working tree."""
    if path in seen:
        return
    seen.add(path)
    src = os.path.join(ROOT, path)
    out = os.path.join(dest, path)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    text = open(src).read()
    with open(out, "w") as f:
        f.write(text)
    for inc in re.findall(r'^\s*%include\s+"([^"]+)"', text, re.M):
        copy_tree(inc, dest, seen)


def stub_out(path, names):
    """Replace each named routine's body with `jmp <name>_stub`.

    A call between two routines in the same file assembles to a PC-relative
    displacement with no relocation, so no amount of linker work can point it
    somewhere else. Rewriting the callee before assembly is the only way for a
    difftest to see that the call happened.
    """
    lines = open(path).read().split("\n")
    starts, order = {}, []
    for i, line in enumerate(lines):
        m = re.match(r"NEWSYM\s+(\w+)\s*$", line)
        if m:
            starts[m.group(1)] = i
            order.append(i)
        # A routine's body ends at the next NEWSYM *or* at the next macro
        # definition. Once a cluster is ported to C its thunk can be followed
        # by macros that the routines further down still use, and deleting
        # those leaves their invocations looking like instructions.
        elif re.match(r"%i?macro\s", line):
            order.append(i)
    order.sort()
    out, missing = list(lines), [n for n in names if n not in starts]
    if missing:
        sys.exit("mkoracle: no such routine: %s" % " ".join(missing))
    for name in sorted(names, key=lambda n: -starts[n]):
        i = starts[name]
        j = next((p for p in order if p > i), len(out))
        out[i:j] = ["EXTERN %s_stub" % name,
                    "NEWSYM %s" % name,
                    "    jmp %s_stub" % name,
                    ""]
    with open(path, "w") as f:
        f.write("\n".join(out))


def rewrite_macro(root, specs):
    """Replace the body of `%macro NAME 0` wherever it is defined under root.

    The 65816 opcodes end in `endloop`, which dispatches straight into the next
    one. Rewriting that macro to `ret` is what makes a single opcode callable,
    and so what lets a difftest drive one at a time instead of a whole ROM.
    """
    want = {}
    for spec in specs:
        name, _, body = spec.partition("=")
        want[name] = body.replace("\\n", "\n").split("\n")
    seen = set()
    for dirpath, _, files in os.walk(root):
        for fn in files:
            path = os.path.join(dirpath, fn)
            lines = open(path).read().split("\n")
            out, i, hit = [], 0, False
            while i < len(lines):
                m = re.match(r"%i?macro\s+(\w+)\s+0\s*$", lines[i])
                if m and m.group(1) in want:
                    j = i
                    while j < len(lines) and not re.match(r"%endmacro",
                                                          lines[j]):
                        j += 1
                    out += [lines[i]] + want[m.group(1)] + ["%endmacro"]
                    seen.add(m.group(1))
                    i, hit = j + 1, True
                    continue
                out.append(lines[i])
                i += 1
            if hit:
                with open(path, "w") as f:
                    f.write("\n".join(out))
    missing = [n for n in want if n not in seen]
    if missing:
        sys.exit("mkoracle: no such macro: %s" % " ".join(missing))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("source", help="repo-relative path, e.g. video/mv16tms.asm")
    ap.add_argument("-o", "--output", required=True, help="object file to write")
    ap.add_argument("--prefix", default="asm_",
                    help="prepended to every symbol the file defines")
    ap.add_argument("--rev", help="revision to take the source from")
    ap.add_argument("--worktree", action="store_true",
                    help="take the source from the working tree instead of "
                         "the archive, to build a 'current' object")
    ap.add_argument("--ported-marker", default=r"call c_",
                    help="regex the ported file has and the original does not")
    ap.add_argument("--define", action="append", default=["ELF"],
                    help="extra -D for nasm")
    ap.add_argument("--stubs", metavar="PATH",
                    help="write a C file defining every symbol the oracle "
                         "leaves undefined that --provided-by does not")
    ap.add_argument("--provided-by", nargs="*", default=[],
                    help="objects whose definitions the test links for real")
    ap.add_argument("-v", "--verbose", action="store_true",
                    help="list the entry points and undefined symbols; the "
                         "counts alone are enough once a target builds")
    ap.add_argument("--exclude", nargs="*", default=[],
                    help="symbols the difftest itself defines")
    ap.add_argument("--stub-routine", nargs="*", default=[],
                    help="replace these routines' bodies with a jump to an "
                         "external <name>_stub, so a difftest can observe a "
                         "call that would otherwise be PC-relative and "
                         "impossible to intercept")
    ap.add_argument("--requires", metavar="PATTERN",
                    help="pick the newest revision of the source that still "
                         "matches this - a pattern only the original has, such "
                         "as the %%include of the code under test. Safer than "
                         "--ported-marker, which also matches the finished "
                         "port once it drops its own scaffolding")
    ap.add_argument("--rewrite-macro", nargs="*", default=[], metavar="NAME=BODY",
                    help="replace a nullary macro's body before assembly, "
                         r"\n separating lines; endloop=ret makes each 65816 "
                         "opcode return instead of dispatching the next")
    a = ap.parse_args()

    rev = "worktree" if a.worktree else (
        a.rev or find_rev(a.source, a.ported_marker, a.requires))
    tmp = tempfile.mkdtemp(prefix="mkoracle.")
    try:
        if a.worktree:
            copy_tree(a.source, tmp, set())
        else:
            extract(rev, a.source, tmp, set())
        if a.rewrite_macro:
            rewrite_macro(tmp, a.rewrite_macro)
        if a.stub_routine:
            stub_out(os.path.join(tmp, a.source), a.stub_routine)
        raw = os.path.join(tmp, "raw.o")
        cmd = ["nasm", "-Ox", "-f", "elf32", "-w-orphan-labels", "-i", tmp + "/"]
        cmd += ["-D" + d for d in a.define]
        cmd += ["-o", raw, os.path.join(tmp, a.source)]
        # From tmp, so `%include "cpu/foo.inc"` picks up the extracted copy.
        # NASM searches the current directory before -i, and running from the
        # repo root silently gave the oracle the working tree's includes.
        r = subprocess.run(cmd, capture_output=True, text=True, cwd=tmp)
        if r.returncode != 0:
            sys.stderr.write(r.stderr)
            sys.exit("mkoracle: assembly failed")

        nm = subprocess.run(["nm", "-g", "--defined-only", raw],
                            capture_output=True, text=True, check=True)
        defined = sorted(l.split()[-1] for l in nm.stdout.splitlines() if l.strip())
        redef = os.path.join(tmp, "redef")
        with open(redef, "w") as f:
            for s in defined:
                f.write("%s %s%s\n" % (s, a.prefix, s))
        subprocess.run(["objcopy", "--redefine-syms=" + redef, raw, a.output],
                       check=True)

        und = subprocess.run(["nm", "-u", a.output],
                             capture_output=True, text=True, check=True)
        undef = sorted(l.split()[-1] for l in und.stdout.splitlines() if l.strip())
        print("oracle %s from %s (%s): %d entry points, %d undefined"
              % (a.output, a.source, rev[:8], len(defined), len(undef)))
        if a.verbose:
            print("  entry points: %s" % " ".join(defined))
            print("  to define:    %s" % " ".join(undef))

        if a.stubs:
            have = set()
            for obj in a.provided_by:
                out = subprocess.run(["nm", "-g", "--defined-only", obj],
                                     capture_output=True, text=True)
                syms = {l.split()[-1] for l in out.stdout.splitlines()
                        if l.strip()}
                # A `make win_i686` leaves PE objects in the tree, where every
                # symbol carries a leading underscore. They then look like they
                # provide nothing, everything gets a stub instead of the real
                # definition, and the link fails far from the cause.
                if syms and all(x.startswith("_") for x in syms):
                    sys.exit("mkoracle: %s has only _-prefixed symbols - it is "
                             "a Windows object. Run `make` to rebuild native."
                             % obj)
                have |= syms
            have |= set(a.exclude)
            need = [s for s in undef if s not in have]
            with open(a.stubs, "w") as f:
                f.write("/* Generated by tools/mkoracle.py - do not edit.\n"
                        " *\n"
                        " * A whole-file oracle pulls in every routine in the\n"
                        " * file, not just the ones under test, so all of their\n"
                        " * symbols have to resolve. These stubs exist only to\n"
                        " * link; the routines that touch them are not the ones\n"
                        " * being compared. Anything the test actually reads\n"
                        " * must come from a real object via --provided-by,\n"
                        " * because a stub has neither the right size nor the\n"
                        " * right neighbours.\n"
                        " */\n")
                for sym in need:
                    f.write("unsigned char %s[4096];\n" % sym)
            print("  %d stubs written to %s" % (len(need), a.stubs))
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


main()
