#!/bin/bash

function compile() {
    clang++ -std=c++20 -Wall -Wextra catch_amalgamated.cpp test_gapbuffer.cpp -o gv_test

        # Only run tests if build is successful
        if [ $? -eq 0 ]; then
            ./gv_test "$1"
        fi
}

function watch() {
    local file_path="gapbuffer.h"
    local last_mod_time=""

    while true; do
        # Get the current modification time
        current_mod_time=$(stat -c %Y "$file_path" 2>/dev/null)

        if [[ "$current_mod_time" != "$last_mod_time" ]]; then
            if [[ -n "$last_mod_time" ]]; then
                compile
            fi
            last_mod_time="$current_mod_time"
        fi

        sleep 1
    done
}

if [[ "$1" == "--watch" ]]; then
    watch
else
    compile
fi
