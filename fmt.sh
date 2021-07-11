#!/bin/sh
find . -path ./cpu -prune -false -o -name "*.c" -exec clang-format -style=file -i {} \;
find . -path ./cpu -prune -false -o -name "*.h" -exec clang-format -style=file -i {} \;
find . -path ./cpu -prune -false -o -name "*.cpp" -exec clang-format -style=file -i {} \;
