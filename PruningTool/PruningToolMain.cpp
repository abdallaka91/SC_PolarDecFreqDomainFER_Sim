#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <sstream>
#include <algorithm>
#include "HelperFuncPruningTool.h"

using std::array;
using std::cout;
using std::endl;
using std::stod;
using std::stoi;
using std::string;
using std::vector;

int main()
{

    int q, N, K, n, nH, nL, nm;
    float Pt1, Pt2, Pt, EbN0, nbmontcarlo;

    string main_bub_dir = "/home/abdallah_ubuntu/Desktop/NBPolar_decoder/PA_NBPC_bubble_and_dec/BubblesPattern/ccsk_nb";
    EbN0 = -6.5;
    q = 64;
    N = 64;
    K = 42;
    nH = 20;
    nL = 20;
    Pt1 = 100;
    Pt2 = 50;

    float norm1 = 1e6;
    n = log2(N);
    nm = nL;
    std::ostringstream fname;
    fname.str("");
    fname.clear();

    fname << main_bub_dir << "/N" << N << "/ContributionMatrices/"
                                          "bubbles_N"
          << N << "_K" << K << "_GF" << q
          << "_SNR" << std::fixed << std::setprecision(3) << EbN0 << "_" << nH << "x" << nL
          << "_Cs_mat.txt";

    vector<vector<vector<vector<int>>>> Bt;
    vector<vector<vector<vector<float>>>> Cs;
    Cs.resize(n);
    Bt.resize(n);

    for (int l = 0; l < n; l++)
    {
        Cs[l].resize(1 << l);
        Bt[l].resize(1 << l, vector<vector<int>>(nH, vector<int>(nL, 0)));
    }
    std::string filename = fname.str();
    std::string line;
    std::ifstream file(filename);

    int line_cnt0, line_cnt = 0;
    int l, s, k;
    std::string value;
    for (int i = 0; i < 1e7; i++)
    {
        std::getline(file, line);
        size_t pos = line.find("Observations nb:");
        if (pos != std::string::npos)
        {
            value = line.substr(pos + 16);
            size_t start = value.find_first_not_of(" ");
            if (start != std::string::npos)
                value = value.substr(start);
            nbmontcarlo = std::stod(value);
            break;
        }
    }
    for (l = 0; l < n; l++)
    {
        for (s = 0; s < (1u << l); s++)
        {
            line_cnt0 = ((1u << l) - 1 + s) * nH;
            k = 0;
            while (k < nH)
            {
                std::getline(file, line);
                if (line.find_first_not_of("0123456789. ") != std::string::npos)
                {
                    continue;
                }
                if (!line.empty())
                {
                    std::istringstream iss(line);
                    std::vector<float> row;
                    float num;
                    while (iss >> num)
                    {
                        row.push_back(num);
                    }
                    Cs[l][s].push_back(row);
                    k++;
                }
            }
        }
    }

    for (l = 0; l < n; l++)
    {
        for (s = 0; s < (1u << l); s++)
        {
            for (auto &rw : Cs[l][s])
                for (auto &elem : rw)
                {
                    elem /= nbmontcarlo;
                    elem = std::round(elem * norm1);
                }
        }
    }

    fname.str("");
    fname.clear();

    fname << "./data/N" << N << "/ContributionMatrices/"
                                "bubbles_N"
          << N << "_K" << K << "_GF" << q
          << "_SNR" << std::fixed << std::setprecision(3) << EbN0 << "_" << nH << "x" << nL
          << "_Cs_norm_mat.txt";

    bool succ_writing = false, newsim = true;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < 1u << i; j++)
        {
            succ_writing = AppendClustBubblesToFile(fname.str(), Cs[i][j], i, j, newsim);
            newsim = false;
        }
    }
    if (succ_writing)
        std::cout << "Cs norm Matrices written to: " << filename << std::endl;

    vector<vector<int>> cnt_1st(n), cnt_1st_1;

    int j00, j11, cnt0, cnt1;
    for (int l = 0; l < n; l++)
    {
        if (l < 3)
            Pt = Pt1;
        else
            Pt = Pt2;
        cnt_1st[l].assign(1 << l, 0);
        for (int s = 0; s < 1 << l; s++)
        {
            cnt0 = 0;
            cnt1 = 0;
            for (int j0 = 0; j0 < nH; j0++)
            {
                for (int j1 = 0; j1 < nL; j1++)
                {
                    if (Cs[l][s][j0][j1] > Pt)
                    {
                        Bt[l][s][j0][j1] = 1;
                        if (j0 == 0)
                        {
                            j11 = j1;
                            cnt1++;
                        }
                        if (j1 == 0)
                        {
                            j00 = j0;
                            cnt0++;
                        }
                    }
                }
            }
            if (Bt[l][s][0][0])
            {
                cnt_1st[l][s] = j11 + 1;
                if (cnt1 < j11 + 1)
                    for (int j1 = 0; j1 <= j11; j1++)
                        Bt[l][s][0][j1] = 1;
                if (cnt0 < j00 + 1)
                    for (int j0 = 0; j0 <= j00; j0++)
                        Bt[l][s][j0][0] = 1;
            }
        }
    }
    cnt_1st_1 = cnt_1st;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < cnt_1st[i].size(); j++)
        {
            std::cout << cnt_1st[i][j] << " ";
        }
        std::cout << std::endl;
    }
    for (int l = n - 1; l > 0; l--)
    {
        for (int s = 0; s < cnt_1st_1[l].size() / 2; s++)
        {
            cnt_1st_1[l - 1][s] = std::max(cnt_1st_1[l][s * 2], cnt_1st_1[l - 1][s]);
            cnt_1st_1[l - 1][s] = std::max(cnt_1st_1[l - 1][s], cnt_1st_1[l][s * 2 + 1]);
        }
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < cnt_1st_1[i].size(); j++)
        {
            std::cout << cnt_1st_1[i][j] << " ";
        }
        std::cout << std::endl;
    }
    for (int l = 0; l < n; l++)
    {
        for (int s = 0; s < 1 << l; s++)
        {
            if (cnt_1st[l][s] != cnt_1st_1[l][s])
            {
                for (int j = 0; j < cnt_1st_1[l][s]; j++)
                {
                    Bt[l][s][0][j] = 1;
                }
            }
        }
    }

    fname.str("");
    fname.clear();

    fname << main_bub_dir << "/N" << N << "/BubblesIndicatorsMatrices/"
                                          "bubbles_N"
          << N << "_K" << K << "_GF" << q
          << "_SNR" << std::fixed << std::setprecision(3) << EbN0 << "_" << nH << "x" << nL
          << "_Pt1_" << std::fixed << std::setprecision(3) << Pt1
          << "_Pt2_" << std::fixed << std::setprecision(3) << Pt2
          << "_Bt_mat.txt";

    filename = fname.str();

    newsim = true;
    succ_writing = AppendClustBubblesToFile(fname.str(), cnt_1st_1, 0, 0, newsim);
    newsim = false;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < 1u << i; j++)
        {
            succ_writing = AppendClustBubblesToFile(fname.str(), Bt[i][j], i, j, newsim);
            newsim = false;
        }
    }
    if (succ_writing)
        std::cout << "Bt Matrices written to: " << filename << std::endl;

    fname.str("");
    fname.clear();

    fname << main_bub_dir << "/N" << N << "/BubblesIndicatorsLists/"
                                          "bubbles_N"
          << N << "_K" << K << "_GF" << q
          << "_SNR" << std::fixed << std::setprecision(3) << EbN0 << "_" << nH << "x" << nL
          << "_Pt1_" << std::fixed << std::setprecision(3) << Pt1
          << "_Pt2_" << std::fixed << std::setprecision(3) << Pt2
          << "_Bt_lsts.txt";

    filename = fname.str();

    writeBtMatrices(filename, n, nH, nL, Bt);
    return 0;
}