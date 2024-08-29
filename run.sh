cmake -G Ninja -S . -B build/
cmake --build build/

# Only run tests if build is successful
if [ $? -eq 0 ]; then
    ./build/gv "$1"
fi
