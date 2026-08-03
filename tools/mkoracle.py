#!/usr/bin/env python3
"""mkoracle.py - build a difftest oracle from a whole pre-port assembly file.

The oracle for a port is the original assembly, assembled from the last git
revision that still had it. The per-routine mk*.sh scripts did that by cutting
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

ROOT = subprocess.run(["git", "rev-parse", "--show-toplevel"],
                      capture_output=True, text=True,
                      check=True).stdout.strip()


def git(*args):
    return subprocess.run(["git", "-C", ROOT, *args],
                          capture_output=True, text=True)


def find_rev(path, ported_marker):
    """Newest revision of `path` from before the port.

    Keyed on a pattern the *port* introduces, not one the original has: a port
    leaves thunks behind that keep plenty of the original's lines, so "still
    looks like the original" is not a safe test. Absence of the thunk is.
    """
    revs = git("log", "--format=%H", "--", path).stdout.split()
    for rev in revs:
        blob = git("show", "%s:%s" % (rev, path))
        if blob.returncode != 0:
            continue
        if not re.search(ported_marker, blob.stdout):
            return rev
    sys.exit("mkoracle: no revision of %s without /%s/" % (path, ported_marker))


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
    starts = {}
    for i, line in enumerate(lines):
        m = re.match(r"NEWSYM\s+(\w+)\s*$", line)
        if m:
            starts[m.group(1)] = i
    order = sorted(starts.values())
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


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("source", help="repo-relative path, e.g. video/mv16tms.asm")
    ap.add_argument("-o", "--output", required=True, help="object file to write")
    ap.add_argument("--prefix", default="asm_",
                    help="prepended to every symbol the file defines")
    ap.add_argument("--rev", help="revision to take the source from")
    ap.add_argument("--worktree", action="store_true",
                    help="take the source from the working tree instead of "
                         "git, to build a 'current' object to compare against")
    ap.add_argument("--ported-marker", default=r"call c_",
                    help="regex the ported file has and the original does not")
    ap.add_argument("--define", action="append", default=["ELF"],
                    help="extra -D for nasm")
    ap.add_argument("--stubs", metavar="PATH",
                    help="write a C file defining every symbol the oracle "
                         "leaves undefined that --provided-by does not")
    ap.add_argument("--provided-by", nargs="*", default=[],
                    help="objects whose definitions the test links for real")
    ap.add_argument("--exclude", nargs="*", default=[],
                    help="symbols the difftest itself defines")
    ap.add_argument("--stub-routine", nargs="*", default=[],
                    help="replace these routines' bodies with a jump to an "
                         "external <name>_stub, so a difftest can observe a "
                         "call that would otherwise be PC-relative and "
                         "impossible to intercept")
    a = ap.parse_args()

    rev = "worktree" if a.worktree else (a.rev
                                        or find_rev(a.source, a.ported_marker))
    tmp = tempfile.mkdtemp(prefix="mkoracle.")
    try:
        if a.worktree:
            copy_tree(a.source, tmp, set())
        else:
            extract(rev, a.source, tmp, set())
        if a.stub_routine:
            stub_out(os.path.join(tmp, a.source), a.stub_routine)
        raw = os.path.join(tmp, "raw.o")
        cmd = ["nasm", "-Ox", "-f", "elf32", "-w-orphan-labels", "-i", tmp + "/"]
        cmd += ["-D" + d for d in a.define]
        cmd += ["-o", raw, os.path.join(tmp, a.source)]
        r = subprocess.run(cmd, capture_output=True, text=True)
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
        print("oracle %s from %s (%s)" % (a.output, a.source, rev[:8]))
        print("  %d entry points: %s" % (len(defined), " ".join(defined)))
        print("  %d symbols to define: %s" % (len(undef), " ".join(undef)))

        if a.stubs:
            have = set()
            for obj in a.provided_by:
                out = subprocess.run(["nm", "-g", "--defined-only", obj],
                                     capture_output=True, text=True)
                have |= {l.split()[-1] for l in out.stdout.splitlines()
                         if l.strip()}
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
