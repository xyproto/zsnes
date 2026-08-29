#!/bin/bash
# Serves the original assembly out of asm-sources.zip so the difftests do not
# need git history; a shallow clone has none. Understands only the few forms
# the mk*.sh scripts use.
set -u
here=$(cd "$(dirname "$0")" && pwd)
zip=$here/asm-sources.zip
cache=$here/_asmsrc

if [ ! -f "$cache/index.tsv" ]; then
    mkdir -p "$cache"
    if command -v unzip >/dev/null 2>&1; then
        (cd "$cache" && unzip -qo "$zip")
    else
        (cd "$cache" && python3 -c 'import sys,zipfile;zipfile.ZipFile(sys.argv[1]).extractall()' "$zip")
    fi
fi
idx=$cache/index.tsv

blobkey() { printf '%s' "$1:$2" | tr '/:^' '___'; }

# drop "-C <dir>": the archive is the repository here
[ "${1:-}" = "-C" ] && shift 2

cmd=${1:-}; shift || true
case "$cmd" in
show)
    spec=$1; rev=${spec%%:*}; path=${spec#*:}
    f=$cache/blobs/$(blobkey "$rev" "$path")
    [ -f "$f" ] || { echo "asmgit: no $spec in $(basename "$zip")" >&2; exit 128; }
    cat "$f" ;;
cat-file)
    [ "$1" = "-e" ] && shift
    spec=$1; rev=${spec%%:*}; path=${spec#*:}
    [ -f "$cache/blobs/$(blobkey "$rev" "$path")" ] ;;
log|rev-list)
    for a in "$@"; do path=$a; done
    n=0
    [ "$cmd" = "rev-list" ] && n=1
    # a recorded rev may carry the caret the caller will add itself
    awk -F'\t' -v p="$path" -v n="$n" \
        '$2==p{r=$1; sub(/\^+$/,"",r); print r; if(n && ++c>=n) exit}' "$idx" ;;
rev-parse)
    [ "$1" = "--short" ] && shift
    printf '%s\n' "${1:0:8}" ;;
*)
    echo "asmgit: unsupported: $cmd $*" >&2; exit 128 ;;
esac
