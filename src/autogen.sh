#!/bin/sh

echo "Generating build information using aclocal and autoconf..."

# Regenerate configuration files
aclocal
autoconf

# Run configure for this platform
./configure $*
