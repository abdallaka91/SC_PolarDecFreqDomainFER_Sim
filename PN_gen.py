import numpy as np

def pseudo_random_binary_sequence(N, seed=1):
    np.random.seed(seed)  # for reproducibility
    return np.random.randint(0, 2, N)

# Example usage:
N = 2048  # or 4096
sequence = pseudo_random_binary_sequence(N)
print(sequence)