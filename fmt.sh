#!/bin/sh
# Format the project's C sources in place.
set -e
cd "$(dirname "$0")"
git ls-files -z '*.c' '*.h' '*.cpp' | xargs -0 clang-format -style=file -i
