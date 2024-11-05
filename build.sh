#!/bin/bash

clang++ -std=c++20 -Wall -Wextra catch_amalgamated.cpp test_gapbuffer.cpp -o gv_test

# Only run tests if build is successful
if [ $? -eq 0 ]; then
    ./gv_test "$1"
fi
