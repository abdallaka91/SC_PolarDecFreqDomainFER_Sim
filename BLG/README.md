# Successive Cancellation NB polar code decoding (FFT-based)
 
## Projet compilation

To get all the features working fine, you should be sure that code.hpp contains:

```
#ifndef CONFIG_CODE_H
#define CONFIG_CODE_H
#include "codes/N8_GF64.hpp"
#endif
```

Then you can configure the project:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

If you want to select your compiler (clang seems to perform better), you can alternatively execute :

```
cmake -DCMAKE_BUILD_TYPE=Release -D CMAKE_CXX_COMPILER=clang++-20 ..
```

Then build the main programs for testing and benchmarking:

```
make
```

Note: other N and GF values are possible but only for geeks :-)

## Behavior verification


## Performance benchmarking


```
python3 ../build_all_GF.py --decoder dec3 --platform optiplex
python3 ../build_all_N.py  --decoder dec3 --platform optiplex
```

```
python3 ../build_all_GF.py --decoder dec4 --platform optiplex
python3 ../build_all_N.py  --decoder dec4 --platform optiplex
```
