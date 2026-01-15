import subprocess
import re

# bits = [8, 10, 12, 14, 16]
bits = [10, 12]
snr_values = [-8.5, -8]
hpp_path = "BLG/src/definitions/custom_types.hpp"

# Read original content once
with open(hpp_path, "r") as f:
    original_content = f.read()

for b in bits:
    # Always start from the original content
    content = original_content
    
    # Replace any INTEGER_BITS value with current b
    content = re.sub(r'#define INTEGER_BITS \d+', f'#define INTEGER_BITS {b}', content)
    
    with open(hpp_path, "w") as f:
        f.write(content)
    
    # Build with shell pipe for error filtering
    subprocess.run("cmake --build build1 -j 2>&1 | grep -i 'error'", shell=True)
    
    # Run simulation for each SNR
    for snr in snr_values:
        cmd = f"OMP_NUM_THREADS=64 ./build1/Sim2 5000000 {snr} 64 1024 512 dec1_integer 60"
        print(f"Running: INTEGER_BITS={b}, SNR={snr}")
        subprocess.run(cmd, shell=True)