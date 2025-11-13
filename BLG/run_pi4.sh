# 1 core experiments

python3 ../build_all_N.py  --decoder dec3 --platform pi4_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec4 --platform pi4_1c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec5 --platform pi4_1c --cores 1 --time 60

python3 ../build_all_GF.py --decoder dec3 --platform pi4_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec4 --platform pi4_1c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec5 --platform pi4_1c --cores 1 --time 60

python3 ../build_all_R.py  --decoder dec3 --platform pi4_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec4 --platform pi4_1c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec5 --platform pi4_1c --cores 1 --time 60

# 4 cores experiments

python3 ../build_all_N.py  --decoder dec3 --platform pi4_4c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec4 --platform pi4_4c --cores 1 --time 60
python3 ../build_all_N.py  --decoder dec5 --platform pi4_4c --cores 1 --time 60

python3 ../build_all_GF.py --decoder dec3 --platform pi4_4c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec4 --platform pi4_4c --cores 1 --time 60
python3 ../build_all_GF.py --decoder dec5 --platform pi4_4c --cores 1 --time 60

python3 ../build_all_R.py  --decoder dec3 --platform pi4_4c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec4 --platform pi4_4c --cores 1 --time 60
python3 ../build_all_R.py  --decoder dec5 --platform pi4_4c --cores 1 --time 60
