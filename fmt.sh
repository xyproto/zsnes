#!/bin/sh
set -e
cd "$(dirname "$0")"
git ls-files '*.c' '*.h' '*.cpp' \
  | grep -v test/difftest_op.c \
  | grep -v zloader.c \
  | grep -v test/ng2_harness.h \
  | xargs clang-format -style=file -i

#git ls-files -z '*.c' '*.h' '*.cpp' | xargs -0 clang-format -style=file -i
