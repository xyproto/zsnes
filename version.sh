#!/bin/sh -e
#
# Self-modifying script that updates the version numbers
#

# The current version goes here, as the default value
VERSION=${1:-'2.0.12'}

if [ -z "$1" ]; then
  echo "The current version is $VERSION, pass the new version as the first argument if you wish to change it"
  exit 0
fi

echo "Setting the version to $VERSION"

# Set the version in README.md
sed -i "s/Version: [[:digit:]]*\.[[:digit:]]*\.[[:digit:]]*/Version: $VERSION/g" README.md

# Set the version in various files
sed -i "s/[[:digit:]]*\.[[:digit:]]*\.[[:digit:]]*/$VERSION/g" ver.h

# Set the version in the man page
sed -i "s/[[:digit:]]*\.[[:digit:]]*\.[[:digit:]]*/$VERSION/g" man/zsnes.1

# Set the version in this script
sed -i "s/[[:digit:]]*\.[[:digit:]]*\.[[:digit:]]*/$VERSION/g" "$0"
