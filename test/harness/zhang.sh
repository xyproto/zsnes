#!/bin/bash
# zhang.sh - does this build hang (PPU stuck in force blank) on a scenario?
# Prints OK or HANG plus the frame the hang started. Usable as a git-bisect predicate.
#   -b BIN  binary (default ./zsnes)   -r ROM   -i INPUT_SCRIPT   -t SECS
set -u
BIN=./zsnes; ROM=; INPUT=; SECS=120
while getopts "b:r:i:t:" o; do case $o in
  b) BIN=$OPTARG;; r) ROM=$OPTARG;; i) INPUT=$OPTARG;; t) SECS=$OPTARG;;
esac; done
D=$(mktemp -d)
"$(dirname "$0")/zrun.sh" -b "$BIN" -r "$ROM" ${INPUT:+-i "$INPUT"} -t "$SECS" -p 0 -o "$D" >/dev/null 2>&1
python3 - "$D/zsnes_ppu.txt" <<'PY'
import sys,re
rows=[]
try: lines=open(sys.argv[1])
except OSError: print("NO-DATA"); sys.exit(2)
for l in lines:
    m=re.match(r'(\d+) bright=(\d+) blank=([0-9a-f]+) scrnon=([0-9a-f]+)',l.strip())
    if m: rows.append((int(m[1]),int(m[2]),int(m[3],16),int(m[4],16)))
if len(rows)<2000: print(f"NO-DATA (only {len(rows)} frames)"); sys.exit(2)
black=lambda r: r[1]==0 or (r[2]&0x80) or r[3]==0
# hang = the run ends inside one long unbroken black stretch
i=len(rows)-1
while i>=0 and black(rows[i]): i-=1
tail=len(rows)-1-i
print(f"HANG at frame {rows[i+1][0]} ({tail} trailing black frames of {len(rows)})" if tail>=1500
      else f"OK (longest trailing black {tail}, {len(rows)} frames)")
sys.exit(1 if tail>=1500 else 0)
PY
rc=$?; rm -rf "$D"; exit $rc
