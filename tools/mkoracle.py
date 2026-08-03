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


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("source", help="repo-relative path, e.g. video/mv16tms.asm")
    ap.add_argument("-o", "--output", required=True, help="object file to write")
    ap.add_argument("--prefix", default="asm_",
                    help="prepended to every symbol the file defines")
    ap.add_argument("--rev", help="revision to take the source from")
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
    a = ap.parse_args()

    rev = a.rev or find_rev(a.source, a.ported_marker)
    tmp = tempfile.mkdtemp(prefix="mkoracle.")
    try:
        extract(rev, a.source, tmp, set())
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
