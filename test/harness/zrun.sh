#!/bin/bash
# zrun.sh - run zsnes headlessly under the built-in debug instrumentation and
# collect every artifact into one directory.
#
# The emulator's debug hooks (linux/sdllink.c, c_init.c) hardcode /tmp paths,
# so this wrapper clears them before the run and harvests them after; that also
# keeps parallel runs from reading each other's leftovers.
#
#   -b BIN     zsnes binary                  (default ./zsnes)
#   -r ROM     rom path                      (required)
#   -i SCRIPT  DEBUG_INPUT_SCRIPT            (e.g. "none,3000,start,200,none,2000")
#   -s SLOT    auto-load savestate slot      (-zs)
#   -p N       PNG every N frames            (default 30; 0 disables)
#   -t SECS    wall-clock cap                (default 30)
#   -o OUTDIR  artifact directory            (required)
#   ASCII=1    env: also write ASCII dumps + per-frame hashes (slow; off by default)
set -u
BIN=./zsnes; ROM=; INPUT=; SLOT=; PNGEVERY=30; SECS=30; OUT=; ASCII=${ASCII:-}
while getopts "b:r:i:s:p:t:o:" o; do case $o in
  b) BIN=$OPTARG;; r) ROM=$OPTARG;; i) INPUT=$OPTARG;; s) SLOT=$OPTARG;;
  p) PNGEVERY=$OPTARG;; t) SECS=$OPTARG;; o) OUT=$OPTARG;;
esac; done
[ -n "$ROM" ] && [ -n "$OUT" ] || { echo "usage: zrun.sh -r ROM -o OUTDIR [-b BIN] [-i SCRIPT] [-s SLOT] [-p N] [-t SECS]" >&2; exit 2; }
[ -x "$BIN" ] || { echo "no such binary: $BIN" >&2; exit 2; }
[ -f "$ROM" ] || { echo "no such rom: $ROM" >&2; exit 2; }

# HOME below is derived from $OUT, and a relative HOME makes the emulator
# segfault during start-up before it prints anything - so pin it absolute.
rm -rf "$OUT"; mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd) || exit 2
rm -f /tmp/zsnes_*.png /tmp/zsnes_*.txt /tmp/zsnes_ppu.txt /tmp/zsnes_hashes.txt

args=(-v 0 -m -ds)
[ -n "$SLOT" ] && args+=(-zs "$SLOT")

# The emulator writes its config back on exit, so a run against the real $HOME
# would make -ds and -v 0 permanent. Give each run a throwaway HOME.
RUNHOME=$OUT/home
mkdir -p "$RUNHOME"

# Run on a throwaway X server so the emulator window does not pop up over
# whatever the user is doing. Without Xvfb, fall back to SDL's dummy video
# driver - on a headless box the real display is not there at all, and SDL_Init
# fails before a single frame is logged.
if command -v xvfb-run >/dev/null 2>&1; then
  XVFB=(xvfb-run -a -s "-screen 0 640x480x24")
else
  XVFB=()
  export SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-dummy}
  export SDL_AUDIODRIVER=${SDL_AUDIODRIVER:-dummy}
fi

env HOME="$RUNHOME" PPU_STATE_LOG=1 \
    ${ASCII:+ASCII_SCREENSHOT_EVERY_FIVE=1 ASCII_SCREENSHOT_BURST=3} \
    ${PNGEVERY:+PNG_SCREENSHOT_EVERY_N=$PNGEVERY} \
    ${INPUT:+DEBUG_INPUT_SCRIPT=$INPUT} \
    "${XVFB[@]}" timeout -k 5 "$SECS" "$BIN" "${args[@]}" "$ROM" </dev/null >"$OUT/stdout.log" 2>&1
# -k: zsnes installs a SIGTERM handler and does not always act on it, so the
# cap needs a SIGKILL behind it or a run can sit there forever.
echo "exit=$? (124 = hit the time cap, which is the normal way a run ends)" | tee "$OUT/result.txt"

for f in /tmp/zsnes_ppu.txt /tmp/zsnes_hashes.txt; do
    [ -f "$f" ] && cp -f "$f" "$OUT/" 2>/dev/null
done
mkdir -p "$OUT/png" "$OUT/ascii"
for f in /tmp/zsnes_*.png; do [ -e "$f" ] && cp -f "$f" "$OUT/png/"; done 2>/dev/null
for f in /tmp/zsnes_[0-9]*.txt; do [ -e "$f" ] && cp -f "$f" "$OUT/ascii/"; done 2>/dev/null
rmdir "$OUT/png" "$OUT/ascii" 2>/dev/null

echo "frames_logged=$(wc -l < "$OUT/zsnes_ppu.txt" 2>/dev/null || echo 0)" | tee -a "$OUT/result.txt"
echo "pngs=$(ls "$OUT/png" 2>/dev/null | wc -l)" | tee -a "$OUT/result.txt"
echo "artifacts in $OUT"
