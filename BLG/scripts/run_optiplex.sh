# mono-core experiments

python3 ../build_all_N.py  --decoder dec0 --platform optiplex_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec1 --platform optiplex_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec3 --platform optiplex_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec4 --platform optiplex_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec5 --platform optiplex_1c --cores 1 --time 60

python3 ../build_all_GF.py --decoder dec0 --platform optiplex_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec1 --platform optiplex_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec3 --platform optiplex_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec4 --platform optiplex_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec5 --platform optiplex_1c --cores 1 --time 60

#python3 ../build_all_R.py  --decoder dec0 --platform optiplex_1c --cores 1 --time 60
#python3 ../build_all_R.py  --decoder dec1 --platform optiplex_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec3 --platform optiplex_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec4 --platform optiplex_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec5 --platform optiplex_1c --cores 1 --time 60

# 8 core expermients (8 high-perf. + 4 low-energy)

python3 ../build_all_N.py  --decoder dec3 --platform optiplex_8c --cores 8 --time 60
python3 ../build_all_N.py  --decoder dec4 --platform optiplex_8c --cores 8 --time 60
python3 ../build_all_N.py  --decoder dec5 --platform optiplex_8c --cores 8 --time 60

python3 ../build_all_GF.py --decoder dec3 --platform optiplex_8c --cores 8 --time 60
python3 ../build_all_GF.py --decoder dec4 --platform optiplex_8c --cores 8 --time 60
python3 ../build_all_GF.py --decoder dec5 --platform optiplex_8c --cores 8 --time 60

python3 ../build_all_R.py  --decoder dec3 --platform optiplex_8c --cores 8 --time 60
python3 ../build_all_R.py  --decoder dec4 --platform optiplex_8c --cores 8 --time 60
python3 ../build_all_R.py  --decoder dec5 --platform optiplex_8c --cores 8 --time 60
