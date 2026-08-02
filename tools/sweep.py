#!/usr/bin/env python3
"""sweep.py - mutation-test a difftest.

Deliberately break the ported C one edit at a time and check the difftest turns
red. That is the only thing separating "the port is correct" from "the test
never ran the code" - a difftest that enters no routine prints PASS just as
loudly as one that checks everything.

The mutated file is always a COPY in a scratch directory; the tree is never
touched, because a sweep that is interrupted mid-run once left a broken source
behind and it got committed.

    tools/sweep.py --source video/c_mv16tsms.c --mutants test/mutants_mvall.txt \\
        --build 'gcc -m32 -std=gnu99 -I{root} {src} {root}/video/c_mv16tms.c \\
                 {root}/video/mv16tms.o {root}/test/_mvall.o \\
                 {root}/test/difftest_mvall.c -no-pie -o {bin}'

Mutants file: `name<TAB>old<TAB>new`, with \\n for newlines. A mutant whose
`old` does not appear exactly once is reported as UNANCHORED rather than
silently skipped - a stale anchor is how a sweep starts testing nothing.

Anything that survives the fast pass is retried at the full iteration count
before being called a survivor, so a short run cannot invent one.
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


def load(path):
    out = []
    for line in open(path):
        if not line.strip() or line.startswith("#"):
            continue
        name, old, new = line.rstrip("\n").split("\t")
        out.append((name, old.replace("\\n", "\n"), new.replace("\\n", "\n")))
    return out


def mirror(rel):
    """A scratch tree that behaves like the repo, with one real file in it.

    The mutated copy has to sit where its own relative includes still resolve
    (`#include "../types.h"` and the like), so everything else is symlinked in
    and only the directories leading to the target are made real.
    """
    tmp = tempfile.mkdtemp(prefix="sweep.")
    parts = rel.split(os.sep)
    here, there = ROOT, tmp
    for d in parts[:-1]:
        for e in os.listdir(here):
            if e != d:
                os.symlink(os.path.join(here, e), os.path.join(there, e))
        here, there = os.path.join(here, d), os.path.join(there, d)
        os.mkdir(there)
    for e in os.listdir(here):
        if e != parts[-1]:
            os.symlink(os.path.join(here, e), os.path.join(there, e))
    return tmp, os.path.join(tmp, rel)


def run_one(src_text, mut, tmp, build, iters, rel):
    name, old, new = mut
    n = src_text.count(old)
    if n != 1:
        return "UNANCHORED (%d matches)" % n
    scratch, src = mirror(rel)
    with open(src, "w") as f:
        f.write(src_text.replace(old, new))
    binary = os.path.join(tmp, "m")
    try:
        cmd = build.format(root=ROOT, src=src, bin=binary)
        r = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        if r.returncode != 0:
            return "BUILD FAILED: " + (r.stderr.strip().splitlines() or [""])[-1][:60]
        env = dict(os.environ, DT_ITER=str(iters))
        r = subprocess.run([binary], capture_output=True, text=True, env=env)
        if r.returncode != 0 and "FAIL" not in r.stdout:
            return "CRASHED"
        return "FAIL" if "FAIL" in r.stdout else "PASS"
    finally:
        shutil.rmtree(scratch, ignore_errors=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--source", required=True, help="repo-relative file to mutate")
    ap.add_argument("--mutants", required=True)
    ap.add_argument("--build", required=True,
                    help="shell command; {root} {src} {bin} are substituted")
    ap.add_argument("--fast", type=int, default=2000)
    ap.add_argument("--full", type=int, default=20000)
    ap.add_argument("--only", nargs="*", default=None,
                    help="name prefixes to run; default all")
    a = ap.parse_args()

    src_text = open(os.path.join(ROOT, a.source)).read()
    muts = load(a.mutants)
    if a.only:
        muts = [m for m in muts if any(m[0].startswith(p) for p in a.only)]

    tmp = tempfile.mkdtemp(prefix="sweep.")
    survivors, broken = [], []
    try:
        for mut in muts:
            v = run_one(src_text, mut, tmp, a.build, a.fast, a.source)
            if v == "PASS":
                # Retry at the full count before believing it survived.
                v = run_one(src_text, mut, tmp, a.build, a.full, a.source)
                if v == "PASS":
                    survivors.append(mut[0])
            if v.startswith("UNANCHORED") or v.startswith("BUILD FAILED"):
                broken.append((mut[0], v))
            print("  %-26s %s" % (mut[0], v))
            sys.stdout.flush()
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

    print("\n%d/%d killed" % (len(muts) - len(survivors) - len(broken), len(muts)))
    for n, v in broken:
        print("  BROKEN   %s: %s" % (n, v))
    for n in survivors:
        print("  SURVIVED %s" % n)
    return 1 if survivors or broken else 0


sys.exit(main())
