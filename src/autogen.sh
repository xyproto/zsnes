#!/bin/sh

echo "Generating build information using aclocal and autoconf..."

# Touch the timestamps on all the files since CVS messes them up
directory=`dirname $0`
touch $directory/configure.in

# Regenerate configuration files
aclocal
autoconf

# Run configure for this platform
#./configure $*
echo "Now you are ready to run ./configure"

