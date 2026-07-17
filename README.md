# SC Polar Decoder Frequency-Domain Simulation

This project runs Monte Carlo simulations for non-binary polar code construction with a genie-aided decoder.

`BLG` is included directly in this repository as normal source code. It is not a Git submodule anymore.

The `hybrid` branch also keeps the copied `ccsk_simulator/` folder from the FFT simulator branch, including `aff3ct_randn_gen`. The active executable is still the BLG genie-aided simulation.

## Requirements

- CMake 3.16+
- C++17 compiler
- OpenMP
- FFTW3, required for CCSK LLR computation
- Python 3

On Ubuntu:

```bash
sudo apt install build-essential cmake libfftw3-dev
```

## Run

From the project root:

```bash
python3 run_sim.py 10000 -8 64 64 entropy --threads 4 --jobs 4
```

Arguments:

- `10000`: number of simulated frames
- `-8`: SNR in dB
- `64`: GF size
- `64`: code length `N`
- `entropy`: sorting metric, either `entropy` or `probability`
- `--threads 4`: OpenMP threads
- `--jobs 4`: parallel build jobs

The Python script always deletes and rebuilds `build/`, then runs:

```bash
OMP_NUM_THREADS=4 ./build/Sim2 10000 -8 64 64 entropy
```

## Outputs

The simulator overwrites these files for each `(GF, N)`:

```text
entropies_probabilities/GF<GF>/GF<GF>N<N>.txt
matrices/GF<GF>/GF<GF>N<N>.txt
```

The `entropies_probabilities` file has four columns:

```text
index  average(1 - P(true symbol))  average entropy  hard-success count
```

Posterior probabilities are floored/saturated to `1e-12` before the entropy and `1 - P(true symbol)` values are computed.

## Notes

`run_sim.py` generates `build/generated/definitions/code.hpp` with `_GF_`, `_logGF_`, `_N_`, and `_logN_`. The simulation path does not need `reliab_seq`.

The channel LLR computation uses `ccsk_simulator/ccsk_llr.hpp`: observation FFT, multiply by the reversed CCSK base-sequence FFT, inverse FFT, subtract the minimum LLR, then normalize with `exp(-LLR)`. It uses the decoder-simulator style of one shared LLR calculator with per-thread FFT buffers. There is no lookup-table probability conversion and no non-FFTW fallback.
