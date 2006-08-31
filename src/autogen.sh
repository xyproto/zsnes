#!/bin/sh

echo "Generating build information using aclocal and autoconf..."

# Regenerate configuration files
aclocal  --acdir=`sdl-config --prefix`/share/aclocal	# thanks asfand
autoconf

# Run configure for this platform, or simply update Makefile
case $1 in
  --noconf )
    break ;;
  --recheck )
    ./config.status --recheck; break ;;
  * )
    ./configure $*; break ;;
esac
