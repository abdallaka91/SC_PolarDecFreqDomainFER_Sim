import subprocess
import re
import numpy as np
import os

build_dir = "dav6"
dec = "dec4_integer"
NUM_THREADS = 64
Mont_carlo = 10e6
q = 256
N = 64
K = 48
err_cnt = 100
sigma = 5.4


bits = np.arange(14, 15, 1)
snr_values = np.arange(-15, -10.5, 0.5)
np.append(snr_values, -10.75)
custom_types_path = "BLG/src/definitions/custom_types.hpp"
code_path = "BLG/src/definitions/code.hpp"

with open(code_path, "r") as f2:
    original_content2 = f2.read()
content2 = original_content2

pattern_include = r'#include "codes/N\d+_GF\d+\.hpp"'
replacement_include = f'#include "codes/N{N}_GF{q}.hpp"'
content2 = re.sub(pattern_include, replacement_include, content2)


with open(code_path, "w") as f2:
    f2.write(content2)


for b in bits:
    with open(custom_types_path, "r") as f1:
        original_content1 = f1.read()
    content1 = original_content1
    pattern1 = r"constexpr int NBITS = \d+;"
    replacement1 = f"constexpr int NBITS = {b};"
    content1 = re.sub(pattern1, replacement1, content1)
    with open(custom_types_path, "w") as f1:
        f1.write(content1)
    subprocess.run(
        f"cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++-12 -DCMAKE_C_COMPILER=/usr/bin/gcc-12 -DCMAKE_BUILD_TYPE=Release -S . -B {build_dir}",
        shell=True,
    )
    subprocess.run(f"cmake --build  {build_dir} -j 2>&1 | grep -i 'error'", shell=True)
    for snr in snr_values:
        cmd = f"OMP_NUM_THREADS={NUM_THREADS}  ./{build_dir}/Sim2 {int(Mont_carlo)} {snr} {q} {N} {K} {dec} {err_cnt} {sigma}"
        print(f"Running: INTEGER_BITS={b}, SNR={snr}")
        subprocess.run(cmd, shell=True)
