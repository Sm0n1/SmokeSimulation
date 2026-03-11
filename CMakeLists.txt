cmake_minimum_required(VERSION 3.20)

if(APPLE AND CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures" FORCE)
endif()

project(SmokeSimulation LANGUAGES C)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # static linking

add_subdirectory(external/SDL)

add_executable(smoke_simulation)

target_sources(smoke_simulation PRIVATE
    src/main.c
    src/simulation.c
)

target_link_libraries(smoke_simulation PRIVATE
	SDL3::SDL3
)

target_compile_options(smoke_simulation PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<C_COMPILER_ID:GNU,CLANG>:-Wall -Wextra -Werror -Wshadow -Wconversion -Wdouble-promotion -Wformat=2 -Wundef -Wnull-dereference -Wpedantic>
    $<$<CONFIG:Debug>:-fsanitize=address,undefined -fno-omit-frame-pointer>
    $<$<AND:$<CONFIG:Release>,$<C_COMPILER_ID:MSVC>>:/O2 /permissive->
    $<$<AND:$<CONFIG:Release>,$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>>>:-O3 -march=native -ffast-math>
)

target_link_options(smoke_simulation PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)