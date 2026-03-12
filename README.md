# Running the program

1. Clone the repository using your favorite method. :>
2. Navigate to the root directory and run `cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release` to create the build folder.
3. Run `cmake --build build` to build the project.
4. On Linux/MacOS, run `./build/release/smoke_simulation`, and on Windows, run `build\release\smoke_simulation.exe`.

# Controls

- `Left-Click`: adds density and/or velocity depending on interaction mode.
- `R`: resets the simulation
- `D`: toggle density interaction mode (enabled by default)
- `V`: toggle velocity interaction mode (enabled by default)
- `P`: toggle smoke streeam (disabled by default)
