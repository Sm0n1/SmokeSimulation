cmake_minimum_required(VERSION 3.20)

project(SmokeSimulation LANGUAGES C)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # we want static linking for simplicity

add_subdirectory(external/SDL)

add_executable(smoke_simulation)

target_sources(smoke_simulation PRIVATE
    src/main.c
  "src/simulation.c")

target_link_libraries(smoke_simulation PRIVATE
	SDL3::SDL3
)

target_compile_options(smoke_simulation PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
    $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
)