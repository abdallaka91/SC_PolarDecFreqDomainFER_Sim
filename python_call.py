import subprocess

def run_simulation(num_frames, GF, N, K, decoder, snr_values):
    """Run simulation for a specific K value with given SNR range"""
    
    print(f"\n{'='*60}")
    print(f"Running simulations for K={K}")
    print(f"SNR range: {min(snr_values)} to {max(snr_values)} dB")
    print(f"Total {len(snr_values)} SNR points")
    print(f"{'='*60}")
    
    for snr in snr_values:
        print(f"\n[K={K}] Running: SNR = {snr} dB")
        print("-" * 40)
        
        cmd = [
            "./build/Sim2",
            str(num_frames),
            str(snr),
            str(GF),
            str(N),
            str(K),
            decoder
        ]
        
        try:
            subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error running K={K}, SNR={snr}: {e}")
            continue
    
    print(f"\n✓ Completed all simulations for K={K}")

def main():
    num_frames = 5000000
    GF = 64
    N =64
    decoder = "dec1"
    
    K1 = 16
    snr_range1 = []
    snr = -10.5
    while snr <= -10.5:
        snr_range1.append(round(snr, 3))
        snr += 0.5
    
   
    
    print("Starting dual K-value SNR sweep")
    print("=" * 60)
    print(f"Fixed parameters:")
    print(f"  Frames: {num_frames}")
    print(f"  GF: {GF}")
    print(f"  N: {N}")
    print(f"  Decoder: {decoder}")
    print("=" * 60)
        

    run_simulation(num_frames, GF, N, K1, decoder, snr_range1)
    

    
 
    
    print(f"\n{'='*60}")
    print("All simulations completed successfully!")
    print(f"{'='*60}")

if __name__ == "__main__":
    main()