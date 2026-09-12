#!/bin/bash
# asan_soak.sh BIN [SECONDS] -- run a set of freely downloadable test ROMs
# under an AddressSanitizer build with every filter, CRT setting and video mode
# cycling (ZSNES_FILTER_SOAK), then a short pass of the file fuzzer. This is
# what catches a guest-driven index into a host buffer: the selftest emulates
# nothing, and unit tests do not draw.
#
# The ROMs are krom's SNES demos, fetched by pinned checksum into
# test/roms/ (gitignored). Each covers something the others do not: mode 7,
# interlace, HDMA colour, pseudo-hires, the SPC700, and the CPU test.
set -u
BIN=${1:?asan build of zsnes}; SECS=${2:-30}
here=$(cd "$(dirname "$0")" && pwd)
roms=$here/roms; mkdir -p "$roms"
base=https://raw.githubusercontent.com/PeterLemon/SNES/master
want='
d5580870e152046ca838fa276568e10c671be63b1db0285030a53dc7864cc4c0 PPU/Mode7/RotZoom/RotZoom.sfc
39489ace933125e2aa9e71e6ea90420b96cbb5e64bc07e4b5abaa06a0c3bf133 PPU/Interlace/InterlaceRPG/InterlaceRPG.sfc
01dace4535e9352aba93ea688e8d1c4f769482823b0dca0fda6606cbf3e51729 PPU/HDMA/HiColor64PerTileRowPseudoHiRes/HiColor64PerTileRowPseudoHiRes.sfc
e60a09ec6e5d57c1adc22b2f84fe24ca9259b60e0d3f7d986bd809151980543c PPU/HDMA/Mode7HDMA/Mode7HDMA.sfc
96fdf1cf0ad6cd06cbc2d20127b112ffe88b0e7ac22fd351755ed9a59ce1eaf5 SPC700/Axel-F/Axel-F.sfc
00fa9244fe5a086042c4ca8e9f425892dbf115b135d7a14630c88dee3fd18490 CPUTest/CPU/MSC/CPUMSC.sfc
'
fail=0
while read -r sum path; do
  [ -n "$sum" ] || continue
  f=$roms/$(basename "$path")
  if [ ! -f "$f" ] || ! echo "$sum  $f" | sha256sum -c --quiet 2>/dev/null; then
    curl -sfL "$base/$path" -o "$f" || { echo "could not fetch $path"; fail=1; continue; }
    echo "$sum  $f" | sha256sum -c --quiet || { echo "bad checksum for $path"; rm -f "$f"; fail=1; continue; }
  fi
done <<< "$want"

run() { # ROM MODE
  local rom=$1 mode=$2 H log rc
  H=$(mktemp -d); mkdir -p "$H/.config/zsnes"; log=$H/log
  timeout -k 5 20 xvfb-run -a env HOME="$H" SDL_AUDIODRIVER=dummy "$BIN" -v 0 >/dev/null 2>&1
  sed -i -E "s/^cvidmode=.*/cvidmode=$mode/; s/^hqFilter=.*/hqFilter=1/; s/^hqFilterlevel=.*/hqFilterlevel=4/; s/^sl_intensity=.*/sl_intensity=50/; s/^sl_vibrancy=.*/sl_vibrancy=45/; s/^BloomLevel=.*/BloomLevel=25/; s/^Mode7HiRes16b=.*/Mode7HiRes16b=1/" "$H/.config/zsnes/zsnesl.cfg"
  timeout -k 5 "$SECS" xvfb-run -a -s "-screen 0 1280x960x24" env HOME="$H" SDL_AUDIODRIVER=dummy \
    ASAN_OPTIONS=detect_leaks=0 ZSNES_FILTER_SOAK=20 "$BIN" -m -ds "$rom" >"$log" 2>&1
  if grep -q 'ERROR: AddressSanitizer\|runtime error:' "$log"; then
    printf '%-40s mode %-2s FAIL\n' "$(basename "$rom")" "$mode"; grep -m1 -A6 'ERROR: AddressSanitizer' "$log"; fail=1
  else
    printf '%-40s mode %-2s ok, %s steps\n' "$(basename "$rom")" "$mode" "$(grep -c '^SOAK' "$log")"
  fi
  rm -rf "$H"
}
for rom in "$roms"/*.sfc; do
  run "$rom" 14   # accelerated, fullscreen
  run "$rom" 2    # software
done

first=$(ls "$roms"/*.sfc | head -1)
for kind in ips zip cfg cht cmb zmv; do
  "$here/fuzz/fuzz.sh" "$BIN" "$first" "$kind" 8 "$roms/fuzz-out" | tail -1 || fail=1
done
exit $fail
