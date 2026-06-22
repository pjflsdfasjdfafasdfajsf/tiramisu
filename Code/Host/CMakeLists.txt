cmake_minimum_required(VERSION 3.16)
project(App C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        ca67f19c8f161cfff26ee52c57ff2ae8b9d42ef6
)

FetchContent_Declare(
    WAMR
    GIT_REPOSITORY https://github.com/bytecodealliance/wasm-micro-runtime.git
    GIT_TAG        26756a5c5846c262c9e5c89f3fc8e1e693ee2539
)

FetchContent_Declare(
    STB
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        31c1ad37456438565541f4919958214b6e762fb4
)

set(WAMR_BUILD_FAST_JIT 1 CACHE BOOL "Enable Fast JIT" FORCE)
set(WAMR_BUILD_INTERP 0 CACHE BOOL "Disable Interpreter" FORCE)
set(WAMR_BUILD_LIBC_BUILTIN 0 CACHE BOOL "Disable LibC" FORCE)
set(WAMR_BUILD_SIMD 0 CACHE BOOL "Disable SIMD" FORCE)

FetchContent_MakeAvailable(SDL3 WAMR STB)


###############################################
# Watch
###############################################

add_custom_target(Copy
    COMMAND ${CMAKE_COMMAND} --build "${GAME_BINARY_DIR}" --target Game
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GAME_BINARY_DIR}/Game.wasm"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Game.wasm"
)

###

add_executable(Watcher
    Watcher.c
)

target_include_directories(Watcher PRIVATE ../Public)

add_custom_target(Watch
    COMMAND $<TARGET_FILE:Watcher>
            "${CMAKE_CURRENT_SOURCE_DIR}/../Game"
            "${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target Copy"
    DEPENDS Watcher
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    USES_TERMINAL 
)

###############################################
# Tools
###############################################

add_executable(AtlasPack
    AtlasPack.c
    STB.c 
)

target_include_directories(AtlasPack PRIVATE
    ${stb_SOURCE_DIR}
    ../Public
)
target_compile_options(AtlasPack PRIVATE
    -O3
)

target_link_libraries(AtlasPack PRIVATE m)

file(GLOB ASSET_IMAGES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../../Assets/Images/*.png")
set(ATLAS_OUTPUT_PREFIX "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/GameAtlas")

if(ASSET_IMAGES)
    add_custom_command(
        OUTPUT "${ATLAS_OUTPUT_PREFIX}.png" "${ATLAS_OUTPUT_PREFIX}.txt"
        COMMAND $<TARGET_FILE:AtlasPack> "${ATLAS_OUTPUT_PREFIX}" ${ASSET_IMAGES}
        DEPENDS AtlasPack ${ASSET_IMAGES}
        VERBATIM
    )
    add_custom_target(GenerateAtlas DEPENDS "${ATLAS_OUTPUT_PREFIX}.png" "${ATLAS_OUTPUT_PREFIX}.txt")
else()
    add_custom_target(GenerateAtlas
        COMMAND ${CMAKE_COMMAND} -E echo "Atlas generation will be skipped"
    )
endif()

###############################################
# Executable
###############################################

add_executable(App
    Main.c
    SDL.c
    STB.c
    Runtime.c
)

target_include_directories(App PRIVATE
    ${wamr_SOURCE_DIR}/core/iwasm/include
    ${stb_SOURCE_DIR}
    ../Public
)

target_compile_options(App PRIVATE
    -Wall -Wextra
    -Wno-attributes
    -Wno-unused-parameter
    
    $<$<CONFIG:Debug>:-ggdb3>
    $<$<CONFIG:Debug>:-Og>
)

target_link_options(App PRIVATE
    $<$<CONFIG:Release>:-s>
)

target_link_libraries(App PRIVATE SDL3::SDL3 vmlib m)

add_dependencies(App GenerateAtlas)
