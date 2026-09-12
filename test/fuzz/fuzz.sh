#!/bin/bash
# fuzz.sh BIN ROM KIND ITERATIONS [OUTDIR]
#
# Feed the loader for KIND mutated files, one process each, and keep every
# input that made the sanitizer speak or the process die. BIN should be an
# AddressSanitizer build with debug hooks: ZSNES_STATE_HASH makes each run
# exit on its own after a few frames.
#
#   KIND   zst  save state auto-loaded with -zs 0 (seed: SEED_ZST, or one is made)
#          zmv  movie auto-played with -zm 0
#          cht  cheat file beside the SRAM
#          cmb  combo file beside the SRAM
#          ips  patch applied to the ROM on load
#          zip  the ROM itself, zipped
#          cfg  zsnesl.cfg
set -u
BIN=$1; ROM=$2; KIND=$3; N=$4; OUT=${5:-fuzz-out}
here=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$OUT"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
python3 "$here/seeds.py" "$work" "$ROM"

base=$(basename "$ROM"); stem=${base%.*}
found=0
for ((i = 1; i <= N; i++)); do
  H="$work/home"; rm -rf "$H"; mkdir -p "$H/.config/zsnes" "$H/rom"
  romuse="$H/rom/$base"
  cp "$ROM" "$romuse"
  args=(-m -ds)
  case $KIND in
    zst) seed=${SEED_ZST:?set SEED_ZST to a save state for this ROM}
         python3 "$here/mutate.py" "$seed" "$H/.config/zsnes/$stem.zst" "$i"; args+=(-zs 0);;
    zmv) python3 "$here/mutate.py" "$work/seed.zmv" "$H/.config/zsnes/$stem.zmv" "$i"; args+=(-zm 0);;
    cht) python3 "$here/mutate.py" "$work/seed.cht" "$H/.config/zsnes/$stem.cht" "$i";;
    cmb) python3 "$here/mutate.py" "$work/seed.cmb" "$H/.config/zsnes/$stem.cmb" "$i";;
    ips) python3 "$here/mutate.py" "$work/seed.ips" "$H/rom/$stem.ips" "$i";;
    zip) src=$([ $((i % 2)) = 0 ] && echo "$work/seed.zip" || echo "$work/stored.zip")
         python3 "$here/mutate.py" "$src" "$H/rom/game.zip" "$i"; romuse="$H/rom/game.zip";;
    cfg) [ -f "$work/seed.cfg" ] || { timeout -k 5 20 xvfb-run -a env HOME="$H" SDL_AUDIODRIVER=dummy "$BIN" -v 0 >/dev/null 2>&1; cp "$H/.config/zsnes/zsnesl.cfg" "$work/seed.cfg"; }
         python3 "$here/mutate.py" "$work/seed.cfg" "$H/.config/zsnes/zsnesl.cfg" "$i";;
    *) echo "unknown kind $KIND" >&2; exit 2;;
  esac
  log="$work/run.log"
  timeout -k 5 60 xvfb-run -a env HOME="$H" SDL_AUDIODRIVER=dummy ASAN_OPTIONS=detect_leaks=0:abort_on_error=0 \
    ZSNES_STATE_HASH=30 "$BIN" "${args[@]}" "$romuse" >"$log" 2>&1
  rc=$?
  if grep -q 'ERROR: AddressSanitizer\|runtime error:' "$log" || { [ $rc -ge 128 ] && [ $rc -ne 137 ] && [ $rc -ne 124 ]; }; then
    found=$((found + 1))
    keep="$OUT/$KIND-$i"
    case $KIND in
      zst|zmv|cht|cmb) cp "$H/.config/zsnes/$stem.$KIND" "$keep.$KIND";;
      ips) cp "$H/rom/$stem.ips" "$keep.ips";;
      zip) cp "$H/rom/game.zip" "$keep.zip";;
      cfg) cp "$H/.config/zsnes/zsnesl.cfg" "$keep.cfg";;
    esac
    cp "$log" "$keep.log"
    printf '%s #%d: rc=%d %s\n' "$KIND" "$i" "$rc" "$(grep -m1 -oE 'SUMMARY: AddressSanitizer: [a-z-]+ [^ ]+ in [A-Za-z0-9_]+' "$log" | sed 's/SUMMARY: AddressSanitizer: //')"
  fi
done
echo "$KIND: $N runs, $found kept in $OUT"
[ $found = 0 ]
