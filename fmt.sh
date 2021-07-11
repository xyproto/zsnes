#!/bin/sh
find . -name "*.c" -exec clang-format -style=file -i {} \;
find . -name "*.h" -exec clang-format -style=file -i {} \;
find . -name "*.cpp" -exec clang-format -style=file -i {} \;
