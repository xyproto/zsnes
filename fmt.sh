#!/bin/sh
find . -path ./cpu -prune -false -o -name "*.c" -exec clang-format-16 -style=file -i {} \;
find . -path ./cpu -prune -false -o -name "*.h" -exec clang-format-16 -style=file -i {} \;
