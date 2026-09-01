# mono-core experiments

python3 ../build_all_N.py  --decoder dec0 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec1 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec3 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec4 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec5 --platform cyberapp2_1c --cores 1 --time 60

python3 ../build_all_GF.py --decoder dec0 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec1 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec3 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec4 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec5 --platform cyberapp2_1c --cores 1 --time 60

python3 ../build_all_R.py  --decoder dec0 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec1 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec3 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec4 --platform cyberapp2_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec5 --platform cyberapp2_1c --cores 1 --time 60

# 32 core expermients (32 high-perf.)

python3 ../build_all_N.py  --decoder dec3 --platform cyberapp2_32c --cores 32 --time 60
python3 ../build_all_N.py  --decoder dec4 --platform cyberapp2_32c --cores 32 --time 60
python3 ../build_all_N.py  --decoder dec5 --platform cyberapp2_32c --cores 32 --time 60

python3 ../build_all_GF.py --decoder dec3 --platform cyberapp2_32c --cores 32 --time 60
python3 ../build_all_GF.py --decoder dec4 --platform cyberapp2_32c --cores 32 --time 60
python3 ../build_all_GF.py --decoder dec5 --platform cyberapp2_32c --cores 32 --time 60

python3 ../build_all_R.py  --decoder dec3 --platform cyberapp2_32c --cores 32 --time 60
python3 ../build_all_R.py  --decoder dec4 --platform cyberapp2_32c --cores 32 --time 60
python3 ../build_all_R.py  --decoder dec5 --platform cyberapp2_32c --cores 32 --time 60

# 64 core expermients (64 high-perf.)

python3 ../build_all_N.py  --decoder dec3 --platform cyberapp2_64c --cores 64 --time 60
python3 ../build_all_N.py  --decoder dec4 --platform cyberapp2_64c --cores 64 --time 60
python3 ../build_all_N.py  --decoder dec5 --platform cyberapp2_64c --cores 64 --time 60

python3 ../build_all_GF.py --decoder dec3 --platform cyberapp2_64c --cores 64 --time 60
python3 ../build_all_GF.py --decoder dec4 --platform cyberapp2_64c --cores 64 --time 60
python3 ../build_all_GF.py --decoder dec5 --platform cyberapp2_64c --cores 64 --time 60

python3 ../build_all_R.py  --decoder dec3 --platform cyberapp2_64c --cores 64 --time 60
python3 ../build_all_R.py  --decoder dec4 --platform cyberapp2_64c --cores 64 --time 60
python3 ../build_all_R.py  --decoder dec5 --platform cyberapp2_64c --cores 64 --time 60
