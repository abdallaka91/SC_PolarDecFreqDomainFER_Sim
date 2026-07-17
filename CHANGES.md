# Project Changes

This file is a commit-note style summary of the cleanup done for the simulator.

## What changed

- Vendored `BLG` into this repository as a normal folder instead of a Git submodule.
- Added CMake generation of `definitions/code.hpp` in the build directory.
- Removed the need for `BLG/src/definitions/codes/Nxx_GFyy.hpp` and `reliab_seq` for this simulator flow.
- Added `run_sim.py` to configure, clean-build, and run `Sim2` from one command.
- Made `run_sim.py` always rebuild in `build/` so each run starts from a clean CMake build.
- Made generated matrix files overwrite previous results instead of appending.
- Added argument validation for `Sim2`.
- Added `.gitignore` for build products, caches, and generated outputs.
- Added this README-oriented project documentation.

## Suggested commit message

```text
Vendor BLG and add clean simulation runner
```
