# mono-core experiments

python3 ../build_all_N.py  --decoder dec0 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec1 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec3 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec4 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec5 --platform ryzen7900_1c --cores 1 --time 60

python3 ../build_all_GF.py --decoder dec0 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec1 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec3 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec4 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec5 --platform ryzen7900_1c --cores 1 --time 60

python3 ../build_all_R.py  --decoder dec0 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec1 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec3 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec4 --platform ryzen7900_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec5 --platform ryzen7900_1c --cores 1 --time 60

# 12 core expermients (12 high-perf.)

python3 ../build_all_N.py  --decoder dec3 --platform ryzen7900_12c --cores 12 --time 60
python3 ../build_all_N.py  --decoder dec4 --platform ryzen7900_12c --cores 12 --time 60
python3 ../build_all_N.py  --decoder dec5 --platform ryzen7900_12c --cores 12 --time 60

python3 ../build_all_GF.py --decoder dec3 --platform ryzen7900_12c --cores 12 --time 60
python3 ../build_all_GF.py --decoder dec4 --platform ryzen7900_12c --cores 12 --time 60
python3 ../build_all_GF.py --decoder dec5 --platform ryzen7900_12c --cores 12 --time 60

python3 ../build_all_R.py  --decoder dec3 --platform ryzen7900_12c --cores 12 --time 60
python3 ../build_all_R.py  --decoder dec4 --platform ryzen7900_12c --cores 12 --time 60
python3 ../build_all_R.py  --decoder dec5 --platform ryzen7900_12c --cores 12 --time 60

# 24 core expermients (24 high-perf.)

python3 ../build_all_N.py  --decoder dec3 --platform ryzen7900_24c --cores 24 --time 60
python3 ../build_all_N.py  --decoder dec4 --platform ryzen7900_24c --cores 24 --time 60
python3 ../build_all_N.py  --decoder dec5 --platform ryzen7900_24c --cores 24 --time 60

python3 ../build_all_GF.py --decoder dec3 --platform ryzen7900_24c --cores 24 --time 60
python3 ../build_all_GF.py --decoder dec4 --platform ryzen7900_24c --cores 24 --time 60
python3 ../build_all_GF.py --decoder dec5 --platform ryzen7900_24c --cores 24 --time 60

python3 ../build_all_R.py  --decoder dec3 --platform ryzen7900_24c --cores 24 --time 60
python3 ../build_all_R.py  --decoder dec4 --platform ryzen7900_24c --cores 24 --time 60
python3 ../build_all_R.py  --decoder dec5 --platform ryzen7900_24c --cores 24 --time 60
