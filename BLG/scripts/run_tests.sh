rm -rf ./log

# Amélioration de notre algo de décodage

python3 ../build_all_N.py  --decoder dec0        --platform testing --cores 1  --time 0
python3 ../build_all_N.py  --decoder dec1        --platform testing --cores 1  --time 0
python3 ../build_all_N.py  --decoder dec1_fixed  --platform testing --cores 1  --time 0
python3 ../build_all_N.py  --decoder dec1_cfloat --platform testing --cores 1  --time 0
python3 ../build_all_N.py  --decoder dec3        --platform testing --cores 1  --time 0
python3 ../build_all_N.py  --decoder dec4        --platform testing --cores 1  --time 0
python3 ../build_all_N.py  --decoder dec5        --platform testing --cores 1  --time 0

python3 ../build_all_GF.py --decoder dec0        --platform testing --cores 1  --time 0
python3 ../build_all_GF.py --decoder dec1        --platform testing --cores 1  --time 0
python3 ../build_all_GF.py --decoder dec1_fixed  --platform testing --cores 1  --time 0
python3 ../build_all_GF.py --decoder dec1_cfloat --platform testing --cores 1  --time 0
python3 ../build_all_GF.py --decoder dec3        --platform testing --cores 1  --time 0
python3 ../build_all_GF.py --decoder dec4        --platform testing --cores 1  --time 0
python3 ../build_all_GF.py --decoder dec5        --platform testing --cores 1  --time 0

python3 ../build_all_R.py  --decoder dec0        --platform testing --cores 1  --time 0
python3 ../build_all_R.py  --decoder dec1        --platform testing --cores 1  --time 0
python3 ../build_all_R.py  --decoder dec1_fixed  --platform testing --cores 1  --time 0
python3 ../build_all_R.py  --decoder dec1_cfloat --platform testing --cores 1  --time 0
python3 ../build_all_R.py  --decoder dec3        --platform testing --cores 1  --time 0
python3 ../build_all_R.py  --decoder dec4        --platform testing --cores 1  --time 0
python3 ../build_all_R.py  --decoder dec5        --platform testing --cores 1  --time 0
