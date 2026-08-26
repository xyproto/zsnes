#!/bin/bash
# zrev.sh REV -- builds REV in a scratch worktree and runs the SMRPG hang check.
set -u
REV=$1; shift
ROOT=$(git rev-parse --show-toplevel)
# Outside the checkout; see the note in zab.sh.
WTBASE=${TMPDIR:-/tmp}/zsnes-harness-$(id -u)
WT=$WTBASE/rev
mkdir -p "$WTBASE"
git -C "$ROOT" worktree prune
git -C "$ROOT" worktree remove --force "$WT" >/dev/null 2>&1
git -C "$ROOT" worktree add --detach "$WT" "$REV" >/dev/null 2>&1 || { echo "$REV: WORKTREE-FAIL"; exit 2; }
if ! make -C "$WT" WITH_DEBUG_HOOKS=1 -j"$(nproc)" >/tmp/zrev_build_$$.log 2>&1; then
    echo "$REV: BUILD-FAIL"; tail -3 /tmp/zrev_build_$$.log; exit 2
fi
printf '%s: ' "$REV"
"$ROOT/test/harness/zhang.sh" -b "$WT/zsnes" "$@"
