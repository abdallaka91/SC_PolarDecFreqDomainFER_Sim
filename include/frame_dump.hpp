//
// loader_so.h
//
#pragma once

#include <dlfcn.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <format>
#include <filesystem>   // std::filesystem::create_directories

inline bool make_dirs(const std::string& path)
{
    try {
        return std::filesystem::create_directories(path);   // true if anything was created
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error creating directories for '"
                  << path << "': " << e.what() << '\n';
        return false;
    }
}

class frame_dumper
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
    const float target;    
    const bool enable;
	//
public:

    frame_dumper(const int _N, const int _K, const int _GF, const float _snr, const float _target, const bool enabled)
        : N(_N), K(_K), GF(_GF), snr(_snr), target(_target), enable(enabled)
    {
        if( enable == false )
            return;

        //
        // On cree les repertoires pour stocker les fichiers en cas de besoin
        //
        std::string path = std::format("./replay/N_{}/K_{}/SNR{}-TARGET{}/",  N, K, snr, target);
        make_dirs( path );

        std::string filen = std::format("./replay/N_{}/K_{}/SNR{}-TARGET{}/frame_symb_k.raw",  N, K, snr, target);
        file_k = fopen(filen.c_str(), "wb");
        if( file_k == nullptr ){
            std::cout << "(EE) Error during file opening " << filen << std::endl;
            std::cout << "(EE) Located in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }

        filen = std::format("./replay/N_{}/K_{}/SNR{}-TARGET{}/frame_symb_n.raw",  N, K, snr, target);
        file_n = fopen(filen.c_str(), "wb");
        if( file_n == nullptr ){ 
            std::cout << "(EE) Error during file opening " << filen << std::endl;
            std::cout << "(EE) Located in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }

        filen = std::format("./replay/N_{}/K_{}/SNR{}-TARGET{}/frame_symb_k.raw", N, K, snr, target);
        file_r = fopen(filen.c_str(), "wb");
        if( file_r == nullptr ){
            std::cout << "(EE) Error during file opening " << filen << std::endl;
            std::cout << "(EE) Located in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }
    }
    //
    //
    //
    bool write_k_symbols(const std::vector<uint16_t>& array)
    {
        if( enable == false ){
            return false;
        }
        if( array.size() != K ){
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }
        int nBytes = fwrite((void*)array.data(), sizeof(uint16_t), array.size(), file_k);
		if( nBytes != array.size() ) {
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
		}
        return true;
    }
    //
    //
    //
    bool write_u_symbols(const std::vector<uint16_t>& array)
    {
        if( enable == false ){
            return false;
        }
        if( array.size() != N ){
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }
        int nBytes = fwrite((void*)array.data(), sizeof(uint16_t), array.size(), file_n);
		if( nBytes != array.size() ) {
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
		}
        return true;
    }
    //
    //
    //
    bool write_noisy_llrs(const std::vector<float>& llrs_n)
    {
        if( enable == false ){
            return false;
        }
        if( llrs_n.size() != (N * GF) ){
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
        }
		int nElmts = fwrite((void*)llrs_n.data(), sizeof(float), llrs_n.size(), file_r);
		if( nElmts != llrs_n.size() ) {
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
		}
        return true;
    }
    //
    //
    //
    bool write_number_proc_frames(const uint64_t counter)
    {
        if( enable == false ){
            return false;
        }
        std::string filen = std::format("./replay/N_{}/K_{}/SNR{}-TARGET{}/corrected_frames.raw", N, K, snr, target);
        FILE* hh = fopen(filen.c_str(), "wb");
        if( hh == nullptr ){
            exit( EXIT_FAILURE );
        }
        int nElmts = fwrite((void*)&counter, sizeof(uint64_t), 1, hh);
		if( nElmts != 1 ) {
            std::cout << "(EE) Error in " << __FILE__ << ":" << __LINE__ << std::endl;
            exit( EXIT_FAILURE );
		}
        fclose(hh);
        return true;
    }
    //
    //
    //
    ~frame_dumper( )
    {
        if( enable == false ){
            return;
        }
        fclose( file_k );
        fclose( file_n );
        fclose( file_r );
    }
    //
    //
    //


};
