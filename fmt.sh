#!/bin/sh
# Format the project's C sources in place.
set -e
cd "$(dirname "$0")"
git ls-files '*.c' '*.h' '*.cpp' | grep -v test/difftest_op.c | xargs clang-format -style=file -i
#git ls-files -z '*.c' '*.h' '*.cpp' | xargs -0 clang-format -style=file -i
