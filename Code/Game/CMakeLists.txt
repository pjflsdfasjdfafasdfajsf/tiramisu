cmake_minimum_required(VERSION 3.16)
project(Game C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###############################################
# Game.wasm
###############################################

add_executable(Game Game.c ../../Public/Init.c)

set_target_properties(Game PROPERTIES SUFFIX ".wasm")
target_compile_options(Game PRIVATE -nostdlib)
target_include_directories(Game PRIVATE ../../Public)
target_link_options(Game PRIVATE
    -nostdlib
    -fuse-ld=lld
    -Wl,--no-entry
    -Wl,--export-dynamic
    -Wl,--allow-undefined
)
