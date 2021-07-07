#!/bin/sh
find . -regex '.*\.\(\c|\h|cpp\|hpp\|cc\|cxx\)' -exec clang-format -style=file -i {} \;
