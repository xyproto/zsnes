#!/bin/bash
# savecycle.sh BIN ROM [OLDSTATE...] -- the journey a player takes: play, save,
# quit, come back, load. Passes if the machine after loading is the machine
# that was saved, and if every OLDSTATE named still loads.
#
# BIN needs WITH_DEBUG_HOOKS=1: ZSNES_HOTKEY presses the state keys and
# ZSNES_STATE_HASH prints the machine at a frame and exits.
set -u
BIN=${1:?debug-hooks build of zsnes}
ROM=${2:?a rom}
shift 2
SAVE_AT=240
HASH_AT=260
LOAD_AT=30
LOAD_HASH_AT=$((LOAD_AT + HASH_AT - SAVE_AT))
fail=0
home=$(mktemp -d)
trap 'rm -rf "$home"' EXIT

run() { # HOME extra-env...
  local h=$1; shift
  env HOME="$h" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$@" \
    timeout -k 2 120 "$BIN" -v 0 -m -ds "$ROM" 2>&1
}
hash_of() { grep -o 'hash=[0-9a-f]*' | head -1; }

saved=$(run "$home" ZSNES_HOTKEY="save:$SAVE_AT" ZSNES_STATE_HASH=$HASH_AT | hash_of)
state=$(find "$home" -name '*.zst' | head -1)
if [ -z "$state" ]; then
  echo "FAIL: pressing the save key wrote no state"
  exit 1
fi
echo "saved $(stat -c%s "$state") bytes, $saved"

loaded=$(run "$home" ZSNES_HOTKEY="load:$LOAD_AT" ZSNES_STATE_HASH=$LOAD_HASH_AT | hash_of)
cold=$(run "$(mktemp -d)" ZSNES_STATE_HASH=$LOAD_HASH_AT | hash_of)
echo "loaded $loaded"
if [ "$saved" != "$loaded" ]; then
  echo "FAIL: the state that came back is not the one that went in"; fail=1
elif [ "$saved" = "$cold" ]; then
  echo "FAIL: loading changed nothing - the same as never loading"; fail=1
else
  echo "PASS: save and load return the same machine"
fi

# States written by an older ZSNES, if any were named.
for old in "$@"; do
  h=$(mktemp -d); mkdir -p "$h/.config/zsnes"
  cp "$old" "$h/.config/zsnes/$(basename "${ROM%.*}").zst"
  got=$(run "$h" ZSNES_HOTKEY="load:$LOAD_AT" ZSNES_STATE_HASH=$LOAD_HASH_AT | hash_of)
  rm -rf "$h"
  if [ -n "$got" ] && [ "$got" != "$cold" ]; then
    echo "PASS: $(basename "$old") loads"
  else
    echo "FAIL: $(basename "$old") did not load"; fail=1
  fi
done
exit $fail
