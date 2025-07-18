#include "decoder_naive_pruning.hpp"
#include "../shared/f_function.hpp"
#include "../shared/g_function.hpp"
//
//
//
//
//
void remove_xors(uint16_t * values, int size) {
    if (size == 1) {
        return;
    }
    for (int i = 0; i < size / 2; i += 1) {
        values[i] ^= values[i + size / 2];
    }
    remove_xors(values, size / 2);
    remove_xors(values + size / 2, size / 2);
}
//
//
//
//
//
template <int gf_size>
decoder_naive_pruning<gf_size>::decoder_naive_pruning(
    const int n,
    const int* frozen_symb ) : N(n)
{
    internal = new symbols_t[N];
    symbols  = new uint16_t [N];
    frozen   = new uint32_t [N];

    for (int i = 0; i < N; i++) {
        frozen[i] = frozen_symb[i];
    }
}
//
//
//
//
//
template <int gf_size>
decoder_naive_pruning<gf_size>::decoder_naive_pruning() : N(0)
{
    internal = nullptr;
    symbols  = nullptr;
    frozen   = nullptr;
    printf("(EE) Error we should never be there...\n");
    printf("(EE) %s %d\n", __FILE__, __LINE__);
    exit(EXIT_FAILURE);
}
//
//
//
//
//
template <int gf_size>
decoder_naive_pruning<gf_size>::~decoder_naive_pruning()
{
    delete[]internal;
    delete[]symbols;
    delete[]frozen;
}
//
//
//
//
//
template <int gf_size>
void decoder_naive_pruning<gf_size>::execute(symbols_t * channel, uint16_t *  decoded)
{
    const int n = N / 2; // Assuming size is the number of symbols
    //
    //
    //
    for (int i = 0; i < n; i++) {
        f_function<gf_size>( internal + i, channel + i, channel + n + i);
    }
    //
    //
    //
    middle_node_with_pruning( internal, internal + n, decoded, symbols, n, 0);
    //
    //
    //
    for (int i = 0; i < n; i++) {
        g_function<gf_size>( internal + i, channel + i, channel + n + i, symbols[i]);
    }
    //
    //
    //
    middle_node_with_pruning( internal, internal + n, decoded, symbols, n, n);
    //
    //
    //
    // No H computations as we are at the top node and we have a non systematic code !!!
    //
    //
    //
}
//
//
//
//
//
//#define HARD_DEBUG
bool fix_xor_list(int *list1, const int *list2, const float *proba1, const float *proba2, const symbols_t* internal, const int N) {
    int   total_xor   = 0;
    float total_proba = 1.f;
#ifdef HARD_DEBUG
    printf("\n");
    printf("[list 1] "); for (int i = 0; i < N; i++) { printf("%9d ",   list1 [i]); } printf("\n");
    printf("[list 2] "); for (int i = 0; i < N; i++) { printf("%9d ",   list2 [i]); } printf("\n");
    printf("[proba1] "); for (int i = 0; i < N; i++) { printf("%1.7f ", proba1[i]); } printf("\n");
    printf("[proba2] "); for (int i = 0; i < N; i++) { printf("%1.7f ", proba2[i]); } printf("\n");
#endif
//    exit( EXIT_FAILURE );

    for (int i = 0; i < N; i++) {
        total_xor   ^= list1 [i];
        total_proba *= proba1[i];
    }

    int list_c[32];
    for (int i = 0; i < N; i++)
        list_c[i] = list1[i];

    if (total_xor == 0) {
        // Déjà correct, rien à changer
        return true;
    }

    int   symbole[32];
    float score  [32];

#ifdef HARD_DEBUG
    for (int k = 0; k < N; k++)
        printf(" %3d", list_c[k]);
    printf(" : proba (%f)\n", total_proba);
#endif

    ///////////////////////////////////////////////////////////////////////////////////////
    //
    //
    for (int i = 0; i < N; i++) {
        const int   old_symb  = list1[i];
        const int   new_symb  = total_xor   ^ old_symb;
#ifdef HARD_DEBUG
        printf("total_xor = %2d, old_symb = %2d, new_symb = %2d :: ", total_xor, old_symb, new_symb);
#endif
        const float new_proba = /*total_proba * */ internal[i].value[ new_symb ] /* / internal[i].value[ old_symb ] */;
        symbole  [i] = new_symb;
        score    [i] = new_proba;
#ifdef HARD_DEBUG
        printf("- Case (1) %2d : [symb = %3d, score = %f]\n", i,  symbole[i], score[i]);
#endif
    }

    int   best_pos_1err   = 0;
    float best_score_1err = score[0];
    for (int i = 1; i < N; i++) {
        if( best_score_1err < score[i] ){
            best_pos_1err   = i;
            best_score_1err = score[i];
#ifdef HARD_DEBUG
            printf("- Solution (1) %2d : [symb = %3d, score = %f] (UPDATE)\n", i, symbole[i], score[i]);
#endif
        }
    }

    //
    // On le keep pour la suite 
    //
    const int best_symb_1err = symbole[best_pos_1err];
    
    //
    //
    ///////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////////////////
    //
    //
    int   pos_argmax_2 = 0;
    float val_argmax_2 = proba2[0];
    for(int i = 0; i < N; i += 1){
        if( proba2[i] > val_argmax_2 ){
            val_argmax_2 = proba2[i];
            pos_argmax_2 = i;
        }
    }

#ifdef HARD_DEBUG
    printf("ARGMAX2= %d - proba (%f)\n", pos_argmax_2, val_argmax_2);
#endif

    if( best_pos_1err == pos_argmax_2 )
    {
        //
        // On prend un bypass car le resultat sera tjs le meme...
        //
        list1[best_pos_1err] = best_symb_1err;
        return true;
    }

    const int symbol_backup = list1[pos_argmax_2];
    list1[pos_argmax_2]     = list2[pos_argmax_2];


    total_xor   = total_xor   ^ symbol_backup        ^ list1 [pos_argmax_2]; //
    total_proba = /*total_proba * */ proba2[pos_argmax_2] /* / / proba1[pos_argmax_2] */; //

    for (int i = 0; i < N; i++) {

        int   old_symb  = list1[i];
        int   new_symb  = total_xor   ^ old_symb;
        float new_proba = total_proba * internal[i].value[ new_symb ] /* / internal[i].value[ old_symb ] */;

        if(i == pos_argmax_2){
            new_symb  = -1;
            new_proba = 0.f;
        }

#ifdef HARD_DEBUG
        printf("total_xor = %2d, old_symb = %2d, new_symb = %2d :: ", total_xor, old_symb, new_symb);
#endif
        symbole  [i] = new_symb;
        score    [i] = new_proba;
#ifdef HARD_DEBUG
        printf("- Case (2) %2d : [symb = %3d, score = %f]\n", i,  symbole[i], score[i]);
#endif
    }

    list1[pos_argmax_2] = symbol_backup; // On remet la liste dans son état initial

    int   best_pos_2err   = 0;
    float best_score_2err = score[0];
    for (int i = 1; i < N; i++) {
        if( best_score_2err < score[i] ){
            best_pos_2err   = i;
            best_score_2err = score[i];
#ifdef HARD_DEBUG
            printf("- Solution (2) %2d : [symb = %3d, score = %f] (UPDATE)\n", i, symbole[i], score[i]);
#endif
        }
    }
    
    //
    //
    ///////////////////////////////////////////////////////////////////////////////////////

    if( best_score_1err >= best_score_2err )
    {
        list1[best_pos_1err] = best_symb_1err;
#ifdef HARD_DEBUG
        printf("[list A] "); for (int i = 0; i < N; i++) { printf("%9d ",   list1 [i]); } printf("\n");
#endif
    }else{
        list1[pos_argmax_2 ] = list2[pos_argmax_2];    // le second minimum
        list1[best_pos_2err] = symbole[best_pos_2err]; // le symbol que l'on a identifié
#ifdef HARD_DEBUG
        printf("[list B] "); for (int i = 0; i < N; i++) { printf("%9d ",   list1 [i]); } printf("\n");
#endif
    }
    return true; // on repond tjs true car on n'a plus d'idée à ce stade ;-)
}
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
bool fix_xor_list_old(int *list1, const int *list2, int N) {
    int total_xor = 0;
    for (int i = 0; i < N; i++) {
        total_xor ^= list1[i];
    }

    if (total_xor == 0) {
        // Déjà correct, rien à changer
        return true;
    }

    for (int i = 0; i < N; i++) {
        int current_xor = total_xor   ^ list1[i];
        int new_xor     = current_xor ^ list2[i];
        if (new_xor == 0) {
            list1[i] = list2[i];
            return true;
        }
    }

    // Aucun remplacement possible
    return false;
}

template <int gf_size>
void argmax2_indices(const float *arr, int *max1_idx, int *max2_idx) {
    float first_max_value  = arr[0];
    float second_max_value = 0.f;
    *max1_idx =  0;
    *max2_idx = -1;

    for (int i = 1; i < gf_size; i++)
    {
        if (arr[i] > first_max_value) {
            // Décale le premier vers le second
            second_max_value = first_max_value;
            *max2_idx        = *max1_idx;

            first_max_value = arr[i];
            *max1_idx = i;
        } else if (arr[i] > second_max_value) {
            second_max_value = arr[i];
            *max2_idx = i;
        }
    }
}

template <int gf_size>
void decoder_naive_pruning<gf_size>::middle_node_with_pruning(
    symbols_t * inputs,   // Inputs are the symbols from the channel (from the right)
    symbols_t * internal, // Internal nodes are the symbols computed during the process (to the left)
    uint16_t *  decoded,  // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
    int         size,     // Size is the number of symbols (should be a power of 2)
    const int   symbol_id)  // Symbol ID is the index of the FIRST symbol in the symbols array
{
    const int n = size / 2; // Assuming size is the number of symbols

    ///////////////////////////////////////////////////////////////////////////////////////
    //
#if 1
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += frozen[symbol_id + i];
    }
    if (sum == size) {
#if defined(debug_rate_1)
        printf("Frozen pruning with rate = 0\n");
#endif
        for (int i = 0; i < size; i++) {
            symbols[symbol_id + i] = 0;
            decoded[symbol_id + i] = 0;
        }
        return;
    }
#endif
    //
    ///////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////
    //
#if 1
    if (sum == 0) {
#if defined(debug_rate_1)
        printf("Frozen pruning with rate = 1\n");
#endif
        for (int i = 0; i < size; i++) {
            int value              = argmax<gf_size>(inputs[i].value);
            symbols[symbol_id + i] = value;
            decoded[symbol_id + i] = value; // should be corrected (it is systematic solution actually)
#if defined(debug_rate_1)
            printf("-> hard decision [%2d] = %d\n", symbol_id + i, symbols[symbol_id + i]);
#endif
        }
        remove_xors(decoded + symbol_id, size);
        return;
    }
#endif
    //
    ///////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////
    //
    //
    // REPETITION NODE !
    //
#if 1
    if( (sum == (size-1)) && (frozen[symbol_id + (size-1)] == false) ) {
#if defined(debug_rate_1)
        printf("Frozen pruning in REP mode [%d::%d]\n", symbol_id, size);
#endif
        // la decision dure sur size/2-1 = n-1 ???
        for(int i = 0; i < size; i++) {
            if ( inputs[i].is_freq == true ) {
                fwht_norm<gf_size>( inputs[i].value );
                inputs[i].is_freq = false;
            }
        }

        float temp[gf_size];
        for (int j = 0; j < gf_size; j++)
            temp[j] = inputs[0].value[j] * inputs[1].value[j];

        for(int i = 2; i < size; i++){
            if( (i & 0x1) == 1)
            normalize<gf_size>( temp );
            for (int j = 0; j < gf_size; j++)
                temp[j] *= inputs[i].value[j] ;
        }

        const int value = argmax<gf_size>( temp );
        for(int i = 0; i < size; i++)
        {
            symbols[symbol_id + i] = value;
            decoded[symbol_id + i] = 0; // should be corrected (it is systematic solution actually)
        }
        decoded[symbol_id + (size-1)] = value; // should be corrected (it is systematic solution actually)
        return ;
    }
#endif
    //
    ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////
    //
    //
    // SINGLE PARITY NODE
    //
#define SPC_NODE
#if defined(SPC_NODE)
    int check_node = 0;
    if( (sum == 1) && (frozen[symbol_id] == true) ) {
//#define debug_rate_spc
#if defined(debug_rate_spc)
        printf("Frozen pruning in SPC mode [%d::%d]\n", symbol_id, size);
#endif
        for(int i = 0; i < size; i++)
            if ( inputs[i].is_freq == true ) {
                fwht<gf_size>( inputs[i].value );
                inputs[i].is_freq = false;
                normalize<gf_size>(inputs[i].value);
            }

        for (int i = 0; i < size; i++) {
            int value              = argmax<gf_size>(inputs[i].value);
            check_node            ^= value;
            symbols[symbol_id + i] = value;
            decoded[symbol_id + i] = value; // should be corrected (it is systematic solution actually)
        }
        if ( check_node == 0 ) {
#if defined(debug_rate_spc)
            printf("-> CN equation is validated !\n");
#endif
            remove_xors(decoded + symbol_id, size);
            return;
        }
#if 1
        else {
#if defined(debug_rate_spc) 
            printf("-> CN equation is NOT validated (first round)\n");
            remove_xors(decoded + symbol_id, size);
            for (int j = 0; j < size; j++) {
                printf("  - symbol [%d :: %d] (%f) - Un = %d\n", j, symbols[symbol_id + j], inputs[j].value[symbols[symbol_id + j]], decoded[j + symbol_id]);
            }
            for (int j = 0; j < size; j++)
                show_symbols< gf_size >( inputs[j].value );

            printf("-> Testing second round\n");
#endif
            int arg_1[32];
            int arg_2[32];
            float val_1[32];
            float val_2[32];
            for (int j = 0; j < size; j++) {
                argmax2_indices<gf_size>(inputs[j].value, arg_1 + j, arg_2 + j);
                val_1[j] = inputs[j].value[arg_1[j]];
                val_2[j] = inputs[j].value[arg_2[j]];
#if defined(debug_rate_spc)
                printf("  - [%d] arg_1(%2d) and arg_1(%2d)\n", j, arg_1[j], arg_2[j]);
#endif
            }
            //
            bool isOK = fix_xor_list(arg_1, arg_2, val_1, val_2, inputs, size);
            if ( isOK ) {
#if defined(debug_rate_spc)
                printf("-> CN equation is validated !\n");
                for (int j = 0; j < size; j++)
                    printf("  - arg_1 [%d] (%2d)\n", j, arg_1[j]);
#endif
                for (int j = 0; j < size; j++) {
                    symbols[symbol_id + j] = arg_1[j];
                    decoded[symbol_id + j] = arg_1[j]; // should be corrected (it is systematic solution actually)
                }
                remove_xors(decoded + symbol_id, size);
#if defined(debug_rate_spc)
                for (int j = 0; j < size; j++) {
                    printf("  - symbol [%d :: %d] (%f) - Un = %d\n", j, symbols[symbol_id + j], inputs[j].value[symbols[symbol_id + j]], decoded[j + symbol_id]);
                }
#endif
                return;
            } else {
#if defined(debug_rate_spc)
                printf("-> CN equation is NOT validated (second round)\n");
#endif
            }
        }
#endif
    }
#endif

    //
    ///////////////////////////////////////////////////////////////////////////////////////

    //
    //
    //
    for (int i = 0; i < n; i++) {
        f_function<gf_size>(internal + i, inputs + i, inputs + n + i); // Example operation
    }
    //
    //
    //
    if (n == 1) {
        leaf_node( internal, decoded, symbols, symbol_id);
    } else {
        middle_node_with_pruning( internal, internal + n, decoded, symbols, n, symbol_id);
    }
    //
    //
    //
    for (int i = 0; i < n; i++) {
        g_function<gf_size>( internal + i, inputs + i, inputs + n + i, symbols[symbol_id + i]);
    }
    //
    //
    //
    if (n == 1) {
        leaf_node( internal, decoded, symbols, symbol_id + n);
    } else {
        middle_node_with_pruning( internal, internal + n, decoded, symbols, n, symbol_id + n);
    }
    //
    //
    //
    for (int i = 0; i < n; i++) {
        symbols[symbol_id + i] ^= symbols[symbol_id + n + i];
    }
#if defined(debug_rate_spc)
    if( check_node )
    {
        for (int i = 0; i < size; i++) {
            printf("(DD) symbols[%d] = %3d and decoded = %3d\n", symbol_id + i, symbols[symbol_id + i], decoded[symbol_id + i]);
        }
        exit(EXIT_FAILURE);
    }
#endif
    //
    //
    //
}
//
//
//
//
//
template <int gf_size>
void decoder_naive_pruning<gf_size>::middle_node_rate0(
    uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
    int         size,     // Size is the number of symbols (should be a power of 2)
    const int   symbol_id)  // Symbol ID is the index of the FIRST symbol in the symbols array
{
    const int n = size / 2; // Assuming size is the number of symbols
    //
    //
    //
    for (int i = 0; i < n; i += 1) {
        symbols[symbol_id + i] = 0; // to be checked !
    }
    //
    //
    //
}
//
//
//
//
//
template <int gf_size>
void decoder_naive_pruning<gf_size>::middle_node_rate1(
    symbols_t * inputs,   // Inputs are the symbols from the channel (from the right)
    uint16_t *  decoded,  // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
    int         size,     // Size is the number of symbols (should be a power of 2)
    const int   symbol_id)  // Symbol ID is the index of the FIRST symbol in the symbols array
{
    //
    //
    // on pourrait simplifier le code de la feuille car on sais qu'elle n'ait pas gelée
    //
    for (int i = 0; i < size; i++) {
        leaf_node(inputs + i, decoded, symbols, symbol_id + i);
    }
    //
    //
    //
}
//
//
//
//
//
template <int gf_size>
void decoder_naive_pruning<gf_size>::leaf_node(
    symbols_t * var,
    uint16_t *  decoded,
    uint16_t *  symbols,
    const int   symbol_id) const {
    if (frozen[symbol_id] == true) {
        decoded[symbol_id] = 0;
        symbols[symbol_id] = 0;
        return;
    }

    if (var->is_freq) {
        FWHT_NORM<gf_size>(var->value);
        var->is_freq = false;
    }
    const int max_index = argmax<gf_size>(var->value);
    decoded[symbol_id] = max_index;
    symbols[symbol_id] = max_index;
}
//
//
//
//
//
#if _GF_ == 8
    template class decoder_naive_pruning<  8>;
#elif _GF_ == 16
    template class decoder_naive_pruning< 16>;
#elif _GF_ == 32
    template class decoder_naive_pruning< 32>;
#elif _GF_ == 64
    template class decoder_naive_pruning< 64>;
#elif _GF_ == 128
    template class decoder_naive_pruning<128>;
#elif _GF_ == 256
    template class decoder_naive_pruning<256>;
#elif _GF_ == 512
    template class decoder_naive_pruning<512>;
#elif _GF_ == 1024
    template class decoder_naive_pruning<1024>;
#endif