# Iterative Reliability-Guided Polar Shortening

This project constructs a reusable shortening chain for non-binary polar
codes over the CCSK/AWGN channel. For a fixed mother length `N`, Galois-field
size `GF`, and SNR, one construction supports every later shortened length
`NS` and every dimension `K <= NS`.

## Principle

The mother transform is

```text
G_N = [[1, 0], [1, 1]] ^ (Kronecker log2(N)).
```

At shortening depth `S`, the active columns of `G_N` are inspected. Only
columns of active weight one are candidates. With the natural indexing used
here, the unique active input of candidate output `x[j]` is `u[j]`.

A genie-aided Monte Carlo simulation is run for the current shortening state.
Among the weight-one candidates, the input with the weakest measured
reliability is selected, output `x[j]` is shortened, and index `j` is removed
from both the active rows and columns used to construct the nested chain.

During every reliability simulation, all `N` inputs are random, including
inputs whose indices were previously removed. Each accumulated shortened
output is injected as its perfectly known actual encoded symbol:

```text
P(X[j] = encoded x[j]) = 1
P(X[j] != encoded x[j]) = 0
```

This makes it possible to observe how the reliability of every input channel
changes as more outputs become perfectly known. The process saves a new
reliability snapshot after every shortening depth.

## Requirements

- CMake 3.16+
- C++17 compiler
- OpenMP
- FFTW3
- Python 3

## Run

The following constructs depths `S=0` through `S=9` for `N=64`, using
10,000 Monte Carlo frames at every depth:

```bash
python3 run_sim.py 10000 -8 64 64 entropy --max-shortening 9 --threads 4 --jobs 4
```

Omit `--max-shortening` to construct the complete nested chain from `S=0`
through `S=N-1`:

```bash
python3 run_sim.py 10000 -8 64 64 entropy --threads 4 --jobs 4
```

To run the two-pass non-iterative construction for exactly `S=9`, use:

```bash
python3 run_sim.py 10000 -8 64 64 entropy --strategy non-iterative --max-shortening 9 --threads 4 --jobs 4
```

This mode measures reliability once without shortening, constructs all `S`
shortening positions from that fixed initial reliability and the evolving
weight-one candidate sets, then freezes their inputs to zero and measures
reliability once more with the corresponding outputs perfectly known as zero.
Its output directory ends in `_non_iterative_Sxxxx` so it cannot overwrite an
iterative construction.

For a Cartesian sweep over several SNR values and shortening counts, edit the
arrays and shared parameters near the top of `sweep_non_iterative.py`, then run:

```bash
python3 sweep_non_iterative.py
```

The selection metric is either `entropy` or `probability`, where probability
means the average `1 - P(true symbol)`.

## Outputs

Results are written under:

```text
constructions/GF<GF>/N<N>/SNR<SNR>_<metric>/
```

`shortening_order.txt` records the nested shortening order, the candidate set
at every decision, and the selected channel metrics.

Each depth also has a file such as:

```text
reliability_S0009_NS0055.txt
```

It contains:

- all shortened positions accumulated through that depth;
- the current weight-one candidates;
- a two-line `reliability_order` header listing all `N` inputs from best to
  worst;
- a two-line `shortened_positions` header;
- one naturally indexed row (`0` through `N-1`) containing entropy, average
  error probability, `is_active`, and `is_weight_one_candidate`.

For a later choice `NS`, use depth `S=N-NS`. The shortened positions are the
first `S` entries of `shortening_order.txt`. For any `K <= NS`, filter the
snapshot to its active inputs and take the first `K` in best-to-worst order as
the information set; the other `NS-K` active inputs are reliability-frozen.

The derivation helper performs this extraction automatically:

```bash
python3 derive_code.py constructions/GF64/N64/SNR-8.000_entropy --NS 55 --K 32
```

It reports the shortened outputs, shortening-induced frozen inputs,
additional reliability-frozen inputs, total frozen set, and information set.
