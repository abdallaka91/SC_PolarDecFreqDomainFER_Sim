# SCL FER simulation

This worktree uses the SCL-enabled decoder library from the current
`software-nb-polar-decoder` main branch.

## Build the decoder library

```bash
cd ../software-nb-polar-decoder-scl
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target NbScFFTdec -j4
```

Install the resulting library beside the simulator executable sources:

```bash
cp build/libNbScFFTdec.so ../ccsk_simulator_fft_scl/libNbScFFTdec.so
```

## Build the simulator

```bash
cd ../ccsk_simulator_fft_scl
cmake -S . -B build
cmake --build build --target Sim2 -j4
```

## Run an SCL simulation

The decoder parameter order is list size, split count, metric, and debug flag.
For example, this runs naive SCL with `L=8`, `C=8`, metric 8, and debugging
disabled:

```bash
./build/Sim2 -snr-min -8 -snr-max -8 -snr-step 0.5 -q 64 -N 64 -K 32 -dec 'scl_naive_f32{8-8-8-0}' -cores 1 -errors 100 -cw 100000
```

Fast-SC-assisted naive SCL uses:

```text
f_scl_naive_f32{L-C-metric-debug}
```

Other available SCL families include `scl_pruned_f32`,
`scl_pruned_zc_f32`, `scl_spec_f32`, `scl_zc_f32`, `scl_zc_f32_t`,
`scl_zc_f32_c`, `scl_zc_f32_e`, `scl_zc_i32`, `scl_wave`, and
`scl-oracle`.
