import subprocess
import re
import numpy as np
import os

build_dir = "dav2"
dec = "dec1"
NUM_THREADS = 15
Monte_carlo = int(5e9)
q =256
N = 64
K = 16
err_cnt = 80

snr_values = np.arange(-20, -14.5, 0.5)

custom_types_path = "BLG/src/definitions/custom_types.hpp"
code_path = "BLG/src/definitions/code.hpp"

with open(code_path, "r") as f2:
    original_content2 = f2.read()

pattern_include = r'#include "codes/N\d+_GF\d+\.hpp"'
replacement_include = f'#include "codes/N{N}_GF{q}.hpp"'
content2 = re.sub(pattern_include, replacement_include, original_content2)

with open(code_path, "w") as f2:
    f2.write(content2)

subprocess.run(
    f"cmake -S . -B {build_dir}  -DCMAKE_CXX_COMPILER=/usr/bin/g++   -DCMAKE_BUILD_TYPE=Release   -DFFTW3_INCLUDE_DIR=$HOME/fftw/include   -DFFTW3_LIBRARY=$HOME/fftw/lib/libfftw3.so",
    shell=True,
    check=True,
)

subprocess.run(
    f"cmake --build {build_dir} -j",
    shell=True,
    check=True,
)

for snr in snr_values:
    cmd = (
        f"OMP_NUM_THREADS={NUM_THREADS} "
        f"./{build_dir}/Sim2 {Monte_carlo} {snr} {q} {N} {K} {dec} {err_cnt}"
    )
    subprocess.run(cmd, shell=True, check=True)