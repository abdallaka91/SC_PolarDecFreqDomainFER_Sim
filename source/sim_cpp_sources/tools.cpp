#include "tools.h"
#include <cmath>

void PoAwN::tools::Encoder(vector<vector<uint16_t>> &ucap,
                           vector<uint16_t> &NSYMB)
{
  uint16_t N = ucap[0].size();
  uint16_t n = ucap.size() - 1;

  for (int l = n - 1; l >= 0; l--)
  {
    for (uint16_t k = 0; k < N / 2; k++)
    {
      uint16_t pw1 = pow(2, n - l - 1);
      uint16_t a = 2 * k - k % pw1;
      uint16_t b = pw1 + 2 * k - k % pw1;
      ucap[l][a] = ucap[l + 1][a] ^ ucap[l + 1][b];
      ucap[l][b] = ucap[l + 1][b];
    }
  }

  for (uint16_t i = 0; i < N; ++i)
    NSYMB[i] = ucap[0][i];
}
