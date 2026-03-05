cmake_minimum_required(VERSION 3.20)

project(SmokeSimulation LANGUAGES C)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # we want static linking for simplicity

add_subdirectory(external/SDL)

add_executable(smoke_simulation)

target_sources(smoke_simulation PRIVATE
    src/main.c
)

target_link_libraries(smoke_simulation PRIVATE
	SDL3::SDL3
)