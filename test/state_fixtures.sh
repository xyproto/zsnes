#!/bin/bash
# state_fixtures.sh BIN -- load the committed save-state fixtures with BIN and
# check each decodes to the machine it decoded to when it was recorded.
#
# This guards the formats users depend on across releases:
#   *.v2.zst    this build's own format (2.4.0+, self-describing envelope)
#   *.v144.zst  the 2.3.x format this build still reads
#   *.wide.zst  the 64-bit 2.3.0/2.3.1 releases (pointer-width fields)
#   *.v143.zst  1.51 (add one when a genuine 1.51 save is available; the check
#               picks it up automatically once a golden hash is set for it)
#
# BIN must be a WITH_DEBUG_HOOKS build: ZST_LOADONLY loads the fixture and
# reports which size class it matched, and ZSNES_STATE_HASH prints the machine
# one frame later and exits. The fixtures were made from CPUMSC.sfc (krom's CPU
# test), fetched here by pinned checksum into test/roms/ (gitignored) exactly
# as test/asan_soak.sh does.
set -u
BIN=${1:?a WITH_DEBUG_HOOKS build of zsnes}
here=$(cd "$(dirname "$0")" && pwd)
roms=$here/roms
fixtures=$here/fixtures
rom=$roms/CPUMSC.sfc
rom_sha=00fa9244fe5a086042c4ca8e9f425892dbf115b135d7a14630c88dee3fd18490
rom_url=https://raw.githubusercontent.com/PeterLemon/SNES/master/CPUTest/CPU/MSC/CPUMSC.sfc

# fixture           expected fits   post-load hash (frame 61)
expect() {
    case $1 in
    cpumsc.v2) echo "V2 3f6a387ddd48a187" ;;
    cpumsc.v144) echo "V144 32b57e5eb5e22d14" ;;
    cpumsc.wide) echo "V144-WIDE c955754bdcc22e6e" ;;
    *) echo "" ;;
    esac
}

mkdir -p "$roms"
if [ ! -f "$rom" ] || ! echo "$rom_sha  $rom" | sha256sum -c --quiet 2>/dev/null; then
    curl -sfL "$rom_url" -o "$rom" || { echo "state_fixtures: could not fetch CPUMSC.sfc; skipping"; exit 0; }
    echo "$rom_sha  $rom" | sha256sum -c --quiet || { echo "state_fixtures: bad CPUMSC.sfc checksum"; exit 1; }
fi

fail=0
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
for f in "$fixtures"/*.zst; do
    [ -e "$f" ] || { echo "state_fixtures: no fixtures found"; exit 1; }
    name=$(basename "$f" .zst)
    want=$(expect "$name")
    if [ -z "$want" ]; then
        echo "$name: no golden recorded - add one to expect() to guard it"
        continue
    fi
    want_fits=${want% *}; want_hash=${want#* }

    home=$tmp/home; rm -rf "$home"; mkdir -p "$home"
    cp "$f" "$tmp/zsnes_zst_a.zst"
    out=$(env HOME="$home" TMPDIR="$tmp" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        ASAN_OPTIONS=detect_leaks=0 ZST_ROUNDTRIP=60 ZST_LOADONLY=1 ZSNES_STATE_HASH=61 \
        timeout -k 5 60 "$BIN" -v 0 -m -ds "$rom" 2>&1)

    fits=$(printf '%s\n' "$out" | sed -n 's/.*fits=\([A-Za-z0-9-]*\).*/\1/p' | head -1)
    got_hash=$(printf '%s\n' "$out" | sed -n 's/.*STATEHASH.*hash=\([0-9a-f]*\).*/\1/p' | head -1)
    accepted=$(printf '%s\n' "$out" | grep -c 'LOADONLY ACCEPTED')
    san=$(printf '%s\n' "$out" | grep -c 'ERROR: AddressSanitizer\|runtime error:')

    if [ "$accepted" -ge 1 ] && [ "$fits" = "$want_fits" ] && [ "$got_hash" = "$want_hash" ] && [ "$san" -eq 0 ]; then
        printf '%-14s PASS (fits=%s hash=%s)\n' "$name" "$fits" "$got_hash"
    else
        printf '%-14s FAIL (fits=%s want %s; hash=%s want %s; accepted=%s asan=%s)\n' \
            "$name" "${fits:-none}" "$want_fits" "${got_hash:-none}" "$want_hash" "$accepted" "$san"
        fail=1
    fi
done
exit $fail
