//
// loader_so.h
//
#pragma once

#include <dlfcn.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <format>

class frame_reader
{
private:
	//
	FILE* file_k = nullptr;
	FILE* file_n = nullptr;
	FILE* file_r = nullptr;
	//
    const int N;
    const int K;
    const int GF;
    const float snr;    
	//
public:

    frame_reader(const int _N, const int _K, const int _GF, const float _snr)
        : N(_N), K(_K), GF(_GF), snr(_snr)
    {
        std::string filen = std::format("./replay/N_{}/K_{}/SNR_{}/frame_symb_k.raw",  N, K, snr);
        file_k = fopen(filen.c_str(), "rb");
        if( file_k == nullptr ){
            std::cout << "(EE) Error during file opening " << filen << std::endl;
            std::cout << "(EE) Located in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }

        filen = std::format("./replay/N_{}/K_{}/SNR_{}/frame_symb_n.raw",  N, K, snr);
        file_n = fopen(filen.c_str(), "rb");
        if( file_n == nullptr ){ 
            std::cout << "(EE) Error during file opening " << filen << std::endl;
            std::cout << "(EE) Located in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }

        filen = std::format("./replay/N_{}/K_{}/SNR_{}/frame_symb_k.raw", N, K, snr);
        file_r = fopen(filen.c_str(), "rb");
        if( file_r == nullptr ){
            std::cout << "(EE) Error during file opening " << filen << std::endl;
            std::cout << "(EE) Located in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }
    }
    //
    //
    //
    bool read_k_symbols(std::vector<uint16_t>& array)
    {
        if( array.size() != K ){
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }
        int nBytes = fread((void*)array.data(), sizeof(uint16_t), array.size(), file_k);
		if( nBytes != array.size() ) {
            return false;
		}
        return true;
    }
    //
    //
    //
    bool read_u_symbols(std::vector<uint16_t>& array)
    {
        if( array.size() != N ){
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }
        int nBytes = fread((void*)array.data(), sizeof(uint16_t), array.size(), file_n);
		if( nBytes != array.size() ) {
            return false;
		}
        return true;
    }
    //
    //
    //
    bool read_noisy_llrs(std::vector<float>& llrs_n)
    {
        if( llrs_n.size() != (N * GF) ){
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }
		int nElmts = fread((void*)llrs_n.data(), sizeof(float), llrs_n.size(), file_r);
		if( nElmts != llrs_n.size() ) {
            return false;
		}
        return true;
    }
    //
    //
    //
    uint64_t read_number_proc_frames()
    {
        uint64_t counter = 0;
        std::string filen = std::format("./replay/N_{}/K_{}/SNR_{}/corrected_frames.raw", N, K, snr);
        FILE* hh = fopen(filen.c_str(), "rb");
        if( hh == nullptr ){
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }
        int nElmts = fread((void*)&counter, 1, sizeof(uint64_t), hh);
		if( nElmts != sizeof(uint64_t) ) {
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            std::cout << "(EE) sizeof(uint64_t) = " << sizeof(uint64_t)  << std::endl;
            std::cout << "(EE) nElmts.          = " << nElmts            << std::endl;
            std::cout << "(EE) filenme = "          << filen             << std::endl;
            exit( EXIT_FAILURE );
		}
        fclose(hh);
        return counter;
    }
    //
    //
    //
    ~frame_reader( )
    {
        fclose( file_k );
        fclose( file_n );
        fclose( file_r );
    }
    //
    //
    //
    bool is_open()
    {
        if( file_k == nullptr ) return false;
        if( file_n == nullptr ) return false;
        if( file_r == nullptr ) return false;
        return true;
    }
    //
    //
    //
};
