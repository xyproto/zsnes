#!/bin/sh

echo "Generating build information using aclocal and autoconf..."

# Regenerate configuration files
aclocal	--acdir=`sdl-config --prefix`/share/aclocal	# thanks asfand
autoconf

# Run configure for this platform
./configure $*
