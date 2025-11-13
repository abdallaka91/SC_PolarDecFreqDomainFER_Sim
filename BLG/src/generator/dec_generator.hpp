//
// Created by legal on 29/06/2025.
//

#ifndef _frozen_tree_generator_
#define _frozen_tree_generator_

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>

enum next_node {
    RATE_0,
    RATE_1_FROM_F,
    RATE_1_FROM_G,
    REP_FROM_F,
    REP_FROM_G,
    SPC_FROM_F,
    SPC_FROM_G,
    LEAF_RATE_0,
    LEAF_RATE_1_FROM_F,
    LEAF_RATE_1_FROM_G,
    MID_NODE_FROM_F,
    MID_NODE_FROM_G,
    USELESS
};

class dec_generator {
private:
    std::ofstream ofile;
    const int GF;

public:
    bool verbose = true;

    bool en_rate_0   = true;
    bool en_rate_1   = true;
    bool en_rate_spc = true;
    bool en_rate_rep = true;

public:
    dec_generator(const int n, const int gf) : GF(gf) {
        next_node_status.resize(2 * n);
    }

    ~dec_generator() {
        // nothing
    }

    std::vector<next_node> next_node_status;

    void analyze(const int *frozen, const int size) {
        if (verbose == true) {
            printf("\nFrozen matrix:\n");
            for (int i = 0; i < size; i += 1) {
                if ((i % 8) == 0)
                    printf(" | ");
                if ((i % 32) == 0)
                    printf("\n | ");
                printf("%2d ", frozen[i]);
            }
            printf(" |\n");
        }

        int K_value = 0;
        for (int i = 0; i < size; i += 1) {
            K_value += (frozen[i] == false);
        }


        ofile.open("../src/decoders/dedicated/dedicated_execute.hpp");

        ofile << "#pragma once" << std::endl;
        ofile << std::endl;
        ofile << "#include \"f_function_freq_in.hpp\"" << std::endl;
        ofile << "#include \"f_function_proba_in.hpp\"" << std::endl;
        ofile << "#include \"g_function_freq_in.hpp\"" << std::endl;
        ofile << "#include \"g_function_proba_in.hpp\"" << std::endl;
        ofile << "#include \"decoder_dedicated.hpp\"" << std::endl;
        ofile << "#include \"middle_node_pruned_rep_after_f.hpp\"" << std::endl;
        ofile << "#include \"middle_node_pruned_rep_after_g.hpp\"" << std::endl;
        ofile << "#include \"middle_node_pruned_rate_0.hpp\"" << std::endl;
        ofile << "#include \"middle_node_pruned_rate_1_after_f.hpp\"" << std::endl;
        ofile << "#include \"middle_node_pruned_rate_1_after_g.hpp\"" << std::endl;
        ofile << "#include \"middle_node_pruned_spc_after_f.hpp\"" << std::endl;
        ofile << "#include \"middle_node_pruned_spc_after_g.hpp\"" << std::endl;
        ofile << std::endl;
        ofile << "template <int gf_size>" << std::endl;
        ofile << "void decoder_dedicated<gf_size>::execute(symbols_t * channel, uint16_t * decoded)" << std::endl;
        ofile << "{" << std::endl;

        const int n_elmnt = execute(frozen, 0, next_node_status.data(), 0, size);
        next_node_status.resize(n_elmnt);

        ofile << "}" << std::endl;
        ofile << std::endl;
        ofile << "template <int gf_size>" << std::endl;
        ofile << "const int decoder_dedicated<gf_size>::N_gen = " << size << ";" << std::endl;
        ofile << std::endl;
        ofile << "template <int gf_size>" << std::endl;
        ofile << "const int decoder_dedicated<gf_size>::K_gen = " << K_value << ";" << std::endl;
        ofile << std::endl;
        ofile.close();
        if ( verbose == true ) {
            printf("-> #elements : %d\n", n_elmnt);
            printf("-> #elements : %zu\n", next_node_status.size());
        }
    }

private:
    void add_space(const int level) {
        for (int i = 0; i < level; i += 1) {
            printf(" ");
        }
    }

    void indentation(const int level) {
        for (int z = 0; z < level; z += 1) printf("+  ");
    }

#define _enable_pruning_

    int execute(
        const int *frozen,
        const int curr_frozen,
        next_node *array,
        int curr_elmnt, // identifiant du premier frozen de la branche
        const int size, // nombre de LLRs a ce niveau
        const int p_llrs = 0,
        const int level = 1,
        const bool pred_is_f = false) {
        const int n = size / 2; // Assuming size is the number of symbols

        //
        // Analyse de la branche gauche
        //

        int sum_l = 0;
        for (int i = 0; i < n; i++)
            sum_l += frozen[curr_frozen + i];

        const bool is_rate0_after_f = en_rate_0   && (sum_l == n);
        const bool is_rate1_after_f = en_rate_1   && (sum_l == 0);
        const bool is_rep_after_f   = en_rate_rep && (sum_l == (n - 1)) && (frozen[curr_frozen + n - 1] == false);
        const bool is_spc_after_f   = en_rate_spc && (sum_l == 1      ) && (frozen[                  0] == true );

        int sum_r = 0;
        for (int i = 0; i < n; i++) sum_r += frozen[curr_frozen + n + i];

        const bool is_rate0_after_g = en_rate_0   && (sum_r == n);
        const bool is_rate1_after_g = en_rate_1   && (sum_r == 0);
        const bool is_rep_after_g   = en_rate_rep && (sum_r == (n - 1)) && (frozen[curr_frozen + n - 1] == false);
        const bool is_spc_after_g   = en_rate_spc && (sum_r == 1      ) && (frozen[                  0] == true );

        //
        //
        //

        ofile << "// NODE LEVEL (" << size << ")\n";

        if (verbose) {
            indentation(level); printf("NODE LEVEL (%d)\n", size);
            indentation(level); printf("F edge : "); for (int i = 0; i < n; i++) { printf("%d", frozen[curr_frozen + i]); } printf("\n");
            printf("after_f [%d %d %d %d]\n", is_rate0_after_f, is_rate1_after_f, is_rep_after_f, is_spc_after_f);
        }


        if (is_rate0_after_f) {
            //
            // Rien à faire du tout !
            //
            ofile << "\t" << "// f_function_freq_in<" << GF <<
                    ">(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !" << std::endl;
        } else if (level == 1) {
            if (verbose) {
                indentation(level);
                printf("\e[1;31m Internal[%d...%d] <= Channel(%d...%d) (f) Channel(%d...%d) \e[0m\n", 0, n, 0, n, n, n + n);
            }
            ofile << "\t" << "f_function_proba_in<" << GF << ">(internal, channel, channel + " << n << ", " << n << ");" << std::endl;
        } else {
            if (pred_is_f) {
                if (verbose) {
                    indentation(level);
                    printf("\e[1;31m Internal[%d...%d] <= Internal(%d...%d) (f) Internal(%d...%d) \e[0m\n", p_llrs + size, p_llrs + size + n, p_llrs, p_llrs + n, p_llrs + n, p_llrs + n + n);
                }
                ofile << "\t" << "f_function_freq_in<" << GF << ">(internal + " << p_llrs + size << ", internal + " << p_llrs << ", internal + " << p_llrs + n << ", " << n << ");" << std::endl;
            } else {
                if (verbose) {
                    indentation(level);
                    printf("\e[1;31m Internal[%d...%d] <= Internal(%d...%d) (f) Internal(%d...%d) \e[0m\n", p_llrs + size, p_llrs + size + n, p_llrs, p_llrs + n, p_llrs + n, p_llrs + n + n);
                }
                ofile << "\t" << "f_function_proba_in<" << GF << ">(internal + " << p_llrs + size << ", internal + " << p_llrs << ", internal + " << p_llrs + n << ", " << n << ");" << std::endl;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        //
        // ON TRAITE L'ARC DESCENDANT A GAUCHE DU NOEUD
        //
        //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        int next_elmnt;
        if (is_rate0_after_f) {
            if (n == 1) {
                if (verbose) {
                    indentation(level); printf("> Leaf rate-0 node found (%d)\n", n);
                    indentation(level); printf("\e[1;31m LEAF_RATE_0 (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen,
                           n);
                }
                ofile << "\t" << "leaf_node_rate_0<" << GF << ">(decoded + " << curr_frozen << ", symbols + " << curr_frozen << ");" << std::endl;
                array[curr_elmnt] = LEAF_RATE_0;
            } else {
                if (verbose) {
                    indentation(level); printf("> Rate-0 node found (%d)\n", n);
                    indentation(level); printf("\e[1;31m RATE_0 (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen, n);
                }
                ofile << "\t" << "middle_node_pruned_rate_0<" << GF << ">(decoded + " << curr_frozen << ", symbols + " << curr_frozen << ", " << n << ");" << std::endl;
                array[curr_elmnt] = RATE_0;
            }
            next_elmnt = curr_elmnt + 1;
        //
        // RATE 1
        //
        } else if (is_rate1_after_f) {
            if (n == 1) {
                if (verbose) {
                    indentation(level); printf("> Leaf rate-1 node found (%d)\n", n);
                    indentation(level); printf("\e[1;31m LEAF_RATE_1_FROM_F (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen, n);
                }
                ofile << "\t" << "leaf_node_after_f<" << GF << ">(internal + " << p_llrs << ", p_llrs + size + " << curr_frozen << ", symbols + " << curr_frozen << ");" << std::endl;
                array[curr_elmnt] = LEAF_RATE_1_FROM_F;
            } else {
                if (verbose) {
                    indentation(level); printf("> Rate-1 node found (%d)\n", n);
                    indentation(level); printf("\e[1;31m RATE_1_FROM_F (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen, n);
                }
                ofile << "\t" << "middle_node_pruned_rate_1_after_f<" << GF << ">(internal + " << p_llrs + size << ", decoded + " << curr_frozen << ", symbols + " << curr_frozen << ", " << n << ");" << std::endl;
                array[curr_elmnt] = RATE_1_FROM_F;
            }
            next_elmnt = curr_elmnt + 1;
            //
            // REPETITION node
            //
        } else if (is_rep_after_f) {
            if (verbose) {
                indentation(level); printf("> Leaf REP_F node found (%d)\n", n);
                indentation(level); printf("\e[1;31m REP_FROM_F (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen, n);
            }
            if( level == 1 )
                ofile << "\t" << "middle_node_pruned_rep_after_f<" << GF << ">(internal + " << p_llrs        << ", decoded + " << curr_frozen << ", symbols + " << curr_frozen << ", " << n << ");" << std::endl;
            else
                ofile << "\t" << "middle_node_pruned_rep_after_f<" << GF << ">(internal + " << p_llrs + size << ", decoded + " << curr_frozen << ", symbols + " << curr_frozen << ", " << n << ");" << std::endl;
            array[curr_elmnt] = REP_FROM_F;
            next_elmnt = curr_elmnt + 1;
        //
        // REPETITION node
        //
        } else if (is_spc_after_f) {
            if (verbose) {
                indentation(level); printf("> Leaf SPC_FROM_F node found (%d)\n", n);
                indentation(level); printf("\e[1;31m SPC_FROM_F (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen, n);
            }
            if( level == 1 )
                ofile << "\t" << "middle_node_pruned_spc_after_f<" << GF << ">(internal + " << p_llrs        << ", decoded + " << curr_frozen << ", symbols + " << curr_frozen << ", " << n << ");" << std::endl;
            else
                ofile << "\t" << "middle_node_pruned_spc_after_f<" << GF << ">(internal + " << p_llrs + size << ", decoded + " << curr_frozen << ", symbols + " << curr_frozen << ", " << n << ");" << std::endl;
            array[curr_elmnt] = SPC_FROM_F;
            next_elmnt = curr_elmnt + 1;
        //
        // NORMAL node
        //
        } else {
            if (verbose) {
                indentation(level); printf("> Normal (f) node found (%d)\n", n);
            }
            array[curr_elmnt] = MID_NODE_FROM_F;
            if (level == 1) {
                // au premier niveau, on doit transmettre (internal = 0) car on a utilise le channel
                next_elmnt = execute(frozen, curr_frozen, array, curr_elmnt + 1, n, p_llrs, level + 1, true);
            } else {
                next_elmnt = execute(frozen, curr_frozen, array, curr_elmnt + 1, n, p_llrs + size, level + 1, true);
            }
        }

        //
        // Analyse de la branche droite
        //
        if ( verbose ) {
            indentation(level); printf("G edge : ");
            for (int i = 0; i < n; i++) {
                printf("%d", frozen[curr_frozen + n + i]);
            }
            printf("\n");
            printf("after_g [%d %d %d %d]\n", is_rate0_after_g, is_rate1_after_g, is_rep_after_g, is_spc_after_g);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        //
        // ON TRAITE L'ARC DESCENDANT A DROITE DU NOEUD
        //
        //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        if (is_rate0_after_f) {
            //
            // SIMPLIFICATION DU NOEUD G SI ON A EU UN RATE 0 APRES G !
            //
            if (pred_is_f) {
                if (verbose) {
                    indentation(level); printf("\e[1;31m Internal[%d...%d] <= SPECIAL Internal(%d...%d) (g) Internal(%d...%d) \e[0m\n", p_llrs + size, p_llrs + size + n, p_llrs, p_llrs + n, p_llrs + n, p_llrs + n + n);
                }
                ofile << "\t" << "g_function_freq_in_after_rate_0<" << GF << ">(internal + " << p_llrs + size << ", internal + " << p_llrs << ", internal + " << p_llrs + n << ", " << n << ");" << std::endl;
            } else {
                if (verbose) {
                    indentation(level); printf("\e[1;31m Internal[%d...%d] <= SPECIAL Internal(%d...%d) (g) Internal(%d...%d) \e[0m\n", p_llrs + size, p_llrs + size + n, p_llrs, p_llrs + n, p_llrs + n, p_llrs + n + n);
                }
                if (level == 1)
                    ofile << "\t" << "g_function_proba_in_after_rate_0<" << GF << ">(internal, channel, channel + " << n << ", " << n << ");" << std::endl;
                else
                    ofile << "\t" << "g_function_proba_in_after_rate_0<" << GF << ">(internal + " << p_llrs + size << ", internal + " << p_llrs << ", internal + " << p_llrs + n << ", " << n << ");" << std::endl;
            }
        //
        // NORMAL node
        //
        } else if (level == 1) {
            if (verbose) {
                indentation(level); printf("\e[1;31m Internal[%d...%d] <= Channel(%d...%d) (g) Channel(%d...%d) \e[0m\n", 0, n, 0, n, n, n + n);
            }
            ofile << "\t" << "g_function_proba_in<" << GF << ">(internal, channel, channel + " << n << ", symbols, " << n << ");" << std::endl;
        //
        // NORMAL node
        //
        } else {
            if ( is_rate0_after_g ) {
                //
                // We do not need to perform the computation as their results won't be used !
                //
            }else if (pred_is_f) {
                if (verbose) {
                    indentation(level); printf("\e[1;31m Internal[%d...%d] <= Internal(%d...%d) (g) Internal(%d...%d) \e[0m\n", p_llrs + size, p_llrs + size + n, p_llrs, p_llrs + n, p_llrs + n, p_llrs + n + n);
                }
                ofile << "\t" << "g_function_freq_in<" << GF << ">(internal + " << p_llrs + size << ", internal + " << p_llrs << ", internal + " << p_llrs + n << ", symbols + " << curr_frozen << ", " << n << ");" << std::endl;
            } else {
                if (verbose) {
                    indentation(level); printf("\e[1;31m Internal[%d...%d] <= Internal(%d...%d) (g) Internal(%d...%d) \e[0m\n", p_llrs + size, p_llrs + size + n, p_llrs, p_llrs + n, p_llrs + n, p_llrs + n + n);
                }
                ofile << "\t" << "g_function_proba_in<" << GF << ">(internal + " << p_llrs + size << ", internal + " << p_llrs << ", internal + " << p_llrs + n << ", symbols + " << curr_frozen << ", " << n << ");" << std::endl;
            }
        }

        //
        //
        //
        int final_offset;
        if (is_rate0_after_g) {
            if (n == 1) {
                indentation(level); printf("> Leaf rate-0 node found (size = %d)\n", n);
                indentation(level); printf("\e[1;31m LEAF_RATE_0 (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen + n, n);
                ofile << "\t" << "leaf_node_rate_0<" << GF << ">(decoded + " << curr_frozen << ", symbols + " << curr_frozen + n << ");" << std::endl;
                array[next_elmnt] = LEAF_RATE_0;
            } else {
                indentation(level); printf("> Rate-0 node found (size = %d)\n", n);
                indentation(level); printf("\e[1;31m RATE_0 (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen + n, n);
                ofile << "\t" << "middle_node_pruned_rate_0<" << GF << ">(decoded + " << curr_frozen + n << ", symbols + " << curr_frozen + n << ", " << n << ");" << std::endl;
                array[next_elmnt] = RATE_0;
            }
            final_offset = next_elmnt + 1;
            //
            //
            //
        } else if (is_rate1_after_g) {
            if (n == 1) {
                indentation(level); printf("> Leaf rate-1 node found (size = %d)\n", n);
                indentation(level); printf("\e[1;31m LEAF_RATE_1_FROM_G (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen, n);
                ofile << "\t" << "leaf_node_after_g<" << GF << ">(internal + " << p_llrs + size << ", decoded + " << curr_frozen + n << ", symbols + " << curr_frozen + n << ");" << std::endl;
                array[next_elmnt] = LEAF_RATE_1_FROM_G;
            } else if (level != 1) {
                if ( verbose ) {
                    indentation(level); printf("> Rate-1 node found (size = %d)\n", n);
                    indentation(level); printf("\e[1;31m RATE_1_FROM_G (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen, n);
                }
                ofile << "\t" << "middle_node_pruned_rate_1_after_g<" << GF << ">(internal + " << p_llrs + size << ", decoded + " << curr_frozen + n << ", symbols + " << curr_frozen + n << ", " << n << ");" << std::endl;
                array[next_elmnt] = RATE_1_FROM_G;
            }else {
                ofile << "\t" << "middle_node_pruned_rate_1_after_g<" << GF << ">(internal + " << 0 << ", decoded + " << curr_frozen + n << ", symbols + " << curr_frozen + n << ", " << n << ");" << std::endl;
            }
            final_offset = next_elmnt + 1;
        //
        //
        //
        } else if (is_spc_after_g) {
            if (level == 1) {
                if ( verbose ) {
                    indentation(level); printf("> SPC node found (size = %d)\n", n);
                    indentation(level); printf("\e[1;31m SPC_FROM_G (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen, n);
                }
                ofile << "\t" << "middle_node_pruned_spc_after_g<" << GF << ">(internal + " << p_llrs        << ", decoded + " << curr_frozen + n << ", symbols + " << curr_frozen + n << ", " << n << ");" << std::endl;
            } else {
                if ( verbose ) {
                    indentation(level); printf("> SPC node found (size = %d)\n", n);
                    indentation(level); printf("\e[1;31m SPC_FROM_G (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen, n);
                }
                ofile << "\t" << "middle_node_pruned_spc_after_g<" << GF << ">(internal + " << p_llrs + size << ", decoded + " << curr_frozen + n << ", symbols + " << curr_frozen + n << ", " << n << ");" << std::endl;
            }
            array[next_elmnt] = SPC_FROM_G;
            final_offset = next_elmnt + 1;
        //
        //
        //
        } else if (is_rep_after_g) {
            for (int z = 0; z < level; z += 1) printf("+  ");
            printf("> Node REP_G node found (%d)\n", n);
            indentation(level); printf("\e[1;31m REP_NodeSymbols (Internal(%d) Symbols(%d) Size(%d )) \e[0m\n", p_llrs + n, curr_frozen, n);
            ofile << "\t" << "middle_node_pruned_rep_after_g<" << GF << ">(internal + " << p_llrs + size << ", decoded + " << curr_frozen + n << ", symbols + " << curr_frozen + n << ", " << n << ");" << std::endl;
            array[curr_elmnt] = REP_FROM_G;
            final_offset = next_elmnt + 1;
        } else {
            if ( verbose ) {
                indentation(level); printf(" > Normal (g) node found (size = %d)\n", n);
            }
            array[next_elmnt] = MID_NODE_FROM_G;
            if (level == 1) {
                // au premier niveau, on doit transmettre (internal = 0) car on a utilise le channel
                final_offset = execute(frozen, curr_frozen + n, array, next_elmnt + 1, n, p_llrs, level + 1, false);
            } else {
                final_offset = execute(frozen, curr_frozen + n, array, next_elmnt + 1, n, p_llrs + size, level + 1, false);
            }
        }
        //
        // NORMAL nodes
        //
        if (level == 1) {
            if ( verbose ) {
                indentation(level); printf("\e[1;31m Nothing \e[0m\n");
            }
        } else {
            if ( verbose ) {
                indentation(level); printf("\e[1;31m Symbols[%d...%d] <= Symbols(%d...%d) (xor) Symbols(%d...%d) \e[0m\n", p_llrs + size, p_llrs + size + n, p_llrs, p_llrs + n, p_llrs + n, p_llrs + n + n);
            }
            ofile << "\t" << "for(int i = 0; i < " << n << "; i += 1){" << std::endl;
            ofile << "\t" << "  symbols[" << curr_frozen << " + i] ^= symbols[" << curr_frozen + n << " + i];" << std::endl;
            ofile << "\t" << "}" << std::endl;
        }
        return final_offset;
    }
};

#endif // FROZEN_TREE_H
