#!/bin/bash
# z64.sh - compare the 64-bit build against the 32-bit one on the same ROM.
#
# The difftests cannot help here: they build -m32, where zreg and u4 are the
# same type, so every 64-bit width bug is invisible to them. The only real
# check is running both binaries and comparing what they draw, indexed by
# emulated frame the way test/harness/zab.sh does.
#
#   -r ROM   rom path (required)
#   -t SECS  seconds to run (default 20)
#   -b       reuse the binaries already built (skip the two builds)
#
# One run at a time: both runs write to test/harness/out64/b32 and b64, so two
# of these in parallel overwrite each other's frames and report nonsense.
set -u
ROM=; SECS=20; REUSE=
while getopts "r:t:b" o; do case $o in
  r) ROM=$OPTARG;; t) SECS=$OPTARG;; b) REUSE=1;;
esac; done
[ -n "$ROM" ] || { echo "usage: z64.sh -r ROM [-t SECS] [-b]" >&2; exit 2; }

ROOT=$(git rev-parse --show-toplevel) || exit 2
OUT=$ROOT/test/harness/out64; mkdir -p "$OUT"

if [ -z "$REUSE" ]; then
    echo "=== building 32-bit ==="
    make -C "$ROOT" clean >/dev/null 2>&1
    make -C "$ROOT" WITH_DEBUG_HOOKS=1 linux32 >"$OUT/build32.log" 2>&1 || { echo "32-bit BUILD FAILED"; exit 1; }
    cp "$ROOT/zsnes" "$OUT/zsnes32"
    echo "=== building 64-bit ==="
    make -C "$ROOT" clean >/dev/null 2>&1
    make -C "$ROOT" WITH_DEBUG_HOOKS=1 linux64 >"$OUT/build64.log" 2>&1 || { echo "64-bit BUILD FAILED"; exit 1; }
    cp "$ROOT/zsnes" "$OUT/zsnes64"
fi

H=$ROOT/test/harness/zrun.sh
rm -rf "$OUT/b32" "$OUT/b64"
"$H" -b "$OUT/zsnes32" -r "$ROM" -t "$SECS" -p 30 -o "$OUT/b32" >/dev/null 2>&1
rc32=$?
"$H" -b "$OUT/zsnes64" -r "$ROM" -t "$SECS" -p 30 -o "$OUT/b64" >/dev/null 2>&1
rc64=$?

n32=$(ls "$OUT/b32/png" 2>/dev/null | wc -l)
n64=$(ls "$OUT/b64/png" 2>/dev/null | wc -l)
echo "32-bit: $n32 frames (rc=$rc32)   64-bit: $n64 frames (rc=$rc64)"
if [ "$n64" = 0 ]; then
    echo "RESULT: the 64-bit build drew nothing (32-bit $n32 rc=$rc32, 64-bit rc=$rc64)"
    exit 1
fi

same=0; diff=0; first=
for f in $(ls "$OUT/b32/png"); do
    [ -f "$OUT/b64/png/$f" ] || continue
    if cmp -s "$OUT/b32/png/$f" "$OUT/b64/png/$f"; then
        same=$((same+1))
    else
        diff=$((diff+1)); [ -z "$first" ] && first=$f
    fi
done
if [ "$diff" = 0 ]; then
    echo "RESULT: SAME - $same/$n32 frames identical (32-bit $n32 rc=$rc32, 64-bit $n64 rc=$rc64)"
else
    echo "RESULT: DIFFER - $diff of $((same+diff)), first $first (32-bit $n32, 64-bit $n64)"
fi
