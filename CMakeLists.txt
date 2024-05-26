cmake_minimum_required(VERSION 3.22.0)
project(gapvector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-g)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.6.0
)

FetchContent_MakeAvailable(Catch2)
include(CTest)
include(Catch)

add_executable(gv test_gapvector.cpp)

target_link_libraries(gv PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(gv)
