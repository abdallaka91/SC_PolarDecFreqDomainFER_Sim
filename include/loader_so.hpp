//
// loader_so.h
//

#ifndef GENERATOR_LOADER_SO_H
#define GENERATOR_LOADER_SO_H

#include <dlfcn.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include "polar_encoder.hpp"
#include "decoder.hpp"  // inclure le header réel si tu l’as

// Définition des types de fonction correspondant à ta bibliothèque
typedef decoder* (*allocate_dec_func)(const std::string, int, int, const int*);
typedef polar_encoder* (*allocate_enc_func)(int, int, int, const int*);

class loader_so
{
private:
    static void* handle;
    static allocate_dec_func allocate_dec_ptr;
    static allocate_enc_func allocate_enc_ptr;

public:
    // Ouvre la librairie et récupère les pointeurs
    static bool open(const char* libname)
    {
        handle = dlopen(libname, RTLD_LAZY);
        if (!handle) {
            printf("(EE) Error during dlopen operation\n");
            printf("(EE) library = [%s] could not be opened\n", libname);
            printf("(EE) location [%s:%d]\n", __FILE__, __LINE__);
            exit( EXIT_FAILURE );
            return false;
        }

        dlerror(); // clear previous errors

        allocate_dec_ptr = (allocate_dec_func)dlsym(handle, "allocate_dec");
        char* err = dlerror();
        if ( err != nullptr ) {
            dlclose(handle);
            handle = nullptr;
            printf("(EE) Error dlsym allocate_dec failed\n");
            printf("(EE) library = [%s] could not be opened\n", libname);
            printf("(EE) location [%s:%d]\n", __FILE__, __LINE__);
            exit( EXIT_FAILURE );
            return false;
        }

        allocate_enc_ptr = (allocate_enc_func)dlsym(handle, "allocate_enc");
        err = dlerror();
        if ( err != nullptr ) {
            dlclose(handle);
            handle = nullptr;
            printf("(EE) Error dlsym allocate_enc failed\n");
            printf("(EE) library = [%s] could not be opened\n", libname);
            printf("(EE) location [%s:%d]\n", __FILE__, __LINE__);
            return false;
        }

        return true;
    }

    // Ferme la librairie
    static void close()
    {
        if (handle) {
            dlclose(handle);
            handle = nullptr;
        }
    }

    // Wrapper pour créer un encoder
    static polar_encoder* allocate_enc(int N, int K, int GF, const int* f_vector)
    {
        if (!allocate_enc_ptr) return nullptr;
        return allocate_enc_ptr(N, K, GF, f_vector);
    }

    // Wrapper pour créer un decoder
    static decoder* allocate_dec(const std::string& type, int N, int GF, const int* f_vector)
    {
        if (!allocate_dec_ptr) return nullptr;
        return allocate_dec_ptr(type, N, GF, f_vector);
    }
};

// Définition des pointeurs statiques
void* loader_so::handle = nullptr;
allocate_dec_func loader_so::allocate_dec_ptr = nullptr;
allocate_enc_func loader_so::allocate_enc_ptr = nullptr;

#endif // GENERATOR_LOADER_SO_H
