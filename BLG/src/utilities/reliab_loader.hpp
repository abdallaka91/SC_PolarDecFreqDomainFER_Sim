#pragma once
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Load reliability sequence (method selectable)
// method = 1 → first sequence
// method = 2 → second sequence
inline std::vector<uint16_t> load_reliability_sequence(int GF, int N, float SNR)
{
    const int method = 1; // 🔧 Hard-coded now, can be changed later

    // Build the file path: ./matrices/GF64/N64/mat_N64_GF64_SNR1.000.txt
    std::ostringstream fname;
    fname << "./matrices/GF" << GF
          << "/N" << N
          << "/mat_N" << N
          << "_GF" << GF
          << "_SNR" << std::fixed << std::setprecision(3) << SNR
          << ".txt";

    std::string   filename = fname.str();
    std::ifstream file(filename);

    if (!file)
    {
        std::cerr << "❌ Cannot open reliability file: " << filename << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<uint16_t> reliab;
    std::string           line;
    int                   line_count = 0;

    // Read line by line until we reach the desired method line
    while (std::getline(file, line))
    {
        if (line.empty())
            continue; // skip blank lines
        ++line_count;

        if (line_count == method)
        {
            std::istringstream iss(line);
            int                val;
            while (iss >> val)
                reliab.push_back(static_cast<uint16_t>(val));
            break; // stop after the selected sequence
        }
    }

    if (reliab.empty())
    {
        std::cerr << "❌ Could not read reliability sequence (method "
                  << method << ") from file: " << filename << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "✅ Loaded reliability sequence (method "
              << method << ") of length " << reliab.size()
              << " from " << filename << std::endl;

    // Optional sanity check
    if (reliab.size() != static_cast<size_t>(N))
    {
        std::cerr << "⚠️  Warning: expected N=" << N
                  << " but file contains " << reliab.size()
                  << " values." << std::endl;
    }

    return reliab;
}
