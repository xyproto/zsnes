#!/bin/bash
# zab.sh - deterministic A/B of the working tree against a baseline revision.
#
# Builds BASE_REV in a throwaway worktree, runs both binaries through zrun.sh on
# the identical scenario, then diffs the per-frame PPU log and frame hashes.
#
# Exit-code smoke tests do NOT discriminate here: an unmodified build exits
# early on roughly half of all runs for environmental reasons. Only the
# per-frame state comparison below is a real signal.
#
#   -R REV   baseline revision   (default HEAD)
#   -r ROM   rom path            (required)
#   -i SCRIPT / -s SLOT / -t SECS   passed through to zrun.sh
set -u
REV=HEAD; ROM=; INPUT=; SLOT=; SECS=30
while getopts "R:r:i:s:t:" o; do case $o in
  R) REV=$OPTARG;; r) ROM=$OPTARG;; i) INPUT=$OPTARG;; s) SLOT=$OPTARG;; t) SECS=$OPTARG;;
esac; done
[ -n "$ROM" ] || { echo "usage: zab.sh -r ROM [-R REV] [-i SCRIPT] [-s SLOT] [-t SECS]" >&2; exit 2; }

ROOT=$(git rev-parse --show-toplevel) || exit 2
# Scratch worktrees live outside the checkout: a second build tree inside it
# shows up as untracked files and gets picked up by tools that walk the repo.
# Keyed by user so two people on one machine do not collide.
WTBASE=${TMPDIR:-/tmp}/zsnes-harness-$(id -u)
WT=$WTBASE/ab_baseline
mkdir -p "$WTBASE"
git -C "$ROOT" worktree prune
OUT=$ROOT/test/harness/out; mkdir -p "$OUT"

echo "=== building baseline $REV ==="
git -C "$ROOT" worktree remove --force "$WT" 2>/dev/null
git -C "$ROOT" worktree add --detach "$WT" "$REV" >/dev/null 2>&1 || { echo "worktree add failed"; exit 1; }
if ! make -C "$WT" -j"$(nproc)" >"$OUT/baseline_build.log" 2>&1; then
    echo "BASELINE BUILD FAILED - see $OUT/baseline_build.log"; exit 1
fi
echo "=== building candidate (working tree) ==="
if ! make -C "$ROOT" -j"$(nproc)" >"$OUT/candidate_build.log" 2>&1; then
    echo "CANDIDATE BUILD FAILED - see $OUT/candidate_build.log"; exit 1
fi

H=$ROOT/test/harness/zrun.sh
common=(-r "$ROM" -t "$SECS" -p 0)
[ -n "$INPUT" ] && common+=(-i "$INPUT")
[ -n "$SLOT" ]  && common+=(-s "$SLOT")
# zrun.sh bails before it clears its output directory, so a run that never
# started leaves the previous game's artifacts behind and the diff below would
# compare those and report SAME. Stop instead.
rm -rf "$OUT/base" "$OUT/cand"
"$H" -b "$WT/zsnes"   "${common[@]}" -o "$OUT/base" >/dev/null || exit 2
"$H" -b "$ROOT/zsnes" "${common[@]}" -o "$OUT/cand" >/dev/null || exit 2

# A baseline that never booted produces one repeated frame; that is a broken
# measurement, not a passing comparison.
distinct=$(sort -u "$OUT/base/zsnes_ppu.txt" 2>/dev/null | wc -l)
frames=$(wc -l < "$OUT/base/zsnes_ppu.txt" 2>/dev/null || echo 0)
echo "baseline: $frames frames, $distinct distinct states"
if [ "$distinct" -lt 10 ]; then
    echo "INCONCLUSIVE: baseline produced $distinct distinct states (<10) - it likely never booted. Re-run."
    exit 3
fi

python3 - "$OUT/base" "$OUT/cand" <<'PY'
import sys, os
base, cand = sys.argv[1], sys.argv[2]
rc = 0
for name in ("zsnes_ppu.txt", "zsnes_hashes.txt"):
    b, c = os.path.join(base, name), os.path.join(cand, name)
    if not (os.path.exists(b) and os.path.exists(c)):
        continue
    B = open(b).read().splitlines()
    C = open(c).read().splitlines()
    n = min(len(B), len(C))
    if n == 0:
        print(f"{name}: NO OVERLAP"); rc = 3; continue
    first = next((i for i in range(n) if B[i] != C[i]), None)
    # The runs are wall-clock capped, so differing lengths are expected and are
    # not a divergence; only mismatching content within the overlap counts.
    if first is None:
        print(f"SAME: {name} identical over {n} common frames (base {len(B)}, cand {len(C)})")
    else:
        print(f"DIFFER: {name} diverges at frame {first+1} of {n} common")
        print(f"   base: {B[first]}")
        print(f"   cand: {C[first]}")
        rc = 1
    if n < 500:
        print(f"   WARNING: only {n} common frames - raise -t for real coverage")
sys.exit(rc)
PY
