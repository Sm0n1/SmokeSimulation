# Running the program

1. Clone the repository using your favorite method. :>
2. Navigate to the root directory and run `cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release` to create the build folder.
3. Run `cmake --build build` to build the project.
4. On Linux/MacOS, run `./build/release/smoke_simulation`, and on Windows, run `build\release\smoke_simulation.exe`.

# Controls

- `Left-Click`: add density and/or velocity (depending on the current interaction mode) at the current mouse position.
- `R`: reset the simulation
- `D`: toggle density interaction mode (enabled by default)
- `V`: toggle velocity interaction mode (enabled by default)
- `P`: toggle smoke stream (disabled by default)
