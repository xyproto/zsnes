#!/bin/sh -e
#
# Self-modifying script that updates the version numbers
#

# The current version goes here, as the default value
VERSION=${1:-'2.3.0'}

if [ -z "$1" ]; then
  echo "The current version is $VERSION, pass the new version as the first argument if you wish to change it"
  exit 0
fi

case $VERSION in
  [0-9]*.[0-9]*.[0-9]*) ;;
  *) echo "Not a MAJOR.MINOR.PATCH version: $VERSION" >&2; exit 1 ;;
esac

echo "Setting the version to $VERSION"

# GNU sed wants -i with no argument and BSD sed wants -i '', so neither spelling
# is portable; write through a temporary file instead.
edit() {
  script=$1
  shift
  for file do
    sed "$script" "$file" > "$file.tmp$$"
    mv "$file.tmp$$" "$file"
  done
}

VER_RE='[[:digit:]][[:digit:]]*\.[[:digit:]][[:digit:]]*\.[[:digit:]][[:digit:]]*'

edit "s/Version: $VER_RE/Version: $VERSION/g" README.md
edit "s/$VER_RE/$VERSION/g" ver.h man/zsnes.1 "$0"

# AppStream: software centres show this list, so add an entry rather than
# rewriting one. Until the tag is pushed it is the development version.
metainfo=linux/io.github.xyproto.zsnes.metainfo.xml
if ! grep -q "release version=\"$VERSION\"" "$metainfo"; then
  edit "s|<releases>|<releases>\\
    <release version=\"$VERSION\" date=\"$(date +%Y-%m-%d)\" type=\"development\"/>|" "$metainfo"
fi
