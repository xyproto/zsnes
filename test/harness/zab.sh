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
WT=$ROOT/.claude/worktrees/_ab_baseline
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
"$H" -b "$WT/zsnes"   "${common[@]}" -o "$OUT/base" >/dev/null
"$H" -b "$ROOT/zsnes" "${common[@]}" -o "$OUT/cand" >/dev/null

# A baseline that never booted produces one repeated frame; that is a broken
# measurement, not a passing comparison.
distinct=$(sort -u "$OUT/base/zsnes_ppu.txt" 2>/dev/null | wc -l)
frames=$(wc -l < "$OUT/base/zsnes_ppu.txt" 2>/dev/null || echo 0)
echo "baseline: $frames frames, $distinct distinct states"
if [ "$distinct" -lt 10 ]; then
    echo "INCONCLUSIVE: baseline produced $distinct distinct states (<10) - it likely never booted. Re-run."
    exit 3
fi

for f in zsnes_ppu.txt zsnes_hashes.txt; do
    if [ -f "$OUT/base/$f" ] && [ -f "$OUT/cand/$f" ]; then
        if diff -q "$OUT/base/$f" "$OUT/cand/$f" >/dev/null; then
            echo "SAME: $f identical ($(wc -l < "$OUT/base/$f") lines)"
        else
            echo "DIFFER: $f - first divergence:"
            diff "$OUT/base/$f" "$OUT/cand/$f" | head -6
        fi
    fi
done
