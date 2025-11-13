#pragma once
//
//
//
//
//
#include "features/archi.hpp"
#include "definitions/custom_types.hpp"
//
//
//
//
//
inline bool fix_xor_list(uint16_t *list1, const uint16_t *list2, const symbols_t* internal, const int N) {

    int total_xor   = 0;
    for (int i = 0; i < N; i++)
        total_xor   ^= list1 [i];

    ///////////////////////////////////////////////////////////////////////////////////////
    //
    //
    int   best_1err_position = 0; // 32 max
    int   best_1err_symbole  = 0; // 32 max
    float best_1err_score    = 0; // 32 max
    for (int i = 0; i < N; i += 1) {
        const int   new_symb  = total_xor ^ list1[i];
        const float new_proba = internal[i].value[ new_symb ];
        if( new_proba > best_1err_score ){
            best_1err_symbole  = new_symb;
            best_1err_score    = new_proba;
            best_1err_position = i;
        }
    }
    //
    //
    ///////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////////////////
    //
    //
    // On recherche la position du plus grand argmax2
    // ainsi que la valeur de sa probabilité
    int   pos_argmax_2 = 0;
    float val_argmax_2 = 0;
    for(int i = 0; i < N; i += 1){
        const int   n_symbo = list2[i];
        const float n_proba = internal[i].value[ n_symbo ];
        if( n_proba > val_argmax_2 ){
            val_argmax_2 = n_proba;
            pos_argmax_2 = i;
        }
    }

    // Si les 2 valeurs sont identique alors on va tester la meme solution...
    // donc autant s'arreter car cela donnera le meme resultat
    if( best_1err_position == pos_argmax_2 ){
        list1[best_1err_position] = best_1err_symbole;
        return true;
    }

    // On change le symbol issue de la liste 1, et on keep l'ancienne
    // valeur car c'est ce que l'on renvoie...
    const int symbol_backup = list1[pos_argmax_2];
    list1[pos_argmax_2]     = list2[pos_argmax_2];

    // on met a jour la valeur du calcul de parité
    const int   local_xor   = total_xor ^ list1 [pos_argmax_2] ^ symbol_backup;
    const float local_proba = internal[ pos_argmax_2 ].value[ list2[ pos_argmax_2 ] ];//proba2[pos_argmax_2];

    int   best_2err_position = 0; // 32 max
    int   best_2err_symbole  = 0; // 32 max
    float best_2err_score    = 0; // 32 max
    for (int i = 0; i < N; i++) {
        if(i == pos_argmax_2) continue;
        const int   new_symb  = local_xor   ^ list1[i];
        const float new_proba = local_proba * internal[i].value[ new_symb ];
        if( new_proba > best_2err_score ){
            best_2err_symbole  = new_symb;
            best_2err_score    = new_proba;
            best_2err_position = i;
        }
    }
    // on remet la liste 1 dans son état initial
    list1[pos_argmax_2] = symbol_backup;
    //
    //
    ///////////////////////////////////////////////////////////////////////////////////////

    if( best_1err_score >= best_2err_score ){
        list1[best_1err_position] = best_1err_symbole;
        return true; // on repond tjs true car on n'a plus d'idée à ce stade ;-)
    }else{
        list1[pos_argmax_2      ] = list2[pos_argmax_2]; // le second minimum
        list1[best_2err_position] = best_2err_symbole;   // le symbol que l'on a identifié
        return true; // on repond tjs true car on n'a plus d'idée à ce stade ;-)
    }
}
//
//
//
//
//
