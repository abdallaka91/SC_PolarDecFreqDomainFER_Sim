# Debug reliability and shortening override

Create `debug_reliab_short.txt` in this directory to override the normal
matrix reliability and automatically generated shortening sequence.

- Line 1: all `N` channel indices ordered from most reliable to least reliable.
- Line 2: exactly `N-NN` shortened channel indices.

When the file is absent, the simulator uses its normal matrix-based behavior.
