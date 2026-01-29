#include "ccsk_simulator.hpp"
#include <iostream>
#include <chrono>
#include <omp.h>
#include <atomic>

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cout << "Usage: " << argv[0] << " <NbMonteCarlo> <real_sigma> <fake_sigma>" << std::endl;
        return 1;
    }

    int MAX_FRAME_ERRORS = 100;
    uint64_t NbMonteCarlo = std::stoull(argv[1]);
    double real_sigma = std::stod(argv[2]);
    double fake_sigma = std::stod(argv[3]);

    if (argc >= 5)
        MAX_FRAME_ERRORS = std::stoi(argv[4]);

    constexpr int N = 1024;
    constexpr int GF = 1024;

    int num_threads = omp_get_max_threads();
    std::cout << "Using " << num_threads << " threads" << std::endl;

    CCSK_Simulator<GF, N> simulator(real_sigma, fake_sigma, num_threads);

    std::atomic<uint64_t> frame_errors(0);
    std::atomic<uint64_t> frames_simulated(0);

    auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        uint16_t tx_symbol[N];

        while (true)
        {
            // Check stopping conditions
            uint64_t local_frames = frames_simulated.load(std::memory_order_relaxed);
            uint64_t local_errors = frame_errors.load(std::memory_order_relaxed);

            if (local_frames >= NbMonteCarlo || local_errors >= MAX_FRAME_ERRORS)
                break;

            // Try to increment frame counter
            if (!frames_simulated.compare_exchange_weak(local_frames, local_frames + 1))
                continue;

            // Simulate one frame
            simulator.generate_random_symbols(tx_symbol, thread_id);
            double *llr_values = simulator.simulate_frame(tx_symbol, thread_id);

            // Check for frame error
            bool frame_error = false;
            for (int sym_idx = 0; sym_idx < N && !frame_error; sym_idx++)
            {
                int detected = 0;
                for (int i = 1; i < GF; i++)
                {
                    if (llr_values[sym_idx * GF + i] < llr_values[sym_idx * GF + detected])
                    {
                        detected = i;
                    }
                }
                if (detected != tx_symbol[sym_idx])
                {
                    frame_error = true;
                }
            }

            if (frame_error)
            {
                frame_errors++;
            }

            delete[] llr_values;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double sec = std::chrono::duration<double>(end - start).count();

    uint64_t actual_frames = frames_simulated.load();
    uint64_t actual_errors = frame_errors.load();

    std::cout << "\n=== CCSK FER Simulation ===" << std::endl;
    std::cout << "Target frames: " << NbMonteCarlo << std::endl;
    std::cout << "Target errors: " << MAX_FRAME_ERRORS << std::endl;
    std::cout << "Actual frames: " << actual_frames << std::endl;
    std::cout << "Frame errors: " << actual_errors << std::endl;
    std::cout << "FER: " << (double)actual_errors / actual_frames << std::endl;
    std::cout << "Time: " << sec << " sec" << std::endl;
    std::cout << "Throughput: " << actual_frames / sec << " fps" << std::endl;

    return 0;
}